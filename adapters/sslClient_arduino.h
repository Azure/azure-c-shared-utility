// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef SSLCLIENT_ARDUINO_H
#define SSLCLIENT_ARDUINO_H

#ifdef __cplusplus
extern "C" {
#include <cstddef>
#include <cstdint>
#else
#include <stddef.h>
#include <stdint.h>
#endif /* __cplusplus */

#include "azure_c_shared_utility/umock_c_prod.h"

#define SSLCLIENT_HANDLE void*

MOCKABLE_FUNCTION(, SSLCLIENT_HANDLE, sslClient_new);
MOCKABLE_FUNCTION(, void, sslClient_delete, SSLCLIENT_HANDLE, handle);
MOCKABLE_FUNCTION(, void, sslClient_setTimeout, SSLCLIENT_HANDLE, handle, unsigned long, timeout);
MOCKABLE_FUNCTION(, uint8_t, sslClient_connected, SSLCLIENT_HANDLE, handle);
MOCKABLE_FUNCTION(, int, sslClient_connect, SSLCLIENT_HANDLE, handle, uint32_t , ipAddress, uint16_t, port);
MOCKABLE_FUNCTION(, void, sslClient_stop, SSLCLIENT_HANDLE, handle);
MOCKABLE_FUNCTION(, size_t, sslClient_write, SSLCLIENT_HANDLE, handle, const uint8_t*, buf, size_t, size);
MOCKABLE_FUNCTION(, int, sslClient_read, SSLCLIENT_HANDLE, handle, uint8_t*, buf, size_t, size);

MOCKABLE_FUNCTION(, uint8_t, sslClient_hostByName, const char*, hostName, uint32_t*, ipAddress);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* SSLCLIENT_ARDUINO_H */

