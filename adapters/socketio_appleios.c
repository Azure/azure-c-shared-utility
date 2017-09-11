// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <signal.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "azure_c_shared_utility/socketio.h"
#include <sys/types.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include "azure_c_shared_utility/singlylinkedlist.h"
#include "azure_c_shared_utility/gballoc.h"
#include "azure_c_shared_utility/optimize_size.h"
#include "azure_c_shared_utility/optionhandler.h"
#include "azure_c_shared_utility/shared_util_options.h"
#include "azure_c_shared_utility/xlogging.h"
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <CoreFoundation/CoreFoundation.h>
#include <CFNetwork/CFSocketStream.h>

#define SOCKET_SUCCESS                 0
#define INVALID_SOCKET                 -1
#define MAC_ADDRESS_STRING_LENGTH      18

#ifndef IFREQ_BUFFER_SIZE
#define IFREQ_BUFFER_SIZE              1024
#endif

// connect timeout in seconds
#define CONNECT_TIMEOUT         10

typedef enum IO_STATE_TAG
{
    IO_STATE_CLOSED,
    IO_STATE_OPENING,
    IO_STATE_OPEN,
    IO_STATE_CLOSING,
    IO_STATE_ERROR
} IO_STATE;

typedef struct PENDING_SOCKET_IO_TAG
{
    unsigned char* bytes;
    size_t size;
    ON_SEND_COMPLETE on_send_complete;
    void* callback_context;
    SINGLYLINKEDLIST_HANDLE pending_io_list;
} PENDING_SOCKET_IO;

typedef struct SOCKET_IO_INSTANCE_TAG
{
    CFReadStreamRef sockRead;
    CFWriteStreamRef sockWrite;
    ON_IO_OPEN_COMPLETE on_open_complete;
    ON_BYTES_RECEIVED on_bytes_received;
    ON_IO_ERROR on_io_error;
    void* on_open_complete_context;
    void* on_bytes_received_context;
    void* on_io_error_context;
    char* hostname;
    int port;
    char* target_mac_address;
    IO_STATE io_state;
    SINGLYLINKEDLIST_HANDLE pending_io_list;
} SOCKET_IO_INSTANCE;

typedef struct ACCEPTED_SOCKET_INSTANCE_TAG
{
    CFReadStreamRef sockRead;
    CFWriteStreamRef sockWrite;
} ACCEPTED_SOCKET_INSTANCE;

typedef struct NETWORK_INTERFACE_DESCRIPTION_TAG
{
    char* name;
    char* mac_address;
    char* ip_address;
    struct NETWORK_INTERFACE_DESCRIPTION_TAG* next;
} NETWORK_INTERFACE_DESCRIPTION;

/*this function will clone an option given by name and value*/
static void* socketio_CloneOption(const char* name, const void* value)
{
    void* result = NULL;
    
    if (name != NULL)
    {
        if (strcmp(name, OPTION_NET_INT_MAC_ADDRESS) == 0)
        {
            if (value == NULL)
            {
                LogError("Failed cloning option %s (value is NULL)", name);
            }
            else
            {
                if ((result = malloc(sizeof(char) * (strlen((char*)value) + 1))) == NULL)
                {
                    LogError("Failed cloning option %s (malloc failed)", name);
                }
                else if (strcpy((char*)result, (char*)value) == NULL)
                {
                    LogError("Failed cloning option %s (strcpy failed)", name);
                    free(result);
                    result = NULL;
                }
            }
        }
        else
        {
            LogError("Cannot clone option %s (not suppported)", name);
        }
    }
    
    return result;
}

/*this function destroys an option previously created*/
static void socketio_DestroyOption(const char* name, const void* value)
{
    if (name != NULL)
    {
        if (strcmp(name, OPTION_NET_INT_MAC_ADDRESS) == 0 && value != NULL)
        {
            free((void*)value);
        }
    }
}

static OPTIONHANDLER_HANDLE socketio_retrieveoptions(CONCRETE_IO_HANDLE handle)
{
    OPTIONHANDLER_HANDLE result;
    
    if (handle == NULL)
    {
        LogError("failed retrieving options (handle is NULL)");
        result = NULL;
    }
    else
    {
        SOCKET_IO_INSTANCE* socket_io_instance = (SOCKET_IO_INSTANCE*)handle;
        
        result = OptionHandler_Create(socketio_CloneOption, socketio_DestroyOption, socketio_setoption);
        if (result == NULL)
        {
            LogError("unable to OptionHandler_Create");
        }
        else if (socket_io_instance->target_mac_address != NULL &&
                 OptionHandler_AddOption(result, OPTION_NET_INT_MAC_ADDRESS, socket_io_instance->target_mac_address) != OPTIONHANDLER_OK)
        {
            LogError("failed retrieving options (failed adding net_interface_mac_address)");
            OptionHandler_Destroy(result);
            result = NULL;
        }
    }
    
    return result;
}

static const IO_INTERFACE_DESCRIPTION socket_io_interface_description =
{
    socketio_retrieveoptions,
    socketio_create,
    socketio_destroy,
    socketio_open,
    socketio_close,
    socketio_send,
    socketio_dowork,
    socketio_setoption
};

static void indicate_error(SOCKET_IO_INSTANCE* socket_io_instance)
{
    if (socket_io_instance->on_io_error != NULL)
    {
        socket_io_instance->on_io_error(socket_io_instance->on_io_error_context);
    }
}

static int add_pending_io(SOCKET_IO_INSTANCE* socket_io_instance, const unsigned char* buffer, size_t size, ON_SEND_COMPLETE on_send_complete, void* callback_context)
{
    int result;
    PENDING_SOCKET_IO* pending_socket_io = (PENDING_SOCKET_IO*)malloc(sizeof(PENDING_SOCKET_IO));
    if (pending_socket_io == NULL)
    {
        result = __FAILURE__;
    }
    else
    {
        pending_socket_io->bytes = (unsigned char*)malloc(size);
        if (pending_socket_io->bytes == NULL)
        {
            LogError("Allocation Failure: Unable to allocate pending list.");
            free(pending_socket_io);
            result = __FAILURE__;
        }
        else
        {
            pending_socket_io->size = size;
            pending_socket_io->on_send_complete = on_send_complete;
            pending_socket_io->callback_context = callback_context;
            pending_socket_io->pending_io_list = socket_io_instance->pending_io_list;
            (void)memcpy(pending_socket_io->bytes, buffer, size);
            
            if (singlylinkedlist_add(socket_io_instance->pending_io_list, pending_socket_io) == NULL)
            {
                LogError("Failure: Unable to add socket to pending list.");
                free(pending_socket_io->bytes);
                free(pending_socket_io);
                result = __FAILURE__;
            }
            else
            {
                result = 0;
            }
        }
    }
    
    return result;
}

#ifndef __APPLE__
static void destroy_network_interface_descriptions(NETWORK_INTERFACE_DESCRIPTION* nid)
{
    if (nid != NULL)
    {
        if (nid->next != NULL)
        {
            destroy_network_interface_descriptions(nid->next);
        }
        
        if (nid->name != NULL)
        {
            free(nid->name);
        }
        
        if (nid->mac_address != NULL)
        {
            free(nid->mac_address);
        }
        
        if (nid->ip_address != NULL)
        {
            free(nid->ip_address);
        }
        
        free(nid);
    }
}

static NETWORK_INTERFACE_DESCRIPTION* create_network_interface_description(struct ifreq *ifr, NETWORK_INTERFACE_DESCRIPTION* previous_nid)
{
    NETWORK_INTERFACE_DESCRIPTION* result;
    
    if ((result = (NETWORK_INTERFACE_DESCRIPTION*)malloc(sizeof(NETWORK_INTERFACE_DESCRIPTION))) == NULL)
    {
        LogError("Failed allocating NETWORK_INTERFACE_DESCRIPTION");
    }
    else if ((result->name = (char*)malloc(sizeof(char) * (strlen(ifr->ifr_name) + 1))) == NULL)
    {
        LogError("failed setting interface description name (malloc failed)");
        destroy_network_interface_descriptions(result);
        result = NULL;
    }
    else if (strcpy(result->name, ifr->ifr_name) == NULL)
    {
        LogError("failed setting interface description name (strcpy failed)");
        destroy_network_interface_descriptions(result);
        result = NULL;
    }
    else
    {
        char* ip_address;
        unsigned char* mac = (unsigned char*)ifr->ifr_hwaddr.sa_data;
        
        if ((result->mac_address = (char*)malloc(sizeof(char) * MAC_ADDRESS_STRING_LENGTH)) == NULL)
        {
            LogError("failed formatting mac address (malloc failed)");
            destroy_network_interface_descriptions(result);
            result = NULL;
        }
        else if (sprintf(result->mac_address, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]) <= 0)
        {
            LogError("failed formatting mac address (sprintf failed)");
            destroy_network_interface_descriptions(result);
            result = NULL;
        }
        else if ((ip_address = inet_ntoa(((struct sockaddr_in*)&ifr->ifr_addr)->sin_addr)) == NULL)
        {
            LogError("failed setting the ip address (inet_ntoa failed)");
            destroy_network_interface_descriptions(result);
            result = NULL;
        }
        else if ((result->ip_address = (char*)malloc(sizeof(char) * (strlen(ip_address) + 1))) == NULL)
        {
            LogError("failed setting the ip address (malloc failed)");
            destroy_network_interface_descriptions(result);
            result = NULL;
        }
        else if (strcpy(result->ip_address, ip_address) == NULL)
        {
            LogError("failed setting the ip address (strcpy failed)");
            destroy_network_interface_descriptions(result);
            result = NULL;
        }
        else
        {
            result->next = NULL;
            
            if (previous_nid != NULL)
            {
                previous_nid->next = result;
            }
        }
    }
    
    return result;
}

static int get_network_interface_descriptions(int socket, NETWORK_INTERFACE_DESCRIPTION** nid)
{
    int result;
    
    struct ifreq ifr;
    struct ifconf ifc;
    char buf[IFREQ_BUFFER_SIZE];
    
    ifc.ifc_len = sizeof(buf);
    ifc.ifc_buf = buf;
    
    if (ioctl(socket, SIOCGIFCONF, &ifc) == -1)
    {
        LogError("ioctl failed querying socket (SIOCGIFCONF, errno=%s)", errno);
        result = __FAILURE__;
    }
    else
    {
        NETWORK_INTERFACE_DESCRIPTION* root_nid = NULL;
        NETWORK_INTERFACE_DESCRIPTION* new_nid = NULL;
        
        struct ifreq* it = ifc.ifc_req;
        const struct ifreq* const end = it + (ifc.ifc_len / sizeof(struct ifreq));
        
        result = 0;
        
        for (; it != end; ++it)
        {
            strcpy(ifr.ifr_name, it->ifr_name);
            
            if (ioctl(socket, SIOCGIFFLAGS, &ifr) != 0)
            {
                LogError("ioctl failed querying socket (SIOCGIFFLAGS, errno=%d)", errno);
                result = __FAILURE__;
                break;
            }
            else if (ioctl(socket, SIOCGIFHWADDR, &ifr) != 0)
            {
                LogError("ioctl failed querying socket (SIOCGIFHWADDR, errno=%d)", errno);
                result = __FAILURE__;
                break;
            }
            else if (ioctl(socket, SIOCGIFADDR, &ifr) != 0)
            {
                LogError("ioctl failed querying socket (SIOCGIFADDR, errno=%d)", errno);
                result = __FAILURE__;
                break;
            }
            else if ((new_nid = create_network_interface_description(&ifr, new_nid)) == NULL)
            {
                LogError("Failed creating network interface description");
                result = __FAILURE__;
                break;
            }
            else if (root_nid == NULL)
            {
                root_nid = new_nid;
            }
        }
        
        if (result == 0)
        {
            *nid = root_nid;
        }
        else
        {
            destroy_network_interface_descriptions(root_nid);
        }
    }
    
    return result;
}

static int set_target_network_interface(int socket, char* mac_address)
{
    int result;
    NETWORK_INTERFACE_DESCRIPTION* nid;
    
    if (get_network_interface_descriptions(socket, &nid) != 0)
    {
        LogError("Failed getting network interface descriptions");
        result = __FAILURE__;
    }
    else
    {
        NETWORK_INTERFACE_DESCRIPTION* current_nid = nid;
        
        while(current_nid != NULL)
        {
            if (strcmp(mac_address, current_nid->mac_address) == 0)
            {
                break;
            }
            
            current_nid = current_nid->next;
        }
        
        if (current_nid == NULL)
        {
            LogError("Did not find a network interface matching MAC ADDRESS");
            result = __FAILURE__;
        }
        else if (setsockopt(socket, SOL_SOCKET, SO_BINDTODEVICE, current_nid->name, strlen(current_nid->name)) != 0)
        {
            LogError("setsockopt failed (%d)", errno);
            result = __FAILURE__;
        }
        else
        {
            result = 0;
        }
        
        destroy_network_interface_descriptions(nid);
    }
    
    return result;
}
#endif //__APPLE__

CONCRETE_IO_HANDLE socketio_create(void* io_create_parameters)
{
    SOCKETIO_CONFIG* socket_io_config = io_create_parameters;
    SOCKET_IO_INSTANCE* result;
    
    if (socket_io_config == NULL)
    {
        LogError("Invalid argument: socket_io_config is NULL");
        result = NULL;
    }
    else
    {
        result = malloc(sizeof(SOCKET_IO_INSTANCE));
        if (result != NULL)
        {
            result->pending_io_list = singlylinkedlist_create();

            if (result->pending_io_list == NULL)
            {
                LogError("Failure: singlylinkedlist_create unable to create pending list.");
                free(result);
                result = NULL;
            }
            else
            {
                result->sockRead = NULL;
                result->sockWrite = NULL;

                if (socket_io_config->hostname != NULL)
                {
                    result->hostname = (char*)malloc(strlen(socket_io_config->hostname) + 1);
                    if (result->hostname != NULL)
                    {
                        (void)strcpy(result->hostname, socket_io_config->hostname);
                    }
                }
                else if (socket_io_config->accepted_socket != NULL)
                {
                    result->hostname = NULL;
                    result->sockRead = ((ACCEPTED_SOCKET_INSTANCE *)socket_io_config->accepted_socket)->sockRead;
                    result->sockWrite = ((ACCEPTED_SOCKET_INSTANCE *)socket_io_config->accepted_socket)->sockWrite;
                }
                
                if ((result->hostname == NULL) && (result->sockRead == NULL))
                {
                    LogError("Failure: hostname == NULL and socket is invalid.");
                    singlylinkedlist_destroy(result->pending_io_list);
                    free(result);
                    result = NULL;
                }
                else
                {
                    result->port = socket_io_config->port;
                    result->target_mac_address = NULL;
                    result->on_bytes_received = NULL;
                    result->on_io_error = NULL;
                    result->on_bytes_received_context = NULL;
                    result->on_io_error_context = NULL;
                    result->io_state = IO_STATE_CLOSED;
                }
            }
        }
        else
        {
            LogError("Allocation Failure: SOCKET_IO_INSTANCE");
        }
    }
    
    return result;
}

void socketio_destroy(CONCRETE_IO_HANDLE socket_io)
{
    if (socket_io != NULL)
    {
        SOCKET_IO_INSTANCE* socket_io_instance = (SOCKET_IO_INSTANCE*)socket_io;
        /* we cannot do much if the close fails, so just ignore the result */
        if (socket_io_instance->sockRead != NULL)
        {
            CFReadStreamClose(socket_io_instance->sockRead);
            CFWriteStreamClose(socket_io_instance->sockWrite);
        }
        
        /* clear allpending IOs */
        LIST_ITEM_HANDLE first_pending_io = singlylinkedlist_get_head_item(socket_io_instance->pending_io_list);
        while (first_pending_io != NULL)
        {
            PENDING_SOCKET_IO* pending_socket_io = (PENDING_SOCKET_IO*)singlylinkedlist_item_get_value(first_pending_io);
            if (pending_socket_io != NULL)
            {
                free(pending_socket_io->bytes);
                free(pending_socket_io);
            }
            
            (void)singlylinkedlist_remove(socket_io_instance->pending_io_list, first_pending_io);
            first_pending_io = singlylinkedlist_get_head_item(socket_io_instance->pending_io_list);
        }
        
        singlylinkedlist_destroy(socket_io_instance->pending_io_list);
        free(socket_io_instance->hostname);
        free(socket_io_instance->target_mac_address);
        free(socket_io);
    }
}

int socketio_open(CONCRETE_IO_HANDLE socket_io, ON_IO_OPEN_COMPLETE on_io_open_complete, void* on_io_open_complete_context, ON_BYTES_RECEIVED on_bytes_received, void* on_bytes_received_context, ON_IO_ERROR on_io_error, void* on_io_error_context)
{
    int result;
    
    SOCKET_IO_INSTANCE* socket_io_instance = (SOCKET_IO_INSTANCE*)socket_io;
    
    if (socket_io == NULL)
    {
        LogError("Invalid argument: SOCKET_IO_INSTANCE is NULL");
        result = __FAILURE__;
    }
    else
    {
        if (socket_io_instance->io_state != IO_STATE_CLOSED)
        {
            LogError("Failure: socket state is not closed.");
            result = __FAILURE__;
        }
        else if (socket_io_instance->sockRead != NULL && socket_io_instance->sockWrite != NULL)
        {
            // Opening an accepted socket
            socket_io_instance->on_bytes_received_context = on_bytes_received_context;
            socket_io_instance->on_bytes_received = on_bytes_received;
            socket_io_instance->on_io_error = on_io_error;
            socket_io_instance->on_io_error_context = on_io_error_context;
            socket_io_instance->on_open_complete = on_io_open_complete;
            socket_io_instance->on_open_complete_context = on_io_open_complete_context;
            
            socket_io_instance->io_state = IO_STATE_OPEN;
            
            result = 0;
        }
        else
        {
            CFStringRef host = CFStringCreateWithCString(NULL, socket_io_instance->hostname, kCFStringEncodingUTF8);
            CFStreamCreatePairWithSocketToHost(NULL, host, socket_io_instance->port, &socket_io_instance->sockRead, &socket_io_instance->sockWrite);
            
            if (!CFReadStreamSetProperty(socket_io_instance->sockRead, kCFStreamPropertySSLSettings, kCFStreamSocketSecurityLevelTLSv1))
            {
                LogError("Failure: SSL property was unrecognized");
                CFReadStreamClose(socket_io_instance->sockRead);
                CFWriteStreamClose(socket_io_instance->sockWrite);
                socket_io_instance->sockRead = NULL;
                socket_io_instance->sockWrite = NULL;
                result = __FAILURE__;
            }
            else if (!CFReadStreamOpen(socket_io_instance->sockRead) || !CFWriteStreamOpen(socket_io_instance->sockWrite))
            {
                LogError("Failure: connect failure");
                CFReadStreamClose(socket_io_instance->sockRead);
                CFWriteStreamClose(socket_io_instance->sockWrite);
                socket_io_instance->sockRead = NULL;
                socket_io_instance->sockWrite = NULL;
                result = __FAILURE__;
            }
            else
            {
                socket_io_instance->io_state = IO_STATE_OPENING;
                socket_io_instance->on_bytes_received = on_bytes_received;
                socket_io_instance->on_bytes_received_context = on_bytes_received_context;
                
                socket_io_instance->on_io_error = on_io_error;
                socket_io_instance->on_io_error_context = on_io_error_context;
                
                socket_io_instance->on_open_complete = on_io_open_complete;
                socket_io_instance->on_open_complete_context = on_io_open_complete_context;

                result = 0;
            }
        }
    }
    
    return result;
}

int socketio_close(CONCRETE_IO_HANDLE socket_io, ON_IO_CLOSE_COMPLETE on_io_close_complete, void* callback_context)
{
    int result = 0;
    
    if (socket_io == NULL)
    {
        result = __FAILURE__;
    }
    else
    {
        SOCKET_IO_INSTANCE* socket_io_instance = (SOCKET_IO_INSTANCE*)socket_io;
        
        if ((socket_io_instance->io_state != IO_STATE_CLOSED) && (socket_io_instance->io_state != IO_STATE_CLOSING))
        {
            // Only close if the socket isn't already in the closed or closing state
            CFReadStreamClose(socket_io_instance->sockRead);
            CFWriteStreamClose(socket_io_instance->sockWrite);
            socket_io_instance->sockRead = NULL;
            socket_io_instance->sockWrite = NULL;
            socket_io_instance->io_state = IO_STATE_CLOSED;
        }
        
        if (on_io_close_complete != NULL)
        {
            on_io_close_complete(callback_context);
        }
        
        result = 0;
    }
    
    return result;
}

int socketio_send(CONCRETE_IO_HANDLE socket_io, const void* buffer, size_t size, ON_SEND_COMPLETE on_send_complete, void* callback_context)
{
    int result;
    
    if ((socket_io == NULL) ||
        (buffer == NULL) ||
        (size == 0))
    {
        /* Invalid arguments */
        LogError("Invalid argument: send given invalid parameter");
        result = __FAILURE__;
    }
    else
    {
        SOCKET_IO_INSTANCE* socket_io_instance = (SOCKET_IO_INSTANCE*)socket_io;
        if (socket_io_instance->io_state != IO_STATE_OPEN && socket_io_instance->io_state != IO_STATE_OPENING)
        {
            LogError("Failure: socket state is not opened.");
            result = __FAILURE__;
        }
        else
        {
            LIST_ITEM_HANDLE first_pending_io = singlylinkedlist_get_head_item(socket_io_instance->pending_io_list);
            if (first_pending_io != NULL || socket_io_instance->io_state == IO_STATE_OPENING)
            {
                if (add_pending_io(socket_io_instance, buffer, size, on_send_complete, callback_context) != 0)
                {
                    LogError("Failure: add_pending_io failed.");
                    result = __FAILURE__;
                }
                else
                {
                    result = 0;
                }
            }
            else
            {
                CFIndex send_result = CFWriteStreamWrite(socket_io_instance->sockWrite, buffer, size);
                
                if (send_result != size)
                {
                    if (send_result == -1)
                    {
                        indicate_error(socket_io_instance);
                        LogError("Failure: sending socket failed. errno=%d.", CFWriteStreamGetStatus(socket_io_instance->sockWrite));
                        result = __FAILURE__;
                    }
                    else
                    {
                        /* queue data */
                        if (add_pending_io(socket_io_instance, buffer + send_result, size - send_result, on_send_complete, callback_context) != 0)
                        {
                            LogError("Failure: add_pending_io failed.");
                            result = __FAILURE__;
                        }
                        else
                        {
                            result = 0;
                        }
                    }
                }
                else
                {
                    if (on_send_complete != NULL)
                    {
                        on_send_complete(callback_context, IO_SEND_OK);
                    }
                    
                    result = 0;
                }
            }
        }
    }
    
    return result;
}

void socketio_dowork(CONCRETE_IO_HANDLE socket_io)
{
    if (socket_io != NULL)
    {
        SOCKET_IO_INSTANCE* socket_io_instance = (SOCKET_IO_INSTANCE*)socket_io;
        
        if (socket_io_instance->io_state == IO_STATE_OPENING)
        {
            CFStreamStatus status = CFReadStreamGetStatus(socket_io_instance->sockRead);
            
            switch (status)
            {
                case kCFStreamStatusNotOpen:
                case kCFStreamStatusError:
                    socket_io_instance->io_state = IO_STATE_ERROR;
                    indicate_error(socket_io_instance);
                    LogError("Socket failed to open");
                    break;
                case kCFStreamStatusClosed:
                    socket_io_instance->io_state = IO_STATE_CLOSED;
                    
                    if (socket_io_instance->on_open_complete != NULL)
                    {
                        socket_io_instance->on_open_complete(socket_io_instance->on_open_complete_context, IO_OPEN_CANCELLED);
                    }
                    break;
                case kCFStreamStatusOpen:
                case kCFStreamStatusOpening:
                case kCFStreamStatusReading:
                case kCFStreamStatusWriting:
                case kCFStreamStatusAtEnd:
                    socket_io_instance->io_state = IO_STATE_OPEN;
                    
                    if (socket_io_instance->on_open_complete != NULL)
                    {
                        socket_io_instance->on_open_complete(socket_io_instance->on_open_complete_context, IO_OPEN_OK);
                    }
                    break;
                default:
                    socket_io_instance->io_state = IO_STATE_ERROR;
                    indicate_error(socket_io_instance);
                    LogError("Socket failed to open");
                    break;
            }
        }
        
        if (socket_io_instance->io_state == IO_STATE_OPEN)
        {
            CFIndex received;
            
            LIST_ITEM_HANDLE first_pending_io = singlylinkedlist_get_head_item(socket_io_instance->pending_io_list);
            while (first_pending_io != NULL)
            {
                PENDING_SOCKET_IO* pending_socket_io = (PENDING_SOCKET_IO*)singlylinkedlist_item_get_value(first_pending_io);
                if (pending_socket_io == NULL)
                {
                    socket_io_instance->io_state = IO_STATE_ERROR;
                    indicate_error(socket_io_instance);
                    LogError("Failure: retrieving socket from list");
                    break;
                }
                
                CFIndex send_result = CFWriteStreamWrite(socket_io_instance->sockWrite, pending_socket_io->bytes, pending_socket_io->size);
                if (send_result != pending_socket_io->size)
                {
                    if (send_result == -1)
                    {
                        free(pending_socket_io->bytes);
                        free(pending_socket_io);
                        (void)singlylinkedlist_remove(socket_io_instance->pending_io_list, first_pending_io);
                            
                        LogError("Failure: sending Socket information. errno=%d (%s).", errno, strerror(errno));
                        socket_io_instance->io_state = IO_STATE_ERROR;
                        indicate_error(socket_io_instance);
                    }
                    else
                    {
                        /* simply wait until next dowork */
                        (void)memmove(pending_socket_io->bytes, pending_socket_io->bytes + send_result, pending_socket_io->size - send_result);
                        pending_socket_io->size -= send_result;
                        break;
                    }
                }
                else
                {
                    if (pending_socket_io->on_send_complete != NULL)
                    {
                        pending_socket_io->on_send_complete(pending_socket_io->callback_context, IO_SEND_OK);
                    }
                    
                    free(pending_socket_io->bytes);
                    free(pending_socket_io);
                    if (singlylinkedlist_remove(socket_io_instance->pending_io_list, first_pending_io) != 0)
                    {
                        socket_io_instance->io_state = IO_STATE_ERROR;
                        indicate_error(socket_io_instance);
                        LogError("Failure: unable to remove socket from list");
                    }
                }
                
                first_pending_io = singlylinkedlist_get_head_item(socket_io_instance->pending_io_list);
            }
            
            while (CFReadStreamHasBytesAvailable(socket_io_instance->sockRead))
            {
                unsigned char* recv_bytes = malloc(RECEIVE_BYTES_VALUE);
                if (recv_bytes == NULL)
                {
                    LogError("Socketio_Failure: NULL allocating input buffer.");
                    indicate_error(socket_io_instance);
                }
                else
                {
                    received = CFReadStreamRead(socket_io_instance->sockRead, recv_bytes, (CFIndex)RECEIVE_BYTES_VALUE);
                    if (received > 0)
                    {
                        if (socket_io_instance->on_bytes_received != NULL)
                        {
                            /* explictly ignoring here the result of the callback */
                            (void)socket_io_instance->on_bytes_received(socket_io_instance->on_bytes_received_context, recv_bytes, received);
                        }
                    }
                    free(recv_bytes);
                }
            }
        }
    }
}

int socketio_setoption(CONCRETE_IO_HANDLE socket_io, const char* optionName, const void* value)
{
    int result;
    
    if (socket_io == NULL ||
        optionName == NULL ||
        value == NULL)
    {
        result = __FAILURE__;
    }
    else
    {
        // Currently iOS does not support any socket options
        result = __FAILURE__;
    }

    return result;
/*
        SOCKET_IO_INSTANCE* socket_io_instance = (SOCKET_IO_INSTANCE*)socket_io;
        
        if (strcmp(optionName, "tcp_keepalive") == 0)
        {
            result = setsockopt(socket_io_instance->socket, SOL_SOCKET, SO_KEEPALIVE, value, sizeof(int));
            if (result == -1) result = errno;
        }
        else if (strcmp(optionName, "tcp_keepalive_time") == 0)
        {
#ifdef __APPLE__
            result = setsockopt(socket_io_instance->socket, IPPROTO_TCP, TCP_KEEPALIVE, value, sizeof(int));
#else
            result = setsockopt(socket_io_instance->socket, SOL_TCP, TCP_KEEPIDLE, value, sizeof(int));
#endif
            if (result == -1) result = errno;
        }
        else if (strcmp(optionName, "tcp_keepalive_interval") == 0)
        {
            result = setsockopt(socket_io_instance->socket, SOL_TCP, TCP_KEEPINTVL, value, sizeof(int));
            if (result == -1) result = errno;
        }
        else if (strcmp(optionName, OPTION_NET_INT_MAC_ADDRESS) == 0)
        {
#ifdef __APPLE__
            LogError("option not supported.");
            result = __FAILURE__;
#else
            if (strlen(value) == 0)
            {
                LogError("option value must be a valid mac address");
                result = __FAILURE__;
            }
            else if ((socket_io_instance->target_mac_address = (char*)malloc(sizeof(char) * (strlen(value) + 1))) == NULL)
            {
                LogError("failed setting net_interface_mac_address option (malloc failed)");
                result = __FAILURE__;
            }
            else if (strcpy(socket_io_instance->target_mac_address, value) == NULL)
            {
                LogError("failed setting net_interface_mac_address option (strcpy failed)");
                free(socket_io_instance->target_mac_address);
                socket_io_instance->target_mac_address = NULL;
                result = __FAILURE__;
            }
            else
            {
                strtoup(socket_io_instance->target_mac_address);
                result = 0;
            }
#endif
        }
        else
        {
            result = __FAILURE__;
        }
    }
    return result;
*/
}

const IO_INTERFACE_DESCRIPTION* socketio_get_interface_description(void)
{
    return &socket_io_interface_description;
}

