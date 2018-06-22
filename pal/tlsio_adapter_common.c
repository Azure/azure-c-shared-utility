// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdlib.h>
#include "azure_c_shared_utility/gballoc.h"
#include "azure_c_shared_utility/tlsio.h"
#include "azure_c_shared_utility/tlsio_options.h"
#include "azure_c_shared_utility/optimize_size.h"
#include "azure_c_shared_utility/crt_abstractions.h"
#include "azure_c_shared_utility/xlogging.h"
#include "azure_c_shared_utility/xio_state.h"
#include "azure_c_shared_utility/strings_types.h"
#include "azure_c_shared_utility/strings.h"
#include "azure_c_shared_utility/xio_async_result.h"
#include "tlsio_adapter_common.h"
#include "tls_adapter_common.h"

#if (!defined WIN32 && !defined __APPLE__ && !defined __linux__)
#include "azure_iot_os_user_agent.h"
#include "azure_iot_arch_user_agent.h"
#endif

#define XIO_RECEIVE_BUFFER_SIZE 64

#define MAX_VALID_PORT 0xffff

static void tlsio_adapter_common_release_resources(TLSIO_ADAPTER_COMMON* tlsio_adapter_common)
{
    tlsio_options_release_resources(&tlsio_adapter_common->options);
    free((char*)tlsio_adapter_common->hostname);
}

// Private function only for internal Azure IoT SDK use
int tlsio_adapter_common_initialize(TLSIO_ADAPTER_COMMON* tlsio_adapter_common,
        void* create_parameters, int socket_async_option_caps)
{
    int result;
    TLSIO_CONFIG* config = (TLSIO_CONFIG*)create_parameters;
    if (tlsio_adapter_common == NULL || config == NULL || config->hostname == NULL ||
                config->port < 0 || config->port > MAX_VALID_PORT)
    {
        LogError("bad parameter");
        result = __FAILURE__;
    }
    else
    {
        int option_caps = socket_async_option_caps | tls_adapter_common_get_option_caps();
        memset(tlsio_adapter_common, 0, sizeof(TLSIO_ADAPTER_COMMON));
        tlsio_options_initialize(&tlsio_adapter_common->options, option_caps);
        // Collect the create info
        tlsio_adapter_common->port = (uint16_t)config->port;
        if (0 != mallocAndStrcpy_s((char**)&tlsio_adapter_common->hostname, config->hostname))
        {
            LogError("malloc failed");
            tlsio_adapter_common_release_resources(tlsio_adapter_common);
            result = __FAILURE__;
        }
        else
        {
            result = 0;
        }
    }
    return result;
}

void tlsio_adapter_common_destroy(XIO_ADAPTER_INSTANCE_HANDLE xio_adapter_in)
{
    TLSIO_ADAPTER_COMMON* tlsio_adapter_common = (TLSIO_ADAPTER_COMMON*)xio_adapter_in;
    if (tlsio_adapter_common != NULL)
    {
        tlsio_adapter_common_release_resources(tlsio_adapter_common);
        free(tlsio_adapter_common);
    }
}

// Private function only for internal Azure IoT SDK use
// This function isn't strictly necessary since the tlsio_adapter_common struct
// is visible, but it makes the design more clear.
void tlsio_adapter_common_set_open_values(TLSIO_ADAPTER_COMMON* tlsio_adapter_common,
        TLS_ADAPTER_INSTANCE_HANDLE tls, ON_BYTES_RECEIVED on_received, void* on_received_context)
{
    tlsio_adapter_common->tls_adapter = tls;
    tlsio_adapter_common->on_received = on_received;
    tlsio_adapter_common->on_received_context = on_received_context;
}

// Private function only for internal Azure IoT SDK use
XIO_ASYNC_RESULT tlsio_adapter_common_close(XIO_ADAPTER_INSTANCE_HANDLE xio_adapter_in)
{
    TLSIO_ADAPTER_COMMON* tlsio_adapter_common = (TLSIO_ADAPTER_COMMON*)xio_adapter_in;
    if (tlsio_adapter_common != NULL && tlsio_adapter_common->tls_adapter != NULL)
    {
        tls_adapter_common_close_and_destroy(tlsio_adapter_common->tls_adapter);
        tlsio_adapter_common->tls_adapter = NULL;
    }
    // Tlsio adapters don't do async closing, and the SDK doesn't support it anyway
    return XIO_ASYNC_RESULT_SUCCESS;
}

// The parameters to this call have already been guaranteed by xio_state.
XIO_ASYNC_RESULT tlsio_adapter_common_read(XIO_ADAPTER_INSTANCE_HANDLE xio_adapter_in)
{
    XIO_ASYNC_RESULT result;
    int rcv_bytes;
    uint8_t buffer[XIO_RECEIVE_BUFFER_SIZE];
    TLSIO_ADAPTER_COMMON* tlsio_adapter_common = (TLSIO_ADAPTER_COMMON*)xio_adapter_in;

    /* Codes_SRS_XIO_ADAPTER_30_041: [ The  xio_adapter_read  shall attempt to read from its underlying data source. ]*/
    if ((rcv_bytes = tls_adapter_common_read(tlsio_adapter_common->tls_adapter, buffer, XIO_RECEIVE_BUFFER_SIZE)) > 0)
    {
        /* Codes_SRS_XIO_ADAPTER_30_046: [ If data is available, xio_adapter_read shall report the results using the
          stored on_received and on_received_context and return XIO_ASYNC_RESULT_SUCCESS. ]*/
        tlsio_adapter_common->on_received(tlsio_adapter_common->on_received_context, buffer, rcv_bytes);
        result = XIO_ASYNC_RESULT_SUCCESS;
    }
    else if (rcv_bytes == 0)
    {
        /* Codes_SRS_XIO_ADAPTER_30_044: [ If no data is available  xio_adapter_read  shall return  XIO_ASYNC_RESULT_WAITING . ]*/
        result = XIO_ASYNC_RESULT_WAITING;
    }
    else
    {
        /* Codes_SRS_XIO_ADAPTER_30_043: [ On failure, xio_adapter_read shall log an error and return XIO_ASYNC_RESULT_FAILURE. ]*/
        LogError("this_tls_adapter_read failure");
        result = XIO_ASYNC_RESULT_FAILURE;
    }
    return result;
}

// The parameters to this call have already been guaranteed by xio_state.
/* Codes_SRS_XIO_ADAPTER_30_051: [ The xio_adapter_write shall attempt to write buffer_size characters from buffer to its underlying data sink. ]*/
int tlsio_adapter_common_write(XIO_ADAPTER_INSTANCE_HANDLE xio_adapter_in, const uint8_t* buffer, uint32_t count)
{
    int result;
    TLSIO_ADAPTER_COMMON* tlsio_adapter_common = (TLSIO_ADAPTER_COMMON*)xio_adapter_in;

    // The count is guaranteed by the calling framework to be less than INT_MAX
    // in order to ensure that this cast is safe
    result = tls_adapter_common_write(tlsio_adapter_common->tls_adapter, buffer, count);

    return result;
}


int tlsio_adapter_common_setoption(CONCRETE_IO_HANDLE xio, const char* optionName, const void* value)
{
    TLSIO_ADAPTER_COMMON* context = (TLSIO_ADAPTER_COMMON*)xio;
    return tlsio_options_set(&context->options, optionName, value);
}

OPTIONHANDLER_HANDLE tlsio_adapter_common_retrieveoptions(CONCRETE_IO_HANDLE xio)
{
    TLSIO_ADAPTER_COMMON* context = (TLSIO_ADAPTER_COMMON*)xio;
    // Note that we're passing xio_state_setoption here, not our local option setter
    return tlsio_options_retrieve_options(&context->options, xio_state_setoption);
}


// Platform functions //////////////////

int platform_init(void)
{
    return tls_adapter_common_init();
}

#if (defined WIN32 || defined __APPLE__ || defined __linux__)
// When converting big-machine tlsios to this design, remove the platform_init(),
// platform_get_default_tlsio(), and platform_deinit() functions from
// their platform.c file, move any platform_init() and platform_deinit()
// functionality into tls_adapter_init() and tls_adapter_deinit(), and just leave
// platform_get_platform_info() intact.
#else
// A simple platform_get_platform_info suitable for microcontrollers
STRING_HANDLE platform_get_platform_info(void)
{
    // AZURE_IOT_OS_USER_AGENT and AZURE_IOT_ARCH_USER_AGENT must resolve to quoted strings
    STRING_HANDLE result = STRING_construct_sprintf(
            "(direct; " AZURE_IOT_OS_USER_AGENT "; " AZURE_IOT_ARCH_USER_AGENT ")");
    return result;
}
#endif

void platform_deinit(void)
{
    tls_adapter_common_deinit();
}
