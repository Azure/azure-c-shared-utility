// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#define SECURITY_WIN32

#include <stdlib.h>
#ifdef _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#include <stddef.h>
#include <stdio.h>
#include <stdbool.h>
#include "tlsio.h"
#include "socketio.h"
//#include "sspi.h"
#include "iot_logging.h"

typedef enum TLS_STATE_TAG
{
    TLS_STATE_HANDSHAKE_NOT_STARTED,
    TLS_STATE_HANDSHAKE_CLIENT_HELLO_SENT,
    TLS_STATE_HANDSHAKE_SERVER_HELLO_RECEIVED,
    TLS_STATE_HANDSHAKE_DONE,
    TLS_STATE_ERROR
} TLS_STATE;

typedef struct TLS_IO_INSTANCE_TAG
{
    IO_HANDLE socket_io;
    ON_BYTES_RECEIVED on_bytes_received;
    ON_IO_STATE_CHANGED on_io_state_changed;
    void* callback_context;
    LOGGER_LOG logger_log;
    int security_context;
    TLS_STATE tls_state;
    char* host_name;
    int credential_handle;
    bool credential_handle_allocated;
    unsigned char* received_bytes;
    size_t received_byte_count;
    size_t buffer_size;
    size_t needed_bytes;
    size_t consumed_bytes;
    IO_STATE io_state;
} TLS_IO_INSTANCE;

static const IO_INTERFACE_DESCRIPTION tls_io_interface_description =
{
    tlsio_create,
    tlsio_destroy,
    tlsio_open,
    tlsio_close,
    tlsio_send,
    tlsio_dowork
};

static void set_io_state(TLS_IO_INSTANCE* tls_io_instance, IO_STATE io_state)
{
}

static int resize_receive_buffer(TLS_IO_INSTANCE* tls_io_instance, size_t needed_buffer_size)
{
    int result;
    result = 0;
    return result;
}

static int set_receive_buffer(TLS_IO_INSTANCE* tls_io_instance, size_t buffer_size)
{
    int result;
    result = 0;
    return result;
}

static void tlsio_on_bytes_received(void* context, const void* buffer, size_t size)
{
    TLS_IO_INSTANCE* tls_io_instance = (TLS_IO_INSTANCE*)context;
}

static void tlsio_on_io_state_changed(void* context, IO_STATE new_io_state, IO_STATE previous_io_state)
{
    TLS_IO_INSTANCE* tls_io_instance = (TLS_IO_INSTANCE*)context;
}

IO_HANDLE tlsio_create(void* io_create_parameters, LOGGER_LOG logger_log)
{
    TLSIO_CONFIG* tls_io_config = io_create_parameters;
    TLS_IO_INSTANCE* result;
    result = NULL;
    return result;
}

void tlsio_destroy(IO_HANDLE tls_io)
{
    if (tls_io != NULL)
    {
        TLS_IO_INSTANCE* tls_io_instance = (TLS_IO_INSTANCE*)tls_io;
    }
}

int tlsio_open(IO_HANDLE tls_io, ON_BYTES_RECEIVED on_bytes_received, ON_IO_STATE_CHANGED on_io_state_changed, void* callback_context)
{
    int result;
    result = __LINE__;
    return result;
}

int tlsio_close(IO_HANDLE tls_io)
{
    int result = 0;
    result = __LINE__;
    return result;
}

int send_chunk(IO_HANDLE tls_io, const void* buffer, size_t size)
{
    int result;

    if ( (tls_io == NULL) ||
         (buffer == NULL) ||
         (size == 0))
    {
        /* Invalid arguments */
        result = __LINE__;
    }
    else
    {
        result = 0;
    }

    return result;
}

int tlsio_send(IO_HANDLE tls_io, const void* buffer, size_t size)
{
    int result;
    result = 0;
    return result;
}

void tlsio_dowork(IO_HANDLE tls_io)
{
    if (tls_io != NULL)
    {
        TLS_IO_INSTANCE* tls_io_instance = (TLS_IO_INSTANCE*)tls_io;
        io_dowork(tls_io_instance->socket_io);
    }
}

const IO_INTERFACE_DESCRIPTION* tlsio_get_interface_description(void)
{
    return &tls_io_interface_description;
}
