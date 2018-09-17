// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef STRING_TOKEN_H
#define STRING_TOKEN_H

#ifdef __cplusplus
#include <cstddef>
extern "C"
{
#else
#include <stddef.h>
#include <stdbool.h>
#endif

#include "azure_c_shared_utility/umock_c_prod.h"

typedef struct STRING_TOKEN_TAG* STRING_TOKEN_HANDLE;

MOCKABLE_FUNCTION(, STRING_TOKEN_HANDLE, StringToken_GetFirst, const char*, source, size_t, length, const char**, delimiters, size_t, n_delims);
MOCKABLE_FUNCTION(, bool, StringToken_GetNext, STRING_TOKEN_HANDLE, token, const char**, delimiters, size_t, n_delims);
MOCKABLE_FUNCTION(, const char*, StringToken_GetValue, STRING_TOKEN_HANDLE, token);
MOCKABLE_FUNCTION(, size_t, StringToken_GetLength, STRING_TOKEN_HANDLE, token);
MOCKABLE_FUNCTION(, const char*, StringToken_GetDelimiter, STRING_TOKEN_HANDLE, token);
MOCKABLE_FUNCTION(, void, StringToken_Destroy, STRING_TOKEN_HANDLE, token);

#ifdef __cplusplus
}
#endif

#endif  /*STRING_TOKEN_H*/
