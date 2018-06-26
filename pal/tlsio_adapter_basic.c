// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#undef NO_LOGGING

#include <stdlib.h>

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include "azure_c_shared_utility/xio.h"
#include "azure_c_shared_utility/gballoc.h"
#include "azure_c_shared_utility/xlogging.h"
#include "azure_c_shared_utility/agenttime.h"
#include "azure_c_shared_utility/crt_abstractions.h"
#include "azure_c_shared_utility/tlsio.h"
#include "azure_c_shared_utility/xio_adapter.h"
#include "azure_c_shared_utility/xio_state.h"
#include "tlsio_adapter_common.h"
#include "tls_adapter_basic.h"

// Opening steps are really only needed for operations that might take a while
typedef enum
{
    OPENING_STEP_VALIDATE = 0,
    OPENING_STEP_INIT_TLS,
    OPENING_STEP_OPEN_TLS,
    OPENING_STEP_VALIDATION_FAILED
} OPENING_STEPS;

typedef struct TLSIO_ADAPTER_INSTANCE_TAG
{
    // The tlsio_adapter_common member must be the first member of
    // this struct to use tlsio_adapter_common functionality.
    TLSIO_ADAPTER_COMMON    tlsio_adapter_common;
    // Opening steps
    OPENING_STEPS           opening_step;
} TLSIO_ADAPTER_INSTANCE;


static XIO_ADAPTER_INSTANCE_HANDLE tlsio_adapter_basic_create(void* io_create_parameters)
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
    }
    return (XIO_ADAPTER_INSTANCE_HANDLE)result;
}

// The parameters to this call have already been guaranteed by xio_state.
static XIO_ASYNC_RESULT tlsio_adapter_basic_open(XIO_ADAPTER_INSTANCE_HANDLE xio_adapter_in,
    ON_BYTES_RECEIVED on_received, void* on_received_context)
{
    XIO_ASYNC_RESULT result;
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
            context->opening_step = OPENING_STEP_INIT_TLS;
        }
    }

    switch (context->opening_step)
    {
    case OPENING_STEP_INIT_TLS:
        created_tls = tls_adapter_basic_create(&context->tlsio_adapter_common.options,
                context->tlsio_adapter_common.hostname, context->tlsio_adapter_common.port);
        result = created_tls != NULL ? XIO_ASYNC_RESULT_SUCCESS : XIO_ASYNC_RESULT_FAILURE;
        if (result == XIO_ASYNC_RESULT_SUCCESS)
        {
            tlsio_adapter_common_set_open_values(&context->tlsio_adapter_common,
                    created_tls, on_received, on_received_context);
            context->opening_step = OPENING_STEP_OPEN_TLS;
            result = XIO_ASYNC_RESULT_WAITING;
        }
        break;
    case OPENING_STEP_OPEN_TLS:
        result = tls_adapter_basic_open(context->tlsio_adapter_common.tls_adapter);
        break;
    case OPENING_STEP_VALIDATION_FAILED:
        // The error is already logged
        result = XIO_ASYNC_RESULT_FAILURE;
        break;
    default:
        LogError("Calling open after open failure");
        result = XIO_ASYNC_RESULT_FAILURE;
        break;
    }

    if (result == XIO_ASYNC_RESULT_FAILURE)
    {
        (void)tlsio_adapter_basic_close(xio_adapter_in);
    }

    return result;
}


static const XIO_ADAPTER_INTERFACE tls_adapter =
{
    tlsio_adapter_basic_create,
    tlsio_adapter_common_destroy,
    tlsio_adapter_basic_open,
    tlsio_adapter_common_close,
    tlsio_adapter_common_read,
    tlsio_adapter_common_write,
    tlsio_adapter_common_setoption,
    tlsio_adapter_common_retrieveoptions
};


// This macro creates the vtable and declares the platform_get_default_tlsio function.
DECLARE_XIO_GET_INTERFACE_FUNCTION(tls_adapter, platform_get_default_tlsio)

