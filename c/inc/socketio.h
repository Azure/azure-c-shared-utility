// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef SOCKETIO_H
#define SOCKETIO_H

#ifdef __cplusplus
extern "C" {
#include <cstddef>
#else
#include <stddef.h>
#endif /* __cplusplus */

#include "xio.h"
#include "iot_logging.h"

typedef struct SOCKETIO_CONFIG_TAG
{
	const char* hostname;
	int port;
} SOCKETIO_CONFIG;

extern CONCRETE_XIO_HANDLE socketio_create(void* io_create_parameters, LOGGER_LOG logger_log);
extern void socketio_destroy(CONCRETE_XIO_HANDLE socket_io);
extern int socketio_open(CONCRETE_XIO_HANDLE socket_io, ON_BYTES_RECEIVED on_bytes_received, ON_IO_STATE_CHANGED on_io_state_changed, void* callback_context);
extern int socketio_close(CONCRETE_XIO_HANDLE socket_io);
extern int socketio_send(CONCRETE_XIO_HANDLE socket_io, const void* buffer, size_t size, ON_SEND_COMPLETE on_send_complete, void* callback_context);
extern void socketio_dowork(CONCRETE_XIO_HANDLE socket_io);
extern const XIO_INTERFACE_DESCRIPTION* socketio_get_interface_description(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* SOCKETIO_H */
