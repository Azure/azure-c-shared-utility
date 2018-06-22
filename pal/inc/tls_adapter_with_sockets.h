// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef TLSIO_ADAPTER_USING_SOCKET_H
#define TLSIO_ADAPTER_USING_SOCKET_H

#include "azure_c_shared_utility/umock_c_prod.h"
#include "azure_c_shared_utility/xio_async_result.h"
#include "azure_c_shared_utility/tlsio_options.h"
#include "socket_async.h"
#include "tls_adapter_common.h"

#ifdef __cplusplus
extern "C" {
#include <cstddef>
#else
#include <stddef.h>
#endif /* __cplusplus */

// All of the parameters of this call have permanent existence, and need not be deep-copied
MOCKABLE_FUNCTION(, TLS_ADAPTER_INSTANCE_HANDLE, tls_adapter_with_sockets_create, TLSIO_OPTIONS*, tlsio_options,
        const char*, hostname, SOCKET_ASYNC_HANDLE, socket_async);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TLSIO_ADAPTER_USING_SOCKET_H */
