// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef TLSIO_BEARSSL_H
#define TLSIO_BEARSSL_H

#include "azure_c_shared_utility/xio.h"
#include "azure_c_shared_utility/xlogging.h"
#include "azure_c_shared_utility/optionhandler.h"

#ifdef __cplusplus
extern "C" {
#include <cstddef>
#else
#include <stddef.h>
#endif /* __cplusplus */

extern const IO_INTERFACE_DESCRIPTION* tlsio_bearssl_get_interface_description(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TLSIO_BEARSSL_H */
