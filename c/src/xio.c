// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdlib.h>
#ifdef _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#include <stddef.h>
#include "gballoc.h"
#include "xio.h"

typedef struct XIO_INSTANCE_TAG
{
	const XIO_INTERFACE_DESCRIPTION* xio_interface_description;
	XIO_HANDLE concrete_xio_handle;
} XIO_INSTANCE;

XIO_HANDLE xio_create(const XIO_INTERFACE_DESCRIPTION* xio_interface_description, const void* xio_create_parameters, LOGGER_LOG logger_log)
{
	XIO_INSTANCE* xio_instance;
	/* Codes_SRS_IO_01_003: [If the argument xio_interface_description is NULL, xio_create shall return NULL.] */
	if ((xio_interface_description == NULL) ||
		/* Codes_SRS_IO_01_004: [If any xio_interface_description member is NULL, xio_create shall return NULL.] */
		(xio_interface_description->concrete_xio_create == NULL) ||
		(xio_interface_description->concrete_xio_destroy == NULL) ||
		(xio_interface_description->concrete_xio_open == NULL) ||
		(xio_interface_description->concrete_xio_close == NULL) ||
		(xio_interface_description->concrete_xio_send == NULL) ||
		(xio_interface_description->concrete_xio_dowork == NULL))
	{
		xio_instance = NULL;
	}
	else
	{
		xio_instance = (XIO_INSTANCE*)malloc(sizeof(XIO_INSTANCE));

		/* Codes_SRS_IO_01_017: [If allocating the memory needed for the IO interface fails then xio_create shall return NULL.] */
		if (xio_instance != NULL)
		{
			/* Codes_SRS_IO_01_001: [xio_create shall return on success a non-NULL handle to a new IO interface.] */
			xio_instance->xio_interface_description = xio_interface_description;

			/* Codes_SRS_IO_01_002: [In order to instantiate the concrete IO implementation the function concrete_xio_create from the xio_interface_description shall be called, passing the xio_create_parameters and logger_log arguments.] */
			xio_instance->concrete_xio_handle = xio_instance->xio_interface_description->concrete_xio_create((void*)xio_create_parameters, logger_log);

			/* Codes_SRS_IO_01_016: [If the underlying concrete_xio_create call fails, xio_create shall return NULL.] */
			if (xio_instance->concrete_xio_handle == NULL)
			{
				free(xio_instance);
				xio_instance = NULL;
			}
		}
	}
	return (XIO_HANDLE)xio_instance;
}

void xio_destroy(XIO_HANDLE xio)
{
	/* Codes_SRS_IO_01_007: [If the argument io is NULL, xio_destroy shall do nothing.] */
	if (xio != NULL)
	{
		XIO_INSTANCE* xio_instance = (XIO_INSTANCE*)xio;

		/* Codes_SRS_IO_01_006: [xio_destroy shall also call the concrete_xio_destroy function that is member of the xio_interface_description argument passed to xio_create, while passing as argument to concrete_xio_destroy the result of the underlying concrete_xio_create handle that was called as part of the xio_create call.] */
		xio_instance->xio_interface_description->concrete_xio_destroy(xio_instance->concrete_xio_handle);

		/* Codes_SRS_IO_01_005: [xio_destroy shall free all resources associated with the IO handle.] */
		free(xio_instance);
	}
}

int xio_open(XIO_HANDLE xio, ON_BYTES_RECEIVED on_bytes_received, ON_IO_STATE_CHANGED on_xio_state_changed, void* callback_context)
{
	int result;

	if (xio == NULL)
	{
		/* Codes_SRS_IO_01_021: [If handle is NULL, xio_open shall return a non-zero value.] */
		result = __LINE__;
	}
	else
	{
		XIO_INSTANCE* xio_instance = (XIO_INSTANCE*)xio;

		/* Codes_SRS_IO_01_019: [xio_open shall call the specific concrete_xio_open function specified in xio_create, passing the receive_callback and receive_callback_context arguments.] */
		if (xio_instance->xio_interface_description->concrete_xio_open(xio_instance->concrete_xio_handle, on_bytes_received, on_xio_state_changed, callback_context) != 0)
		{
			/* Codes_SRS_IO_01_022: [If the underlying concrete_xio_open fails, xio_open shall return a non-zero value.] */
			result = __LINE__;
		}
		else
		{
			/* Codes_SRS_IO_01_020: [On success, xio_open shall return 0.] */
			result = 0;
		}
	}

	return result;
}

int xio_close(XIO_HANDLE xio)
{
	int result;

	if (xio == NULL)
	{
		/* Codes_SRS_IO_01_025: [If handle is NULL, xio_close shall return a non-zero value.] */
		result = __LINE__;
	}
	else
	{
		XIO_INSTANCE* xio_instance = (XIO_INSTANCE*)xio;

		/* Codes_SRS_IO_01_023: [xio_close shall call the specific concrete_xio_close function specified in xio_create.] */
		if (xio_instance->xio_interface_description->concrete_xio_close(xio_instance->concrete_xio_handle) != 0)
		{
			/* Codes_SRS_IO_01_026: [If the underlying concrete_xio_close fails, xio_close shall return a non-zero value.] */
			result = __LINE__;
		}
		else
		{
			/* Codes_SRS_IO_01_024: [On success, xio_close shall return 0.] */
			result = 0;
		}
	}

	return result;
}

int xio_send(XIO_HANDLE xio, const void* buffer, size_t size, ON_SEND_COMPLETE on_send_complete, void* callback_context)
{
	int result;

	/* Codes_SRS_IO_01_011: [No error check shall be performed on buffer and size.] */
	/* Codes_SRS_IO_01_010: [If handle is NULL, xio_send shall return a non-zero value.] */
	if (xio == NULL)
	{
		result = __LINE__;
	}
	else
	{
		XIO_INSTANCE* xio_instance = (XIO_INSTANCE*)xio;

		/* Codes_SRS_IO_01_008: [xio_send shall pass the sequence of bytes pointed to by buffer to the concrete IO implementation specified in xio_create, by calling the concrete_xio_send function while passing down the buffer and size arguments to it.] */
		/* Codes_SRS_IO_01_009: [On success, xio_send shall return 0.] */
		/* Codes_SRS_IO_01_015: [If the underlying concrete_xio_send fails, xio_send shall return a non-zero value.] */
		/* Codes_SRS_IO_01_027: [xio_send shall pass to the concrete_xio_send function the on_send_complete and callback_context arguments.] */
		result = xio_instance->xio_interface_description->concrete_xio_send(xio_instance->concrete_xio_handle, buffer, size, on_send_complete, callback_context);
	}

	return result;
}

void xio_dowork(XIO_HANDLE xio)
{
	/* Codes_SRS_IO_01_018: [When the handle argument is NULL, xio_dowork shall do nothing.] */
	if (xio != NULL)
	{
		XIO_INSTANCE* xio_instance = (XIO_INSTANCE*)xio;

		/* Codes_SRS_IO_01_012: [xio_dowork shall call the concrete XIO implementation specified in xio_create, by calling the concrete_xio_dowork function.] */
		xio_instance->xio_interface_description->concrete_xio_dowork(xio_instance->concrete_xio_handle);
	}
}
