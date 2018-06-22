// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef XIO_ADAPTER_H
#define XIO_ADAPTER_H

#include "azure_c_shared_utility/xio.h"
#include "azure_c_shared_utility/umock_c_prod.h"
#include "azure_c_shared_utility/xio_async_result.h"

#ifdef __cplusplus
extern "C" {
#include <cstddef>
#include <cstdint>
#else
#include <stddef.h>
#include <stdint.h>
#endif /* __cplusplus */

// xio_adapter components combine with xio_state to form xio components

typedef struct XIO_ADAPTER_INTERFACE_TAG* XIO_ADAPTER_INTERFACE_HANDLE;
typedef struct XIO_ADAPTER_INSTANCE_TAG* XIO_ADAPTER_INSTANCE_HANDLE;

typedef XIO_ADAPTER_INSTANCE_HANDLE(*XIO_ADAPTER_CREATE)(void* create_parameters);
typedef void(*XIO_ADAPTER_DESTROY)(XIO_ADAPTER_INSTANCE_HANDLE xio_adapter_instance);
typedef XIO_ASYNC_RESULT(*XIO_ADAPTER_OPEN)(XIO_ADAPTER_INSTANCE_HANDLE xio_adapter_instance,
    ON_BYTES_RECEIVED on_received, void* on_received_context);
typedef XIO_ASYNC_RESULT(*XIO_ADAPTER_CLOSE)(XIO_ADAPTER_INSTANCE_HANDLE xio_adapter_instance);
typedef XIO_ASYNC_RESULT(*XIO_ADAPTER_READ)(XIO_ADAPTER_INSTANCE_HANDLE xio_adapter_instance);
typedef int(*XIO_ADAPTER_WRITE)(XIO_ADAPTER_INSTANCE_HANDLE xio_adapter_instance, const uint8_t* buffer, uint32_t count);

// Declarations for mocking only -- actual implementations are exposed only
// within an XIO_ADAPTER_CONFIG_INTERFACE
MOCKABLE_FUNCTION(, XIO_ADAPTER_INSTANCE_HANDLE, xio_adapter_create, void*, create_parameters);
MOCKABLE_FUNCTION(, void, xio_adapter_destroy, XIO_ADAPTER_INSTANCE_HANDLE, xio_endpoint);
MOCKABLE_FUNCTION(, XIO_ASYNC_RESULT, xio_adapter_open, XIO_ADAPTER_INSTANCE_HANDLE, xio_adapter_instance, ON_BYTES_RECEIVED, on_received, void*, on_received_context);
MOCKABLE_FUNCTION(, XIO_ASYNC_RESULT, xio_adapter_close, XIO_ADAPTER_INSTANCE_HANDLE, xio_adapter_instance);
// xio_adapter_read sends any data it receives out the callback from xio_adapter_open
MOCKABLE_FUNCTION(, XIO_ASYNC_RESULT, xio_adapter_read, XIO_ADAPTER_INSTANCE_HANDLE, xio_adapter_instance);
// write returns 0 for waiting, non-zero for data transferred, or XIO_ASYNC_WRITE_RESULT_FAILURE
MOCKABLE_FUNCTION(, int, xio_adapter_write, XIO_ADAPTER_INSTANCE_HANDLE, xio_adapter_instance, const uint8_t*, buffer, uint32_t, buffer_size);
MOCKABLE_FUNCTION(, int, xio_adapter_setoption, XIO_ADAPTER_INSTANCE_HANDLE, xio_adapter_instance, const char*, optionName, const void*, value);
MOCKABLE_FUNCTION(, OPTIONHANDLER_HANDLE, xio_adapter_retrieveoptions, XIO_ADAPTER_INSTANCE_HANDLE, xio_adapter_instance);

// XIO_ADAPTER_INTERFACE is always acquired through a concrete type, so there's no need
// for a generic get function.
typedef struct XIO_ADAPTER_INTERFACE_TAG
{
    XIO_ADAPTER_CREATE create;
    XIO_ADAPTER_DESTROY destroy;
    XIO_ADAPTER_OPEN open;
    XIO_ADAPTER_CLOSE close;
    XIO_ADAPTER_READ read;
    XIO_ADAPTER_WRITE write;
    IO_SETOPTION setoption;
    IO_RETRIEVEOPTIONS retrieveoptions;
} XIO_ADAPTER_INTERFACE;

// This macro declares an xio get interface function. 
// For tlsio, for example, this declared function would be called from "platform_get_default_tlsio".
// Use this macro at the bottom of your adapter's .c file.
#define DECLARE_XIO_GET_INTERFACE_FUNCTION(FILTER_INTERFACE, FUNC_NAME) \
static CONCRETE_IO_HANDLE local_xio_create(void* io_create_parameters) \
{ \
    return xio_state_create(&FILTER_INTERFACE, io_create_parameters); \
} \
\
static const IO_INTERFACE_DESCRIPTION local_xio_interface_description = \
{ \
    xio_state_retrieveoptions, \
    local_xio_create, \
    xio_state_destroy, \
    xio_state_open_async, \
    xio_state_close_async, \
    xio_state_send_async, \
    xio_state_dowork, \
    xio_state_setoption \
}; \
\
const IO_INTERFACE_DESCRIPTION* FUNC_NAME() \
{ \
    return &local_xio_interface_description; \
}



#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* XIO_ADAPTER_H */
