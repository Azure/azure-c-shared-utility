// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef TLSIO_CYCLONESSL_SOCKET_H
#define TLSIO_CYCLONESSL_SOCKET_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "tls.h"
#include "azure_c_shared_utility/umock_c_prod.h"

MOCKABLE_FUNCTION(, int, tlsio_cyclonessl_socket_create, const char*, hostname, unsigned int, port, TlsSocket*, new_socket);
MOCKABLE_FUNCTION(, void, tlsio_cyclonessl_socket_destroy, TlsSocket, socket);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TLSIO_CYCLONESSL_SOCKET_H */
