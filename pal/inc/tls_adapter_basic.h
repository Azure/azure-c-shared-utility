// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef TLSIO_ADAPTER_BASIC_H
#define TLSIO_ADAPTER_BASIC_H

#include "azure_c_shared_utility/umock_c_prod.h"
#include "azure_c_shared_utility/xio_async_result.h"
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

// All of the parameters of this call have permanent existence, and need not be deep-copied
MOCKABLE_FUNCTION(, TLS_ADAPTER_INSTANCE_HANDLE, tls_adapter_basic_create, TLSIO_OPTIONS*, tlsio_options,
        const char*, hostname, uint16_t, port);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TLSIO_ADAPTER_BASIC_H */
