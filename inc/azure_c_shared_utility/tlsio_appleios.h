// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef TLSIO_APPLEIOS_H
#define TLSIO_APPLEIOS_H

// DEPRECATED: tls_config will be removed from the tree.
#include "azure_c_shared_utility/tls_config.h"

#ifdef __cplusplus
extern "C" {
#include <cstddef>
#else
#include <stddef.h>
#endif /* __cplusplus */

#include "azure_c_shared_utility/xio.h"
#include "azure_c_shared_utility/umock_c_prod.h"
#include "azure_c_shared_utility/xlogging.h"
#include "azure_c_shared_utility/optionhandler.h"

/** @brief	Return the tlsio table of functions.
*
* @param	void.
*
* @return	The tlsio interface (IO_INTERFACE_DESCRIPTION).
*/
extern const IO_INTERFACE_DESCRIPTION* tlsio_appleios_get_interface_description(void);

/** Expose tlsio state for test proposes.
*/
#define TLSIO_APPLEIOS_STATE_VALUES  \
    TLSIO_APPLEIOS_STATE_CLOSED,     \
    TLSIO_APPLEIOS_STATE_OPENING,    \
    TLSIO_APPLEIOS_STATE_OPEN,       \
    TLSIO_APPLEIOS_STATE_CLOSING,    \
    TLSIO_APPLEIOS_STATE_ERROR,      \
    TLSIO_APPLEIOS_STATE_NULL
DEFINE_ENUM(TLSIO_APPLEIOS_STATE, TLSIO_APPLEIOS_STATE_VALUES);


/** @brief	Return the tlsio state for test proposes.
*
* @param	Unique handle that identifies the tlsio instance.
*
* @return	The tlsio state (TLSIO_APPLEIOS_STATE).
*/
TLSIO_APPLEIOS_STATE tlsio_arduino_get_state(CONCRETE_IO_HANDLE tlsio_handle);
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TLSIO_APPLEIOS_H */