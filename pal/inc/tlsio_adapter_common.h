// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef TLSIO_ADAPTER_COMMON_H
#define TLSIO_ADAPTER_COMMON_H

#include "azure_c_shared_utility/umock_c_prod.h"
#include "azure_c_shared_utility/tlsio_options.h"
#include "tls_adapter_common.h"

#ifdef __cplusplus
extern "C" {
#include <cstddef>
#include <cstdint>
#else
#include <stddef.h>
#include <stdint.h>
#endif /* __cplusplus */

// The tlsio_adapter_common.c file contains functionality that is common to both
// tlsio_adapter_with_sockets.c and tlsio_adapter_basic.c

typedef struct TLSIO_ADAPTER_COMMON_TAG
{
    // config and options
    const char*                 hostname;
    uint16_t                    port;
    TLSIO_OPTIONS               options;
    // on_received info
    ON_BYTES_RECEIVED           on_received;
    void*                       on_received_context;
    // tls adapter
    TLS_ADAPTER_INSTANCE_HANDLE tls_adapter;
} TLSIO_ADAPTER_COMMON;

// Private function only for internal Azure IoT SDK use
MOCKABLE_FUNCTION(, int, tlsio_adapter_common_initialize, TLSIO_ADAPTER_COMMON*, tlsio_adapter_common,
        void*, create_parameters, int, socket_async_option_caps);

// Private function only for internal Azure IoT SDK use
MOCKABLE_FUNCTION(, void, tlsio_adapter_common_destroy, XIO_ADAPTER_INSTANCE_HANDLE, xio_adapter_in);

// Private function only for internal Azure IoT SDK use
MOCKABLE_FUNCTION(, void, tlsio_adapter_common_set_open_values, TLSIO_ADAPTER_COMMON*, tlsio_adapter_common,
        TLS_ADAPTER_INSTANCE_HANDLE, tls, ON_BYTES_RECEIVED, on_received, void*, on_received_context);

// Private function only for internal Azure IoT SDK use
MOCKABLE_FUNCTION(, XIO_ASYNC_RESULT, tlsio_adapter_common_close, XIO_ADAPTER_INSTANCE_HANDLE, xio_adapter_in);

// Private function only for internal Azure IoT SDK use
MOCKABLE_FUNCTION(, XIO_ASYNC_RESULT, tlsio_adapter_common_read, XIO_ADAPTER_INSTANCE_HANDLE, xio_adapter_in);

// Private function only for internal Azure IoT SDK use
MOCKABLE_FUNCTION(, int, tlsio_adapter_common_write, XIO_ADAPTER_INSTANCE_HANDLE, xio_adapter_in,
        const uint8_t*, buffer, uint32_t, count);

// Private function only for internal Azure IoT SDK use
MOCKABLE_FUNCTION(, int, tlsio_adapter_common_setoption, CONCRETE_IO_HANDLE, xio,
        const char*, optionName, const void*, value);

// Private function only for internal Azure IoT SDK use
MOCKABLE_FUNCTION(, OPTIONHANDLER_HANDLE, tlsio_adapter_common_retrieveoptions, CONCRETE_IO_HANDLE, xio);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TLSIO_ADAPTER_COMMON_H */
