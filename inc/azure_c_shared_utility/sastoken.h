// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef SASTOKEN_H
#define SASTOKEN_H

#include "azure_c_shared_utility/strings.h"
#include <stdbool.h>
#include "azure_c_shared_utility/umock_c_prod.h"

#ifdef __cplusplus
extern "C" {
#endif

    DLLEXPORT MOCKABLE_FUNCTION(, bool, SASToken_Validate, STRING_HANDLE, sasToken);
	DLLEXPORT MOCKABLE_FUNCTION(, STRING_HANDLE, SASToken_Create, STRING_HANDLE, key, STRING_HANDLE, scope, STRING_HANDLE, keyName, size_t, expiry);
	DLLEXPORT MOCKABLE_FUNCTION(, STRING_HANDLE, SASToken_CreateString, const char*, key, const char*, scope, const char*, keyName, size_t, expiry);

#ifdef __cplusplus
}
#endif

#endif /* SASTOKEN_H */
