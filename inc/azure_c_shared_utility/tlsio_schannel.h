// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef TLSIO_SCHANNEL_H
#define TLSIO_SCHANNEL_H

#include "azure_c_shared_utility/xio.h"
#include "azure_c_shared_utility/umock_c_prod.h"

#ifdef __cplusplus
extern "C" {
#include <cstddef>
#else
#include <stddef.h>
#endif /* __cplusplus */

#define TLSIO_STATE_VALUES                        \
    TLSIO_STATE_NOT_OPEN,                         \
    TLSIO_STATE_OPENING_UNDERLYING_IO,            \
    TLSIO_STATE_HANDSHAKE_CLIENT_HELLO_SENT,      \
    TLSIO_STATE_HANDSHAKE_SERVER_HELLO_RECEIVED,  \
    TLSIO_STATE_RENEGOTIATE,                      \
    TLSIO_STATE_OPEN,                             \
    TLSIO_STATE_CLOSING,                          \
    TLSIO_STATE_ERROR

DEFINE_ENUM(TLSIO_STATE, TLSIO_STATE_VALUES);

DLLEXPORT MOCKABLE_FUNCTION(, CONCRETE_IO_HANDLE, tlsio_schannel_create, void*, io_create_parameters);
DLLEXPORT MOCKABLE_FUNCTION(, void, tlsio_schannel_destroy, CONCRETE_IO_HANDLE, tls_io);
DLLEXPORT MOCKABLE_FUNCTION(, int, tlsio_schannel_open, CONCRETE_IO_HANDLE, tls_io, ON_IO_OPEN_COMPLETE, on_io_open_complete, void*, on_io_open_complete_context, ON_BYTES_RECEIVED, on_bytes_received, void*, on_bytes_received_context, ON_IO_ERROR, on_io_error, void*, on_io_error_context);
DLLEXPORT MOCKABLE_FUNCTION(, int, tlsio_schannel_close, CONCRETE_IO_HANDLE, tls_io, ON_IO_CLOSE_COMPLETE, on_io_close_complete, void*, callback_context);
DLLEXPORT MOCKABLE_FUNCTION(, int, tlsio_schannel_send, CONCRETE_IO_HANDLE, tls_io, const void*, buffer, size_t, size, ON_SEND_COMPLETE, on_send_complete, void*, callback_context);
DLLEXPORT MOCKABLE_FUNCTION(, void, tlsio_schannel_dowork, CONCRETE_IO_HANDLE, tls_io);
DLLEXPORT MOCKABLE_FUNCTION(, int, tlsio_schannel_setoption, CONCRETE_IO_HANDLE, tls_io, const char*, optionName, const void*, value);

DLLEXPORT MOCKABLE_FUNCTION(, const IO_INTERFACE_DESCRIPTION*, tlsio_schannel_get_interface_description);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TLSIO_SCHANNEL_H */
