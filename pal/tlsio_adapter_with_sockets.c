// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#undef NO_LOGGING

#include <stdlib.h>

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "azure_c_shared_utility/xio.h"
#include "azure_c_shared_utility/gballoc.h"
#include "azure_c_shared_utility/xlogging.h"
#include "azure_c_shared_utility/agenttime.h"
#include "azure_c_shared_utility/crt_abstractions.h"
#include "azure_c_shared_utility/tlsio.h"
#include "azure_c_shared_utility/xio_adapter.h"
#include "azure_c_shared_utility/xio_state.h"
#include "tlsio_adapter_common.h"
#include "tls_adapter_with_sockets.h"
#include "socket_async.h"

// Opening steps are really only needed for operations that might take a while
typedef enum
{
    OPENING_STEP_VALIDATE = 0,
    OPENING_STEP_DNS,
    OPENING_STEP_SOCKET,
    OPENING_STEP_INIT_TLS,
    OPENING_STEP_HANDSHAKE,
    OPENING_STEP_VALIDATION_FAILED
} OPENING_STEPS;

typedef struct TLSIO_ADAPTER_INSTANCE_TAG
{
    // The tlsio_adapter_common member must be the first member of
    // this struct to use tlsio_adapter_common functionality.
    TLSIO_ADAPTER_COMMON    tlsio_adapter_common;
    // Opening steps
    OPENING_STEPS           opening_step;
    // socket stuff
    uint32_t                ipV4;
    SOCKET_ASYNC_HANDLE     socket_async;
} TLSIO_ADAPTER_INSTANCE;


/* Codes_SRS_XIO_ADAPTER_30_000: [ The xio_adapter_create shall allocate and initialize all necessary resources and return an instance of the xio_adapter. ]*/
static XIO_ADAPTER_INSTANCE_HANDLE tlsio_adapter_with_sockets_create(void* io_create_parameters)
{
    TLSIO_ADAPTER_INSTANCE* result;
    if ((result = malloc(sizeof(TLSIO_ADAPTER_INSTANCE))) == NULL)
    {
        LogError("malloc failed");
        result = NULL;
    }
    else
    {
        // Initialize
        memset(result, 0, sizeof(TLSIO_ADAPTER_INSTANCE));
        tlsio_adapter_common_initialize(&result->tlsio_adapter_common,
                io_create_parameters, TLSIO_OPTION_BIT_NONE);
        result->socket_async = SOCKET_ASYNC_INVALID_SOCKET;
    }
    return (XIO_ADAPTER_INSTANCE_HANDLE)result;
}

static XIO_ASYNC_RESULT tlsio_adapter_with_sockets_close(XIO_ADAPTER_INSTANCE_HANDLE xio_adapter_in)
{
    TLSIO_ADAPTER_INSTANCE* context = (TLSIO_ADAPTER_INSTANCE*)xio_adapter_in;

    // Call the base class function first
    (void)tlsio_adapter_common_close(xio_adapter_in);

    socket_async_destroy(context->socket_async);
    context->socket_async = SOCKET_ASYNC_INVALID_SOCKET;
    return XIO_ASYNC_RESULT_SUCCESS;
}

static XIO_ASYNC_RESULT tlsio_adapter_with_sockets_open(XIO_ADAPTER_INSTANCE_HANDLE xio_adapter_in,
    ON_BYTES_RECEIVED on_received, void* on_received_context)
{
    XIO_ASYNC_RESULT result;
    int ret;
    bool socket_is_open;
    TLS_ADAPTER_INSTANCE_HANDLE created_tls;
    TLSIO_ADAPTER_INSTANCE* context = (TLSIO_ADAPTER_INSTANCE*)xio_adapter_in;

    if (context->opening_step == OPENING_STEP_VALIDATE)
    {
        if (tlsio_options_check_set_value_consistency(&context->tlsio_adapter_common.options) != 0)
        {
            // The tlsio_options_check_set_value_consistency logged the error
            context->opening_step = OPENING_STEP_VALIDATION_FAILED;
        }
        else
        {
            context->opening_step = OPENING_STEP_DNS;
        }
    }

    switch (context->opening_step)
    {
    case OPENING_STEP_DNS:
        // Keep trying to get an ipV4 until we succeed. The socket_async_get_ipv4
        // logs failure, and repeating it here would just pollute the output.
        context->ipV4 = socket_async_get_ipv4(context->tlsio_adapter_common.hostname);
        if (context->ipV4 != 0)
        {
            context->opening_step = OPENING_STEP_SOCKET;
        }
        result = XIO_ASYNC_RESULT_WAITING;
        break;
    case OPENING_STEP_SOCKET:
        // Create the socket_async if necessary
        if (context->socket_async == SOCKET_ASYNC_INVALID_SOCKET)
        {
            context->socket_async = socket_async_create(context->ipV4,
                    context->tlsio_adapter_common.port, false, NULL);
        }
        if (context->socket_async == SOCKET_ASYNC_INVALID_SOCKET)
        {
            LogError("Could not create socket_async");
            result = XIO_ASYNC_RESULT_FAILURE;
        }
        else
        {
            ret = socket_async_is_create_complete(context->socket_async, &socket_is_open);
            if (ret != 0)
            {
                LogError("Failed socket_async_is_create_complete");
                result = XIO_ASYNC_RESULT_FAILURE;
            }
            else
            {
                if (socket_is_open)
                {
                    context->opening_step = OPENING_STEP_INIT_TLS;
                }
                result = XIO_ASYNC_RESULT_WAITING;
            }
        }

        break;
    case OPENING_STEP_INIT_TLS:
        created_tls = tls_adapter_with_sockets_create(&context->tlsio_adapter_common.options,
                context->tlsio_adapter_common.hostname, context->socket_async);
        result = created_tls != NULL ? XIO_ASYNC_RESULT_SUCCESS : XIO_ASYNC_RESULT_FAILURE;
        if (result == XIO_ASYNC_RESULT_SUCCESS)
        {
            // Record our newly created tls_adapter and set our on_received callback stuff
            tlsio_adapter_common_set_open_values(&context->tlsio_adapter_common,
                    created_tls, on_received, on_received_context);
            context->opening_step = OPENING_STEP_HANDSHAKE;
            result = XIO_ASYNC_RESULT_WAITING;
        }
        break;
    case OPENING_STEP_HANDSHAKE:
        result = tls_adapter_common_open(context->tlsio_adapter_common.tls_adapter);
        break;
    case OPENING_STEP_VALIDATION_FAILED:
        // The failure is already logged
        result = XIO_ASYNC_RESULT_FAILURE;
        break;
    default:
        LogError("Calling open after open failure");
        result = XIO_ASYNC_RESULT_FAILURE;
        break;
    }

    if (result == XIO_ASYNC_RESULT_FAILURE)
    {
        (void)tlsio_adapter_with_sockets_close(xio_adapter_in);
    }

    return result;
}


static const XIO_ADAPTER_INTERFACE tls_adapter =
{
    tlsio_adapter_with_sockets_create,
    tlsio_adapter_common_destroy,
    tlsio_adapter_with_sockets_open,
    tlsio_adapter_with_sockets_close,
    tlsio_adapter_common_read,
    tlsio_adapter_common_write,
    tlsio_adapter_common_setoption,
    tlsio_adapter_common_retrieveoptions
};


// This macro creates the vtable and declares the platform_get_default_tlsio function.
DECLARE_XIO_GET_INTERFACE_FUNCTION(tls_adapter, platform_get_default_tlsio)

