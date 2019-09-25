// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

// Copyright (c) Express Logic.  All rights reserved.
// Please contact support@expresslogic.com for any questions or use the support portal at www.rtos.com

/* This file is used for porting socketio between threadx and azure-iot-sdk-c.  */

#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include "nx_api.h"
#include "nx_ip.h"
#include "nxd_dns.h"
#include "nx_secure_tls_api.h"
#include "azure_c_shared_utility/socketio.h"
#include "azure_c_shared_utility/singlylinkedlist.h"
#include "azure_c_shared_utility/tcpsocketconnection_c.h"
#include "azure_c_shared_utility/optimize_size.h"
#include "azure_c_shared_utility/xlogging.h"

#define UNABLE_TO_COMPLETE -2

NX_TCP_SOCKET *_threadx_tcp_socket_created_ptr;   /* ThreadX TCP Socket.  */
extern NX_DNS *_threadx_dns_client_created_ptr;   /* ThreadX DNS Client.  */

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
    TCPSOCKETCONNECTION_HANDLE tcp_socket_connection;
    ON_BYTES_RECEIVED on_bytes_received;
    ON_IO_ERROR on_io_error;
    void* on_bytes_received_context;
    void* on_io_error_context;
    char* hostname;
    NXD_ADDRESS ip_address;
    int port;
    IO_STATE io_state;
    SINGLYLINKEDLIST_HANDLE pending_io_list;
    NX_TCP_SOCKET threadx_tcp_socket;
} SOCKET_IO_INSTANCE;

/*this function will clone an option given by name and value*/
static void* socketio_CloneOption(const char* name, const void* value)
{
    (void)name;
    (void)value;
    return NULL;
}

/*this function destroys an option previously created*/
static void socketio_DestroyOption(const char* name, const void* value)
{
    (void)name;
    (void)value;
}

static OPTIONHANDLER_HANDLE socketio_retrieveoptions(CONCRETE_IO_HANDLE socket_io)
{
    OPTIONHANDLER_HANDLE result;
    (void)socket_io;
    result = OptionHandler_Create(socketio_CloneOption, socketio_DestroyOption, socketio_setoption);
    if (result == NULL)
    {
        /*return as is*/
    }
    else
    {
        /*insert here work to add the options to "result" handle*/
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
        result = MU_FAILURE;
    }
    else
    {
        pending_socket_io->bytes = (unsigned char*)malloc(size);
        if (pending_socket_io->bytes == NULL)
        {
            free(pending_socket_io);
            result = MU_FAILURE;
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
                free(pending_socket_io->bytes);
                free(pending_socket_io);
                result = MU_FAILURE;
            }
            else
            {
                result = 0;
            }
        }
    }

    return result;
}

CONCRETE_IO_HANDLE socketio_create(void* io_create_parameters)
{
    SOCKETIO_CONFIG* socket_io_config = io_create_parameters;
    SOCKET_IO_INSTANCE* result = NX_NULL;

    if (socket_io_config == NULL)
    {
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
                free(result);
                result = NULL;
            }
            else
            {
                if (socket_io_config->hostname != NULL)
                {
                    result->hostname = (char*)malloc(strlen(socket_io_config->hostname) + 1);
                }
                else
                {
                    result->hostname = NULL;
                }

                if (result->hostname == NULL)
                {
                    LogError("Failure: hostname == NULL.");
                    singlylinkedlist_destroy(result->pending_io_list);
                    free(result);
                    result = NULL;
                }
                else
                {
                    strcpy(result->hostname, socket_io_config->hostname);
                    result->port = socket_io_config->port;
                    result->on_bytes_received = NULL;
                    result->on_io_error = NULL;
                    result->on_bytes_received_context = NULL;
                    result->on_io_error_context = NULL;
                    result->io_state = IO_STATE_CLOSED;
                    result->tcp_socket_connection = NULL;
                }
            }
        }
    }

    return result;
}

void socketio_destroy(CONCRETE_IO_HANDLE socket_io)
{

    if (socket_io != NULL)
    {
        socketio_close(socket_io, NULL, NULL);
        free(socket_io);
    }
}

int threadx_host_address_get(NXD_ADDRESS *host_address, const char* host_name)
{

  
    /* Look up an IPv4 address over IPv4. */
    if (nxd_dns_host_by_name_get(_threadx_dns_client_created_ptr, (UCHAR *)host_name, host_address, NX_IP_PERIODIC_RATE, NX_IP_VERSION_V4))
    {
        LOG(AZ_LOG_ERROR, LOG_LINE, "Failed to get host address");
        return(MU_FAILURE);
    }
    
    return(0);
}

int socketio_open(CONCRETE_IO_HANDLE socket_io, ON_IO_OPEN_COMPLETE on_io_open_complete, void* on_io_open_complete_context, ON_BYTES_RECEIVED on_bytes_received, void* on_bytes_received_context, ON_IO_ERROR on_io_error, void* on_io_error_context)
{
    int result = NX_NULL;

    SOCKET_IO_INSTANCE* socket_io_instance = (SOCKET_IO_INSTANCE*)socket_io;
    if (socket_io == NULL)
    {
        result = MU_FAILURE;
    }
    else
    {

        /* Create the socket.  */
        _threadx_tcp_socket_created_ptr = NULL;        
        if (nx_tcp_socket_create(_nx_ip_created_ptr, &(socket_io_instance -> threadx_tcp_socket), "THREADX TCP Socket",
                                 NX_IP_NORMAL, NX_DONT_FRAGMENT, 0x80, 8192,
                                 NX_NULL, NX_NULL) != 0)
        {
            socket_io_instance->tcp_socket_connection = NX_NULL;
        }
        else
        {
            socket_io_instance->tcp_socket_connection = &(socket_io_instance -> threadx_tcp_socket);
        }

        if (socket_io_instance->tcp_socket_connection == NULL)
        {
            result = MU_FAILURE;
        }
        else
        {

            /* First attempt to bind the client socket. */
            if (nx_tcp_client_socket_bind(&(socket_io_instance -> threadx_tcp_socket), NX_ANY_PORT, NX_WAIT_FOREVER))
            {
                nx_tcp_socket_delete(&(socket_io_instance -> threadx_tcp_socket));
                socket_io_instance->tcp_socket_connection = NULL;
                result = MU_FAILURE;
            }
            else
            {
#ifdef THREADX_AZURE_IP_ADDRESS        
                socket_io_instance -> ip_address.nxd_ip_version = NX_IP_VERSION_V4;
                socket_io_instance -> ip_address.nxd_ip_address.v4 = THREADX_AZURE_IP_ADDRESS;
#else
                /* Get the Azure address.  */
                if (threadx_host_address_get(&(socket_io_instance -> ip_address), socket_io_instance -> hostname))
                {
                    nx_tcp_client_socket_unbind(&(socket_io_instance -> threadx_tcp_socket));
                    nx_tcp_socket_delete(&(socket_io_instance -> threadx_tcp_socket));
                    socket_io_instance->tcp_socket_connection = NULL;
                    result = MU_FAILURE;
                }
                else
#endif
                {

                    /* Connect to the Azure server */
                    if (nxd_tcp_client_socket_connect(&(socket_io_instance -> threadx_tcp_socket), &(socket_io_instance -> ip_address), socket_io_instance -> port, NX_WAIT_FOREVER) != 0)
                    {
                        nx_tcp_client_socket_unbind(&(socket_io_instance -> threadx_tcp_socket));
                        nx_tcp_socket_delete(&(socket_io_instance -> threadx_tcp_socket));
                        socket_io_instance->tcp_socket_connection = NULL;
                        result = MU_FAILURE;
                    }
                    else
                    {

                        socket_io_instance->on_bytes_received = on_bytes_received;
                        socket_io_instance->on_bytes_received_context = on_bytes_received_context;

                        socket_io_instance->on_io_error = on_io_error;
                        socket_io_instance->on_io_error_context = on_io_error_context;

                        socket_io_instance->io_state = IO_STATE_OPEN;

                        /* Transfer tcp socket to on_io_open_complete.  */
                        _threadx_tcp_socket_created_ptr = &(socket_io_instance -> threadx_tcp_socket);
                        result = 0;
                    }
                }
            }
        }
    
        if (on_io_open_complete != NULL)
        {
            on_io_open_complete(on_io_open_complete_context, result == 0 ? IO_OPEN_OK : IO_OPEN_ERROR);
        }
    }

    return result;
}

int socketio_close(CONCRETE_IO_HANDLE socket_io, ON_IO_CLOSE_COMPLETE on_io_close_complete, void* callback_context)
{
    int result = 0;

    if (socket_io == NULL)
    {
        result = MU_FAILURE;
    }
    else
    {
        SOCKET_IO_INSTANCE* socket_io_instance = (SOCKET_IO_INSTANCE*)socket_io;

        if ((socket_io_instance->io_state == IO_STATE_CLOSED) ||
            (socket_io_instance->io_state == IO_STATE_CLOSING))
        {
            result = MU_FAILURE;
        }
        else
        {

            /* THREADX socket close.  */
            nx_tcp_socket_disconnect(&(socket_io_instance -> threadx_tcp_socket), NX_NO_WAIT);
            nx_tcp_client_socket_unbind(&(socket_io_instance -> threadx_tcp_socket));
            nx_tcp_socket_delete(&(socket_io_instance -> threadx_tcp_socket));
            socket_io_instance->io_state = IO_STATE_CLOSED;

            if (on_io_close_complete != NULL)
            {
                on_io_close_complete(callback_context);
            }

            result = 0;
        }
    }

    return result;
}

int socketio_send(CONCRETE_IO_HANDLE socket_io, const void* buffer, size_t size, ON_SEND_COMPLETE on_send_complete, void* callback_context)
{
    int result = NX_NULL;

    if ((socket_io == NULL) ||
        (buffer == NULL) ||
        (size == 0))
    {
        /* Invalid arguments */
        result = MU_FAILURE;
    }
    else
    {
        SOCKET_IO_INSTANCE* socket_io_instance = (SOCKET_IO_INSTANCE*)socket_io;
        if (socket_io_instance->io_state != IO_STATE_OPEN)
        {
            result = MU_FAILURE;
        }
        else
        {
            LIST_ITEM_HANDLE first_pending_io = singlylinkedlist_get_head_item(socket_io_instance->pending_io_list);
            if (first_pending_io != NULL)
            {
                if (add_pending_io(socket_io_instance, buffer, size, on_send_complete, callback_context) != 0)
                {
                    result = MU_FAILURE;
                }
                else
                {
                    result = 0;
                }
            }
            else
            {

                /* THREADX send tcp data.  */
                int send_result = 0;
                UINT status;
                NX_PACKET *my_packet;
                UINT packet_available_size;

                while(send_result < (int)size)
                {

                    /* Allocate packet.  */
                    status = nx_packet_allocate(_nx_ip_created_ptr -> nx_ip_default_packet_pool, &my_packet, NX_TCP_PACKET, NX_NO_WAIT);

                    /* Check status.  */
                    if (status)
                    {
                        break;
                    }
                    else
                    {

                        /* Compute the available size for packet.  */
                        packet_available_size = (my_packet -> nx_packet_data_end - my_packet -> nx_packet_append_ptr);

                        /* Check if packet can fill all data.  */
                        if ((size - send_result) < packet_available_size)
                             packet_available_size = size - send_result;

                        /* Fill the message.  */
                        memcpy(my_packet -> nx_packet_append_ptr, (UCHAR *)buffer + send_result, packet_available_size); 
                        my_packet -> nx_packet_length = packet_available_size;
                        my_packet -> nx_packet_append_ptr += my_packet -> nx_packet_length;

                        /* Send out the packet.  */
                        status = nx_tcp_socket_send(&(socket_io_instance ->threadx_tcp_socket), my_packet, NX_NO_WAIT);

                        /* Check status.  */
                        if (status)
                        {

                            /* Release the packet.  */
                            nx_packet_release(my_packet);
                            break;
                        }
                        else
                        {

                            /* Update the send result.  */
                            send_result += packet_available_size;
                        }
                    }
                }

                if (send_result != size)
                {
                    if (send_result < 0)
                    {
                        send_result = 0;
                    }

                    /* queue data */
                    if (add_pending_io(socket_io_instance, (unsigned char*)buffer + send_result, size - send_result, on_send_complete, callback_context) != 0)
                    {
                        result = MU_FAILURE;
                    }
                    else
                    {
                        result = 0;
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

/* THREADX socket send and receive.  */
UINT status;
NX_PACKET *my_packet;
NX_PACKET *release_packet;
UINT packet_available_size;

    if (socket_io != NULL)
    {
        SOCKET_IO_INSTANCE* socket_io_instance = (SOCKET_IO_INSTANCE*)socket_io;
        if (socket_io_instance->io_state == IO_STATE_OPEN)
        {

            LIST_ITEM_HANDLE first_pending_io = singlylinkedlist_get_head_item(socket_io_instance->pending_io_list);
            while (first_pending_io != NULL)
            {
                PENDING_SOCKET_IO* pending_socket_io = (PENDING_SOCKET_IO*)singlylinkedlist_item_get_value(first_pending_io);
                if (pending_socket_io == NULL)
                {
                    socket_io_instance->io_state = IO_STATE_ERROR;
                    indicate_error(socket_io_instance);
                    break;
                }

                /* THREADX send tcp data.  */
                int send_result = 0;

                while(send_result < (int)pending_socket_io->size)
                {

                    /* Allocate packet.  */
                    status = nx_packet_allocate(_nx_ip_created_ptr -> nx_ip_default_packet_pool, &my_packet, NX_TCP_PACKET, NX_NO_WAIT);

                    /* Check status.  */
                    if (status)
                    {
                        break;
                    }
                    else
                    {

                        /* Compute the available size for packet.  */
                        packet_available_size = (my_packet -> nx_packet_data_end - my_packet -> nx_packet_append_ptr);

                        /* Check if packet can fill all data.  */
                        if ((pending_socket_io->size - send_result) < packet_available_size)
                             packet_available_size = pending_socket_io->size - send_result;

                        /* Fill the message.  */
                        memcpy(my_packet -> nx_packet_append_ptr, pending_socket_io->bytes + send_result, packet_available_size); 
                        my_packet -> nx_packet_length = packet_available_size;
                        my_packet -> nx_packet_append_ptr += my_packet -> nx_packet_length;

                        /* Send out the packet.  */
                        status = nx_tcp_socket_send(&(socket_io_instance ->threadx_tcp_socket), my_packet, NX_NO_WAIT);

                        /* Check status.  */
                        if (status)
                        {

                            /* Release the packet.  */
                            nx_packet_release(my_packet);
                            break;
                        }
                        else
                        {

                            /* Update the send result.  */
                            send_result += packet_available_size;
                        }
                    }
                }

                if (send_result != pending_socket_io->size)
                {
                    if (send_result < 0)
                    {
                        if (send_result < UNABLE_TO_COMPLETE)
                        {
                            // Bad error.  Indicate as much.
                            socket_io_instance->io_state = IO_STATE_ERROR;
                            indicate_error(socket_io_instance);
                        }
                        break;
                    }
                    else
                    {
                        /* send something, wait for the rest */
                        (void)memmove(pending_socket_io->bytes, pending_socket_io->bytes + send_result, pending_socket_io->size - send_result);
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
                    }
                }

                first_pending_io = singlylinkedlist_get_head_item(socket_io_instance->pending_io_list);
            }

            /* THREADX receive tcp data.  */
            status = NX_SUCCESS;
            while ((status == NX_SUCCESS) && (socket_io_instance->io_state == IO_STATE_OPEN))
            {

                /* Receive the data.  */
                status = nx_tcp_socket_receive(&(socket_io_instance ->threadx_tcp_socket), &my_packet, NX_NO_WAIT);

                /* Check status.  */
                if (status == NX_SUCCESS)
                {
                    release_packet = my_packet;
                    if (socket_io_instance->on_bytes_received != NULL)
                    {

                        ULONG  received;
#ifndef NX_DISABLE_PACKET_CHAIN
                        /* Loop to copy bytes from packet(s).  */
                        while (my_packet)
                        {
#endif /* NX_DISABLE_PACKET_CHAIN */

                            /* Calculate the bytes to copy in this packet. */
                            /*lint -e{946} -e{947} suppress pointer subtraction, since it is necessary. */
                            received = (ULONG)(my_packet -> nx_packet_append_ptr - my_packet -> nx_packet_prepend_ptr);

                            /* explictly ignoring here the result of the callback */
                            (void)socket_io_instance->on_bytes_received(socket_io_instance->on_bytes_received_context, my_packet -> nx_packet_prepend_ptr, received);
#ifndef NX_DISABLE_PACKET_CHAIN
                            /* Move to next packet.  */
                            my_packet =  my_packet -> nx_packet_next;
                        }
#endif /* NX_DISABLE_PACKET_CHAIN */
                    }

                    /* Release the packet.  */
                    nx_packet_release(release_packet);
                }
            }
        }
    }
}

int socketio_setoption(CONCRETE_IO_HANDLE socket_io, const char* optionName, const void* value)
{
    /* Not implementing any options */
    return MU_FAILURE;
}

const IO_INTERFACE_DESCRIPTION* socketio_get_interface_description(void)
{
    return &socket_io_interface_description;
}

