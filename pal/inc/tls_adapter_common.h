// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef TLS_ADAPTER_COMMON_H
#define TLS_ADAPTER_COMMON_H

#include "azure_c_shared_utility/umock_c_prod.h"
#include "azure_c_shared_utility/xio_async_result.h"
#include "azure_c_shared_utility/tlsio_options.h"

#ifdef __cplusplus
extern "C" {
#include <cstddef>
#include <cstdint>
#else
#include <stddef.h>
#include <stdint.h>
#endif /* __cplusplus */

// This file contains declarations that are common to both
// tlsio_adapter_with_sockets.h and tlsio_adapter_no_socket.h

typedef struct TLS_ADAPTER_INSTANCE_TAG* TLS_ADAPTER_INSTANCE_HANDLE;

// Called only once to initialize the underlying TLS API if needed.
// Returns 0 for success, and XIO_ASYNC_RESULT_FAILURE for error
MOCKABLE_FUNCTION(, int, tls_adapter_common_init);

// Called only once to de-initialize the underlying TLS API if needed.
MOCKABLE_FUNCTION(, void, tls_adapter_common_deinit);

// Called repeatedly until it returns either XIO_ASYNC_RESULT_SUCCESS or XIO_ASYNC_RESULT_FAILURE
MOCKABLE_FUNCTION(, XIO_ASYNC_RESULT, tls_adapter_common_open, TLS_ADAPTER_INSTANCE_HANDLE, adapter);

// Tls adapters are created with either tls_adapter_basic_create or tls_adapter_with_sockets_create
MOCKABLE_FUNCTION(, void, tls_adapter_common_close_and_destroy, TLS_ADAPTER_INSTANCE_HANDLE, adapter);

// Return a bit-or of TLSIO_OPTION_BIT values to specify which options the tls
// adapter supports. For microcontrollers this will usually just be
// TLSIO_OPTION_BIT_TRUSTED_CERTS
MOCKABLE_FUNCTION(, int, tls_adapter_common_get_option_caps);

// buffer is guaranteed non-NULL and size_t is guaranteed smaller than INTMAX
// Return positive for success, 0 if waiting for data, and XIO_ASYNC_RESULT_FAILURE for error
MOCKABLE_FUNCTION(, int, tls_adapter_common_read, TLS_ADAPTER_INSTANCE_HANDLE, adapter,
        uint8_t*, buffer, size_t, size);

// buffer is guaranteed non-NULL and size_t is guaranteed smaller than INTMAX
// Return positive number of bytes written for success, 0 if waiting, and XIO_ASYNC_RESULT_FAILURE for error
MOCKABLE_FUNCTION(, int, tls_adapter_common_write, TLS_ADAPTER_INSTANCE_HANDLE, adapter,
        const uint8_t*, buffer, size_t, size);



#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TLS_ADAPTER_COMMON_H */
