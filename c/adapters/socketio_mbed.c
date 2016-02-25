// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdlib.h>
#ifdef _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include "socketio.h"
#include "list.h"
#include "tcpsocketconnection_c.h"

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
    LIST_HANDLE pending_io_list;
} PENDING_SOCKET_IO;

typedef struct SOCKET_IO_INSTANCE_TAG
{
    TCPSOCKETCONNECTION_HANDLE tcp_socket_connection;
    ON_BYTES_RECEIVED on_bytes_received;
    ON_IO_ERROR on_io_error;
    LOGGER_LOG logger_log;
    void* open_callback_context;
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

static void indicate_error(SOCKET_IO_INSTANCE* socket_io_instance)
{
    if (socket_io_instance->on_io_error != NULL)
    {
        socket_io_instance->on_io_error(socket_io_instance->open_callback_context);
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
                    result->on_io_error = NULL;
                    result->logger_log = logger_log;
                    result->open_callback_context = NULL;
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
        SOCKET_IO_INSTANCE* socket_io_instance = (SOCKET_IO_INSTANCE*)socket_io;

        tcpsocketconnection_destroy(socket_io_instance->tcp_socket_connection);

        /* clear all pending IOs */
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

int socketio_open(CONCRETE_IO_HANDLE socket_io, ON_IO_OPEN_COMPLETE on_io_open_complete, ON_BYTES_RECEIVED on_bytes_received, ON_IO_ERROR on_io_error, void* callback_context)
{
    int result;

    SOCKET_IO_INSTANCE* socket_io_instance = (SOCKET_IO_INSTANCE*)socket_io;
    if (socket_io == NULL)
    {
        result = __LINE__;
    }
    else
    {
        socket_io_instance->tcp_socket_connection = tcpsocketconnection_create();
        if (socket_io_instance->tcp_socket_connection == NULL)
        {
            result = __LINE__;
        }
        else
        {
            if (tcpsocketconnection_connect(socket_io_instance->tcp_socket_connection, socket_io_instance->hostname, socket_io_instance->port) != 0)
            {
                tcpsocketconnection_destroy(socket_io_instance->tcp_socket_connection);
                socket_io_instance->tcp_socket_connection = NULL;
                result = __LINE__;
            }
            else
            {
                tcpsocketconnection_set_blocking(socket_io_instance->tcp_socket_connection, false, 0);
                socket_io_instance->on_bytes_received = on_bytes_received;
                socket_io_instance->on_io_error = on_io_error;
                socket_io_instance->open_callback_context = callback_context;
                socket_io_instance->io_state = IO_STATE_OPEN;

                if (on_io_open_complete != NULL)
                {
                    on_io_open_complete(callback_context, IO_OPEN_OK);
                }

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
        result = __LINE__;
    }
    else
    {
        SOCKET_IO_INSTANCE* socket_io_instance = (SOCKET_IO_INSTANCE*)socket_io;

        if ((socket_io_instance->io_state == IO_STATE_CLOSED) ||
            (socket_io_instance->io_state == IO_STATE_CLOSING))
        {
            result = __LINE__;
        }
        else
        {
            tcpsocketconnection_close(socket_io_instance->tcp_socket_connection);
            socket_io_instance->tcp_socket_connection = NULL;
            socket_io_instance->io_state = IO_STATE_CLOSED;

            result = 0;
        }
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
                int send_result = tcpsocketconnection_send(socket_io_instance->tcp_socket_connection, buffer, size);
                if (send_result != size)
                {
                    if (send_result < 0)
                    {
                        send_result = 0;
                    }

                    /* queue data */
                    if (add_pending_io(socket_io_instance, (unsigned char*)buffer + send_result, size - send_result, on_send_complete, callback_context) != 0)
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
                    socket_io_instance->io_state = IO_STATE_ERROR;
                    indicate_error(socket_io_instance);
                    break;
                }

                int send_result = tcpsocketconnection_send(socket_io_instance->tcp_socket_connection, (const char*)pending_socket_io->bytes, pending_socket_io->size);
                if (send_result != pending_socket_io->size)
                {
                    if (send_result < 0)
                    {
                        send_result = 0;
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
                        pending_socket_io->on_send_complete(pending_socket_io->callback_context, IO_SEND_ERROR);
                    }

                    free(pending_socket_io->bytes);
                    free(pending_socket_io);
                    if (list_remove(socket_io_instance->pending_io_list, first_pending_io) != 0)
                    {
                        socket_io_instance->io_state = IO_STATE_ERROR;
                        indicate_error(socket_io_instance);
                    }
                }

                first_pending_io = list_get_head_item(socket_io_instance->pending_io_list);
            }

            while (received > 0)
            {
                unsigned char recv_bytes[1];
                received = tcpsocketconnection_receive(socket_io_instance->tcp_socket_connection, (char*)recv_bytes, sizeof(recv_bytes));
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
                        (void)socket_io_instance->on_bytes_received(socket_io_instance->open_callback_context, recv_bytes, received);
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

