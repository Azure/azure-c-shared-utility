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

extern CONCRETE_IO_HANDLE tlsio_schannel_create(void* io_create_parameters, LOGGER_LOG logger_log);
extern void tlsio_schannel_destroy(CONCRETE_IO_HANDLE tls_io);
extern int tlsio_schannel_open(CONCRETE_IO_HANDLE tls_io, ON_IO_OPEN_COMPLETE on_io_open_complete, ON_BYTES_RECEIVED on_bytes_received, ON_IO_ERROR on_io_error, void* callback_context);
extern int tlsio_schannel_close(CONCRETE_IO_HANDLE tls_io, ON_IO_CLOSE_COMPLETE on_io_close_complete, void* callback_context);
extern int tlsio_schannel_send(CONCRETE_IO_HANDLE tls_io, const void* buffer, size_t size, ON_SEND_COMPLETE on_send_complete, void* callback_context);
extern void tlsio_schannel_dowork(CONCRETE_IO_HANDLE tls_io);
extern const IO_INTERFACE_DESCRIPTION* tlsio_schannel_get_interface_description(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TLSIO_SCHANNEL_H */
