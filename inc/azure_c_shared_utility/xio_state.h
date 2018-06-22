// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef XIO_STATE_H
#define XIO_STATE_H

#include "azure_c_shared_utility/xio.h"
#include "azure_c_shared_utility/xio_adapter.h"
#include "azure_c_shared_utility/umock_c_prod.h"

#ifdef __cplusplus
extern "C" {
#include <cstddef>
#else
#include <stddef.h>
#endif /* __cplusplus */

// xio_state is a component that encapsulates state and callbacks for xio components

MOCKABLE_FUNCTION(, CONCRETE_IO_HANDLE, xio_state_create, 
        const XIO_ADAPTER_INTERFACE*, adapter_interface, void*, io_create_parameters);
    
// These functions implement the corresponding functions in your specific xio_get_interface_description
MOCKABLE_FUNCTION(, void, xio_state_destroy, CONCRETE_IO_HANDLE, xio);
MOCKABLE_FUNCTION(, int, xio_state_open_async, CONCRETE_IO_HANDLE, xio, ON_IO_OPEN_COMPLETE, on_open_complete, void*, 
    on_open_complete_context, ON_BYTES_RECEIVED, on_bytes_received, void*, on_bytes_received_context, ON_IO_ERROR, on_io_error, void*, on_io_error_context);
MOCKABLE_FUNCTION(, int, xio_state_close_async, CONCRETE_IO_HANDLE, xio, ON_IO_CLOSE_COMPLETE, on_close_complete, void*, callback_context);
MOCKABLE_FUNCTION(, int, xio_state_send_async, CONCRETE_IO_HANDLE, xio, const void*, buffer, size_t, size, ON_SEND_COMPLETE, on_send_complete, void*, callback_context);
MOCKABLE_FUNCTION(, void, xio_state_dowork, CONCRETE_IO_HANDLE, xio);
MOCKABLE_FUNCTION(, int, xio_state_setoption, CONCRETE_IO_HANDLE, xio, const char*, optionName, const void*, value);
MOCKABLE_FUNCTION(, OPTIONHANDLER_HANDLE, xio_state_retrieveoptions, CONCRETE_IO_HANDLE, xio);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* XIO_STATE_H */
