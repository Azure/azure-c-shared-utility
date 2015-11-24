// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdlib.h>
#ifdef _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#include <stddef.h>
#include "gballoc.h"
#include "transport_io.h"

typedef struct IO_INSTANCE_TAG
{
	const IO_INTERFACE_DESCRIPTION* io_interface_description;
	IO_HANDLE concrete_io_handle;
} IO_INSTANCE;

IO_HANDLE io_create(const IO_INTERFACE_DESCRIPTION* io_interface_description, const void* io_create_parameters, LOGGER_LOG logger_log)
{
	IO_INSTANCE* io_instance;
	/* Codes_SRS_IO_01_003: [If the argument io_interface_description is NULL, io_create shall return NULL.] */
	if ((io_interface_description == NULL) ||
		/* Codes_SRS_IO_01_004: [If any io_interface_description member is NULL, io_create shall return NULL.] */
		(io_interface_description->concrete_io_create == NULL) ||
		(io_interface_description->concrete_io_destroy == NULL) ||
		(io_interface_description->concrete_io_open == NULL) ||
		(io_interface_description->concrete_io_close == NULL) ||
		(io_interface_description->concrete_io_send == NULL) ||
		(io_interface_description->concrete_io_dowork == NULL))
	{
		io_instance = NULL;
	}
	else
	{
		io_instance = (IO_INSTANCE*)malloc(sizeof(IO_INSTANCE));

		/* Codes_SRS_IO_01_017: [If allocating the memory needed for the IO interface fails then io_create shall return NULL.] */
		if (io_instance != NULL)
		{
			/* Codes_SRS_IO_01_001: [io_create shall return on success a non-NULL handle to a new IO interface.] */
			io_instance->io_interface_description = io_interface_description;

			/* Codes_SRS_IO_01_002: [In order to instantiate the concrete IO implementation the function concrete_io_create from the io_interface_description shall be called, passing the io_create_parameters and logger_log arguments.] */
			io_instance->concrete_io_handle = io_instance->io_interface_description->concrete_io_create((void*)io_create_parameters, logger_log);

			/* Codes_SRS_IO_01_016: [If the underlying concrete_io_create call fails, io_create shall return NULL.] */
			if (io_instance->concrete_io_handle == NULL)
			{
				free(io_instance);
				io_instance = NULL;
			}
		}
	}
	return (IO_HANDLE)io_instance;
}

void io_destroy(IO_HANDLE io)
{
	/* Codes_SRS_IO_01_007: [If the argument io is NULL, io_destroy shall do nothing.] */
	if (io != NULL)
	{
		IO_INSTANCE* io_instance = (IO_INSTANCE*)io;

		/* Codes_SRS_IO_01_006: [io_destroy shall also call the concrete_io_destroy function that is member of the io_interface_description argument passed to io_create, while passing as argument to concrete_io_destroy the result of the underlying concrete_io_create handle that was called as part of the io_create call.] */
		io_instance->io_interface_description->concrete_io_destroy(io_instance->concrete_io_handle);

		/* Codes_SRS_IO_01_005: [io_destroy shall free all resources associated with the IO handle.] */
		free(io_instance);
	}
}

int io_open(IO_HANDLE io, ON_BYTES_RECEIVED on_bytes_received, ON_IO_STATE_CHANGED on_io_state_changed, void* callback_context)
{
	int result;

	if (io == NULL)
	{
		/* Codes_SRS_IO_01_021: [If handle is NULL, io_open shall return a non-zero value.] */
		result = __LINE__;
	}
	else
	{
		IO_INSTANCE* io_instance = (IO_INSTANCE*)io;

		/* Codes_SRS_IO_01_019: [io_open shall call the specific concrete_io_open function specified in io_create, passing the receive_callback and receive_callback_context arguments.] */
		if (io_instance->io_interface_description->concrete_io_open(io_instance->concrete_io_handle, on_bytes_received, on_io_state_changed, callback_context) != 0)
		{
			/* Codes_SRS_IO_01_022: [If the underlying concrete_io_open fails, io_open shall return a non-zero value.] */
			result = __LINE__;
		}
		else
		{
			/* Codes_SRS_IO_01_020: [On success, io_open shall return 0.] */
			result = 0;
		}
	}

	return result;
}

int io_close(IO_HANDLE io)
{
	int result;

	if (io == NULL)
	{
		/* Codes_SRS_IO_01_025: [If handle is NULL, io_close shall return a non-zero value.] */
		result = __LINE__;
	}
	else
	{
		IO_INSTANCE* io_instance = (IO_INSTANCE*)io;

		/* Codes_SRS_IO_01_023: [io_close shall call the specific concrete_io_close function specified in io_create.] */
		if (io_instance->io_interface_description->concrete_io_close(io_instance->concrete_io_handle) != 0)
		{
			/* Codes_SRS_IO_01_026: [If the underlying concrete_io_close fails, io_close shall return a non-zero value.] */
			result = __LINE__;
		}
		else
		{
			/* Codes_SRS_IO_01_024: [On success, io_close shall return 0.] */
			result = 0;
		}
	}

	return result;
}

int io_send(IO_HANDLE io, const void* buffer, size_t size, ON_SEND_COMPLETE on_send_complete, void* callback_context)
{
	int result;

	/* Codes_SRS_IO_01_011: [No error check shall be performed on buffer and size.] */
	/* Codes_SRS_IO_01_010: [If handle is NULL, io_send shall return a non-zero value.] */
	if (io == NULL)
	{
		result = __LINE__;
	}
	else
	{
		IO_INSTANCE* io_instance = (IO_INSTANCE*)io;

		/* Codes_SRS_IO_01_008: [io_send shall pass the sequence of bytes pointed to by buffer to the concrete IO implementation specified in io_create, by calling the concrete_io_send function while passing down the buffer and size arguments to it.] */
		/* Codes_SRS_IO_01_009: [On success, io_send shall return 0.] */
		/* Codes_SRS_IO_01_015: [If the underlying concrete_io_send fails, io_send shall return a non-zero value.] */
		/* Codes_SRS_IO_01_027: [io_send shall pass to the concrete_io_send function the on_send_complete and callback_context arguments.] */
		result = io_instance->io_interface_description->concrete_io_send(io_instance->concrete_io_handle, buffer, size, on_send_complete, callback_context);
	}

	return result;
}

void io_dowork(IO_HANDLE io)
{
	/* Codes_SRS_IO_01_018: [When the handle argument is NULL, io_dowork shall do nothing.] */
	if (io != NULL)
	{
		IO_INSTANCE* io_instance = (IO_INSTANCE*)io;

		/* Codes_SRS_IO_01_012: [io_dowork shall call the concrete IO implementation specified in io_create, by calling the concrete_io_dowork function.] */
		io_instance->io_interface_description->concrete_io_dowork(io_instance->concrete_io_handle);
	}
}
