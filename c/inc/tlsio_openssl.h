// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef TLSIO_OPENSSL_H
#define TLSIO_OPENSSL_H

#ifdef __cplusplus
extern "C" {
#include <cstddef>
#else
#include <stddef.h>
#endif /* __cplusplus */

#include "xio.h"
#include "iot_logging.h"

extern int tlsio_openssl_init(void);
extern void tlsio_openssl_deinit(void);

extern CONCRETE_IO_HANDLE tlsio_openssl_create(void* io_create_parameters, LOGGER_LOG logger_log);
extern void tlsio_openssl_destroy(CONCRETE_IO_HANDLE tls_io);
extern int tlsio_openssl_open(CONCRETE_IO_HANDLE tls_io, ON_IO_OPEN_COMPLETE on_io_open_complete, ON_BYTES_RECEIVED on_bytes_received, ON_IO_ERROR on_io_error, void* callback_context);
extern int tlsio_openssl_close(CONCRETE_IO_HANDLE tls_io, ON_IO_CLOSE_COMPLETE on_io_close_complete, void* callback_context);
extern int tlsio_openssl_send(CONCRETE_IO_HANDLE tls_io, const void* buffer, size_t size, ON_SEND_COMPLETE on_send_complete, void* callback_context);
extern void tlsio_openssl_dowork(CONCRETE_IO_HANDLE tls_io);
extern const IO_INTERFACE_DESCRIPTION* tlsio_openssl_get_interface_description(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TLSIO_OPENSSL_H */
