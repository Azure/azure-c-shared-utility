// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef TLSIO_H
#define TLSIO_H

#ifdef __cplusplus
extern "C" {
#include <cstddef>
#else
#include <stddef.h>
#endif /* __cplusplus */

#include "io.h"
#include "iot_logging.h"

typedef struct TLSIO_CONFIG_TAG
{
	const char* hostname;
	int port;
} TLSIO_CONFIG;

extern IO_HANDLE tlsio_create(void* io_create_parameters, LOGGER_LOG logger_log);
extern void tlsio_destroy(IO_HANDLE tls_io);
extern int tlsio_open(IO_HANDLE tls_io, ON_BYTES_RECEIVED on_bytes_received, ON_IO_STATE_CHANGED on_io_state_changed, void* callback_context);
extern int tlsio_close(IO_HANDLE tls_io);
extern int tlsio_send(IO_HANDLE tls_io, const void* buffer, size_t size);
extern void tlsio_dowork(IO_HANDLE tls_io);
extern const IO_INTERFACE_DESCRIPTION* tlsio_get_interface_description(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TLSIO_H */
