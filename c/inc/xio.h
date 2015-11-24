// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef XIO_H
#define XIO_H

#include "iot_logging.h"

#ifdef __cplusplus
#include <cstddef>
extern "C" {
#else
#include <stddef.h>
#endif /* __cplusplus */

	typedef struct XIO_INSTANCE_TAG* XIO_HANDLE;
	typedef void* CONCRETE_IO_HANDLE;

	typedef enum XIO_STATE_TAG
	{
		XIO_STATE_NOT_OPEN,
		XIO_STATE_OPENING,
		XIO_STATE_OPEN,
		XIO_STATE_ERROR
	} XIO_STATE;

	typedef enum XIO_SEND_RESULT_TAG
	{
		XIO_SEND_OK,
		XIO_SEND_ERROR
	} XIO_SEND_RESULT;

	typedef void(*ON_BYTES_RECEIVED)(void* context, const void* buffer, size_t size);
	typedef void(*ON_SEND_COMPLETE)(void* context, XIO_SEND_RESULT send_result);
	typedef void(*ON_IO_STATE_CHANGED)(void* context, XIO_STATE new_io_state, XIO_STATE previous_io_state);

	typedef CONCRETE_IO_HANDLE(*XIO_CREATE)(void* io_create_parameters, LOGGER_LOG logger_log);
	typedef void(*XIO_DESTROY)(CONCRETE_IO_HANDLE concrete_io);
	typedef int(*XIO_OPEN)(CONCRETE_IO_HANDLE concrete_io, ON_BYTES_RECEIVED on_bytes_received, ON_IO_STATE_CHANGED on_io_state_changed, void* callback_context);
	typedef int(*XIO_CLOSE)(CONCRETE_IO_HANDLE concrete_io);
	typedef int(*XIO_SEND)(CONCRETE_IO_HANDLE concrete_io, const void* buffer, size_t size, ON_SEND_COMPLETE on_send_complete, void* callback_context);
	typedef void(*XIO_DOWORK)(CONCRETE_IO_HANDLE concrete_io);

	typedef struct IO_INTERFACE_DESCRIPTION_TAG
	{
		XIO_CREATE concrete_io_create;
		XIO_DESTROY concrete_io_destroy;
		XIO_OPEN concrete_io_open;
		XIO_CLOSE concrete_io_close;
		XIO_SEND concrete_io_send;
		XIO_DOWORK concrete_io_dowork;
	} IO_INTERFACE_DESCRIPTION;

	extern XIO_HANDLE xio_create(const IO_INTERFACE_DESCRIPTION* xio_interface_description, const void* io_create_parameters, LOGGER_LOG logger_log);
	extern void xio_destroy(XIO_HANDLE xio);
	extern int xio_open(XIO_HANDLE xio, ON_BYTES_RECEIVED on_bytes_received, ON_IO_STATE_CHANGED on_io_state_changed, void* callback_context);
	extern int xio_close(XIO_HANDLE xio);
	extern int xio_send(XIO_HANDLE xio, const void* buffer, size_t size, ON_SEND_COMPLETE on_send_complete, void* callback_context);
	extern void xio_dowork(XIO_HANDLE xio);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* XIO_H */
