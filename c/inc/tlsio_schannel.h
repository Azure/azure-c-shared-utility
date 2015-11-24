// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef TLSIO_SCHANNEL_H
#define TLSIO_SCHANNEL_H

#ifdef __cplusplus
extern "C" {
#include <cstddef>
#else
#include <stddef.h>
#endif /* __cplusplus */

#include "xio.h"
#include "iot_logging.h"

typedef struct TLSIO_SCHANNEL_CONFIG_TAG
{
	const char* hostname;
	int port;
} TLSIO_SCHANNEL_CONFIG;

extern CONCRETE_XIO_HANDLE tlsio_schannel_create(void* io_create_parameters, LOGGER_LOG logger_log);
extern void tlsio_schannel_destroy(CONCRETE_XIO_HANDLE tls_io);
extern int tlsio_schannel_open(CONCRETE_XIO_HANDLE tls_io, ON_BYTES_RECEIVED on_bytes_received, ON_IO_STATE_CHANGED on_io_state_changed, void* callback_context);
extern int tlsio_schannel_close(CONCRETE_XIO_HANDLE tls_io);
extern int tlsio_schannel_send(CONCRETE_XIO_HANDLE tls_io, const void* buffer, size_t size, ON_SEND_COMPLETE on_send_complete, void* callback_context);
extern void tlsio_schannel_dowork(CONCRETE_XIO_HANDLE tls_io);
extern const XIO_INTERFACE_DESCRIPTION* tlsio_schannel_get_interface_description(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TLSIO_SCHANNEL_H */
