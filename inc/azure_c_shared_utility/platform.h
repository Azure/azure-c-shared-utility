// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef PLATFORM_H
#define PLATFORM_H

#include "azure_c_shared_utility/strings.h"
#include "azure_c_shared_utility/xio.h"
#include "azure_c_shared_utility/umock_c_prod.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

    DLLEXPORT MOCKABLE_FUNCTION(, int, platform_init);
    DLLEXPORT MOCKABLE_FUNCTION(, void, platform_deinit);
	DLLEXPORT MOCKABLE_FUNCTION(, const IO_INTERFACE_DESCRIPTION*, platform_get_default_tlsio);
    DLLEXPORT MOCKABLE_FUNCTION(, STRING_HANDLE, platform_get_platform_info);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* PLATFORM_H */
