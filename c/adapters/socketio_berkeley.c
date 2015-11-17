// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdlib.h>
#ifdef _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

//#define _POSIX_C_SOURCE 2

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "socketio.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include "list.h"

#define SOCKET_SUCCESS      0
#define INVALID_SOCKET      -1

typedef struct PENDING_SOCKET_IO_TAG
{
    unsigned char* bytes;
    size_t size;
    ON_SEND_COMPLETE on_send_complete;
    void* callback_context;
    LIST_HANDLE pending_io_list;
} PENDING_SOCKET_IO;

typedef struct SOCKET_IO_INSTANCE_TAG
{
    int socket;
    ON_BYTES_RECEIVED on_bytes_received;
    ON_IO_STATE_CHANGED on_io_state_changed;
    LOGGER_LOG logger_log;
    void* callback_context;
    char* hostname;
    int port;
    IO_STATE io_state;
    LIST_HANDLE pending_io_list;
} SOCKET_IO_INSTANCE;

static const IO_INTERFACE_DESCRIPTION socket_io_interface_description = 
{
    socketio_create,
    socketio_destroy,
    socketio_open,
    socketio_close,
    socketio_send,
    socketio_dowork
};

static void set_io_state(SOCKET_IO_INSTANCE* socket_io_instance, IO_STATE io_state)
{
    IO_STATE previous_state = socket_io_instance->io_state;
    socket_io_instance->io_state = io_state;
    if (socket_io_instance->on_io_state_changed != NULL)
    {
        socket_io_instance->on_io_state_changed(socket_io_instance->callback_context, io_state, previous_state);
    }
}

static int add_pending_io(SOCKET_IO_INSTANCE* socket_io_instance, const unsigned char* buffer, size_t size, ON_SEND_COMPLETE on_send_complete, void* callback_context)
{
    int result;
    PENDING_SOCKET_IO* pending_socket_io = (PENDING_SOCKET_IO*)malloc(sizeof(PENDING_SOCKET_IO));
    if (pending_socket_io == NULL)
    {
        result = __LINE__;
    }
    else
    {
        pending_socket_io->bytes = (unsigned char*)malloc(size);
        if (pending_socket_io->bytes == NULL)
        {
            free(pending_socket_io);
            result = __LINE__;
        }
        else
        {
            pending_socket_io->size = size;
            pending_socket_io->on_send_complete = on_send_complete;
            pending_socket_io->callback_context = callback_context;
            pending_socket_io->pending_io_list = socket_io_instance->pending_io_list;
            (void)memcpy(pending_socket_io->bytes, buffer, size);

            if (list_add(socket_io_instance->pending_io_list, pending_socket_io) == NULL)
            {
                free(pending_socket_io->bytes);
                free(pending_socket_io);
                result = __LINE__;
            }
            else
            {
                result = 0;
            }
        }
    }

    return result;
}

CONCRETE_IO_HANDLE socketio_create(void* io_create_parameters, LOGGER_LOG logger_log)
{
    SOCKETIO_CONFIG* socket_io_config = io_create_parameters;
    SOCKET_IO_INSTANCE* result;

    if (socket_io_config == NULL)
    {
        result = NULL;
    }
    else
    {
        result = malloc(sizeof(SOCKET_IO_INSTANCE));
        if (result != NULL)
        {
            result->pending_io_list = list_create();
            if (result->pending_io_list == NULL)
            {
                free(result);
                result = NULL;
            }
            else
            {
                result->hostname = (char*)malloc(strlen(socket_io_config->hostname) + 1);
                if (result->hostname == NULL)
                {
                    list_destroy(result->pending_io_list);
                    free(result);
                    result = NULL;
                }
                else
                {
                    strcpy(result->hostname, socket_io_config->hostname);
                    result->port = socket_io_config->port;
                    result->on_bytes_received = NULL;
                    result->on_io_state_changed = NULL;
                    result->logger_log = logger_log;
                    result->socket = INVALID_SOCKET;
                    result->callback_context = NULL;
                    result->io_state = IO_STATE_NOT_OPEN;
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
        SOCKET_IO_INSTANCE* socket_io_instance = (SOCKET_IO_INSTANCE*)socket_io;
        /* we cannot do much if the close fails, so just ignore the result */
        close(socket_io_instance->socket);

        /* clear allpending IOs */
        LIST_ITEM_HANDLE first_pending_io = list_get_head_item(socket_io_instance->pending_io_list);
        while (first_pending_io != NULL)
        {
            PENDING_SOCKET_IO* pending_socket_io = (PENDING_SOCKET_IO*)list_item_get_value(first_pending_io);
            if (pending_socket_io != NULL)
            {
                free(pending_socket_io->bytes);
                free(pending_socket_io);
            }

            (void)list_remove(socket_io_instance->pending_io_list, first_pending_io);
            first_pending_io = list_get_head_item(socket_io_instance->pending_io_list);
        }

        list_destroy(socket_io_instance->pending_io_list);
        free(socket_io_instance->hostname);
        free(socket_io);
    }
}

int socketio_open(CONCRETE_IO_HANDLE socket_io, ON_BYTES_RECEIVED on_bytes_received, ON_IO_STATE_CHANGED on_io_state_changed, void* callback_context)
{
    int result;

    SOCKET_IO_INSTANCE* socket_io_instance = (SOCKET_IO_INSTANCE*)socket_io;
    if (socket_io == NULL)
    {
        result = __LINE__;
    }
    else
    {
        struct addrinfo* addrInfo;
        char portString[16];

        socket_io_instance->socket = socket(AF_INET, SOCK_STREAM, 0);
        if (socket_io_instance->socket < SOCKET_SUCCESS)
        {
            set_io_state(socket_io_instance, IO_STATE_ERROR);
            result = __LINE__;
        }
        else
        {
            sprintf(portString, "%u", socket_io_instance->port);
            if (getaddrinfo(socket_io_instance->hostname, portString, NULL, &addrInfo) != 0)
            {
                close(socket_io_instance->socket);
                set_io_state(socket_io_instance, IO_STATE_ERROR);
                socket_io_instance->socket = INVALID_SOCKET;
                result = __LINE__;
            }
            else
            {
                if (connect(socket_io_instance->socket, addrInfo->ai_addr, sizeof(*addrInfo->ai_addr)) != 0)
                {
                    close(socket_io_instance->socket);
                    set_io_state(socket_io_instance, IO_STATE_ERROR);
                    socket_io_instance->socket = INVALID_SOCKET;
                    result = __LINE__;
                }
                else
                {
                    int flags;

                    if ((-1 == (flags = fcntl(socket_io_instance->socket, F_GETFL, 0))) ||
                        (fcntl(socket_io_instance->socket, F_SETFL, flags | O_NONBLOCK) == -1))
                    {
                        close(socket_io_instance->socket);
                        set_io_state(socket_io_instance, IO_STATE_ERROR);
                        socket_io_instance->socket = INVALID_SOCKET;
                        result = __LINE__;
                    }
                    else
                    {
                        socket_io_instance->on_bytes_received = on_bytes_received;
                        socket_io_instance->on_io_state_changed = on_io_state_changed;
                        socket_io_instance->callback_context = callback_context;

                        set_io_state(socket_io_instance, IO_STATE_OPEN);
                        result = 0;
                    }
                }
                freeaddrinfo(addrInfo);
            }
        }
    }

    return result;
}

int socketio_close(CONCRETE_IO_HANDLE socket_io)
{
    int result = 0;

    if (socket_io == NULL)
    {
        result = __LINE__;
    }
    else
    {
        SOCKET_IO_INSTANCE* socket_io_instance = (SOCKET_IO_INSTANCE*)socket_io;
        (void)shutdown(socket_io_instance->socket, SHUT_RDWR);
        close(socket_io_instance->socket);
        socket_io_instance->socket = INVALID_SOCKET;
        set_io_state(socket_io_instance, IO_STATE_NOT_OPEN);
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
        result = __LINE__;
    }
    else
    {
        SOCKET_IO_INSTANCE* socket_io_instance = (SOCKET_IO_INSTANCE*)socket_io;
        if (socket_io_instance->io_state != IO_STATE_OPEN)
        {
            result = __LINE__;
        }
        else
        {
            LIST_ITEM_HANDLE first_pending_io = list_get_head_item(socket_io_instance->pending_io_list);
            if (first_pending_io != NULL)
            {
                if (add_pending_io(socket_io_instance, buffer, size, on_send_complete, callback_context) != 0)
                {
                    result = __LINE__;
                }
                else
                {
                    result = 0;
                }
            }
            else
            {
                int send_result = send(socket_io_instance->socket, buffer, size, 0);
                if (send_result != size)
                {
                    if (send_result == INVALID_SOCKET)
                    {
                        printf("Error sending on socket\r\n");
                        result = __LINE__;
                    }
                    else
                    {
                        /* queue data */
                        if (add_pending_io(socket_io_instance, buffer + send_result, size - send_result, on_send_complete, callback_context) != 0)
                        {
                            result = __LINE__;
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

                    size_t i;
                    for (i = 0; i < size; i++)
                    {
                        LOG(socket_io_instance->logger_log, 0, "%02x-> ", ((unsigned char*)buffer)[i]);
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
        if (socket_io_instance->io_state == IO_STATE_OPEN)
        {
            int received = 1;

            LIST_ITEM_HANDLE first_pending_io = list_get_head_item(socket_io_instance->pending_io_list);
            while (first_pending_io != NULL)
            {
                PENDING_SOCKET_IO* pending_socket_io = (PENDING_SOCKET_IO*)list_item_get_value(first_pending_io);
                if (pending_socket_io == NULL)
                {
                    set_io_state(socket_io_instance, IO_STATE_ERROR);
                    break;
                }

                int send_result = send(socket_io_instance->socket, pending_socket_io->bytes, pending_socket_io->size, 0);
                if (send_result != pending_socket_io->size)
                {
                    if (send_result == INVALID_SOCKET)
                    {
                        free(pending_socket_io->bytes);
                        free(pending_socket_io);
                        (void)list_remove(socket_io_instance->pending_io_list, first_pending_io);

                        set_io_state(socket_io_instance, IO_STATE_ERROR);
                    }
                    else
                    {
                        /* simply wait */
                        (void)memmove(pending_socket_io->bytes, pending_socket_io->bytes + send_result, pending_socket_io->size - send_result);
                    }
                }
                else
                {
                    if (pending_socket_io->on_send_complete != NULL)
                    {
                        pending_socket_io->on_send_complete(pending_socket_io->callback_context, send_result);
                    }

                    free(pending_socket_io->bytes);
                    free(pending_socket_io);
                    if (list_remove(socket_io_instance->pending_io_list, first_pending_io) != 0)
                    {
                        set_io_state(socket_io_instance, IO_STATE_ERROR);
                    }
                }

                first_pending_io = list_get_head_item(socket_io_instance->pending_io_list);
            }

            while (received > 0)
            {
                unsigned char recv_bytes[1];
                received = recv(socket_io_instance->socket, recv_bytes, sizeof(recv_bytes), 0);
                if (received > 0)
                {
                    int i;
                    for (i = 0; i < received; i++)
                    {
                        LOG(socket_io_instance->logger_log, 0, "<-%02x ", (unsigned char)recv_bytes[i]);
                    }

                    if (socket_io_instance->on_bytes_received != NULL)
                    {
                        /* explictly ignoring here the result of the callback */
                        (void)socket_io_instance->on_bytes_received(socket_io_instance->callback_context, recv_bytes, received);
                    }
                }
            }
        }
    }
}

const IO_INTERFACE_DESCRIPTION* socketio_get_interface_description(void)
{
    return &socket_io_interface_description;
}
