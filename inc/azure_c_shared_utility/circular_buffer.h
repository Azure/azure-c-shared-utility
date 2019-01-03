// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef CIRCULAR_BUFFER_H
#define CIRCULAR_BUFFER_H

#ifdef __cplusplus
#include <cstddef>
extern "C"
{
#else
#include <stddef.h>
#endif

#include "azure_c_shared_utility/umock_c_prod.h"

typedef struct CIRCULAR_BUFFER_TAG* CIRCULAR_BUFFER_HANDLE;

MOCKABLE_FUNCTION(, CIRCULAR_BUFFER_HANDLE, circular_buffer_create, size_t, size);
MOCKABLE_FUNCTION(, void, circular_buffer_destroy, CIRCULAR_BUFFER_HANDLE, handle);
MOCKABLE_FUNCTION(, int, circular_buffer_write, CIRCULAR_BUFFER_HANDLE, handle, const unsigned char*, data, size_t, length);
MOCKABLE_FUNCTION(, size_t, circular_buffer_read, CIRCULAR_BUFFER_HANDLE, handle, unsigned char*, data, size_t, length);
MOCKABLE_FUNCTION(, int, circular_buffer_get_data_size, CIRCULAR_BUFFER_HANDLE, handle, size_t*, size);

#ifdef __cplusplus
}
#endif


#endif  /* CIRCULAR_BUFFER_H */
