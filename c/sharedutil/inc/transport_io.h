// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef IO_H
#define IO_H

#include "iot_logging.h"

#ifdef __cplusplus
#include <cstddef>
extern "C" {
#else
#include <stddef.h>
#endif /* __cplusplus */

	typedef void* IO_HANDLE;
	typedef void* CONCRETE_IO_HANDLE;

	typedef enum IO_STATE_TAG
	{
		IO_STATE_NOT_OPEN,
		IO_STATE_OPENING,
		IO_STATE_OPEN,
		IO_STATE_ERROR
	} IO_STATE;

	typedef void(*ON_BYTES_RECEIVED)(void* context, const void* buffer, size_t size);
	typedef void(*ON_IO_STATE_CHANGED)(void* context, IO_STATE new_io_state, IO_STATE previous_io_state);

	typedef CONCRETE_IO_HANDLE(*IO_CREATE)(void* io_create_parameters, LOGGER_LOG logger_log);
	typedef void(*IO_DESTROY)(CONCRETE_IO_HANDLE handle);
	typedef int(*IO_OPEN)(CONCRETE_IO_HANDLE handle, ON_BYTES_RECEIVED on_bytes_received, ON_IO_STATE_CHANGED on_io_state_changed, void* callback_context);
	typedef int(*IO_CLOSE)(CONCRETE_IO_HANDLE handle);
	typedef int(*IO_SEND)(CONCRETE_IO_HANDLE handle, const void* buffer, size_t size);
	typedef void(*IO_DOWORK)(CONCRETE_IO_HANDLE handle);

	typedef struct IO_INTERFACE_DESCRIPTION_TAG
	{
		IO_CREATE concrete_io_create;
		IO_DESTROY concrete_io_destroy;
		IO_OPEN concrete_io_open;
		IO_CLOSE concrete_io_close;
		IO_SEND concrete_io_send;
		IO_DOWORK concrete_io_dowork;
	} IO_INTERFACE_DESCRIPTION;

	extern IO_HANDLE io_create(const IO_INTERFACE_DESCRIPTION* io_interface_description, const void* io_create_parameters, LOGGER_LOG logger_log);
	extern void io_destroy(IO_HANDLE handle);
	extern int io_open(IO_HANDLE handle, ON_BYTES_RECEIVED on_bytes_received, ON_IO_STATE_CHANGED on_io_state_changed, void* callback_context);
	extern int io_close(IO_HANDLE handle);
	extern int io_send(IO_HANDLE handle, const void* buffer, size_t size);
	extern void io_dowork(IO_HANDLE handle);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* IO_H */
