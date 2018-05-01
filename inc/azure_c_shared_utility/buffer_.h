// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef BUFFER_H
#define BUFFER_H

#ifdef __cplusplus
#include <cstddef>
extern "C"
{
#else
#include <stddef.h>
#include <stdbool.h>
#endif

#include "azure_c_shared_utility/umock_c_prod.h"

typedef struct BUFFER_TAG* BUFFER_HANDLE;

DLLEXPORT MOCKABLE_FUNCTION(, BUFFER_HANDLE, BUFFER_new);
DLLEXPORT MOCKABLE_FUNCTION(, BUFFER_HANDLE, BUFFER_create, const unsigned char*, source, size_t, size);
DLLEXPORT MOCKABLE_FUNCTION(, void, BUFFER_delete, BUFFER_HANDLE, handle);
DLLEXPORT MOCKABLE_FUNCTION(, int, BUFFER_pre_build, BUFFER_HANDLE, handle, size_t, size);
DLLEXPORT MOCKABLE_FUNCTION(, int, BUFFER_build, BUFFER_HANDLE, handle, const unsigned char*, source, size_t, size);
DLLEXPORT MOCKABLE_FUNCTION(, int, BUFFER_append_build, BUFFER_HANDLE, handle, const unsigned char*, source, size_t, size);
DLLEXPORT MOCKABLE_FUNCTION(, int, BUFFER_unbuild, BUFFER_HANDLE, handle);
DLLEXPORT MOCKABLE_FUNCTION(, int, BUFFER_enlarge, BUFFER_HANDLE, handle, size_t, enlargeSize);
DLLEXPORT MOCKABLE_FUNCTION(, int, BUFFER_shrink, BUFFER_HANDLE, handle, size_t, decreaseSize, bool, fromEnd);
DLLEXPORT MOCKABLE_FUNCTION(, int, BUFFER_content, BUFFER_HANDLE, handle, const unsigned char**, content);
DLLEXPORT MOCKABLE_FUNCTION(, int, BUFFER_size, BUFFER_HANDLE, handle, size_t*, size);
DLLEXPORT MOCKABLE_FUNCTION(, int, BUFFER_append, BUFFER_HANDLE, handle1, BUFFER_HANDLE, handle2);
DLLEXPORT MOCKABLE_FUNCTION(, int, BUFFER_prepend, BUFFER_HANDLE, handle1, BUFFER_HANDLE, handle2);
DLLEXPORT MOCKABLE_FUNCTION(, int, BUFFER_fill, BUFFER_HANDLE, handle, unsigned char, fill_char);
DLLEXPORT MOCKABLE_FUNCTION(, unsigned char*, BUFFER_u_char, BUFFER_HANDLE, handle);
DLLEXPORT MOCKABLE_FUNCTION(, size_t, BUFFER_length, BUFFER_HANDLE, handle);
DLLEXPORT MOCKABLE_FUNCTION(, BUFFER_HANDLE, BUFFER_clone, BUFFER_HANDLE, handle);

#ifdef __cplusplus
}
#endif


#endif  /* BUFFER_H */
