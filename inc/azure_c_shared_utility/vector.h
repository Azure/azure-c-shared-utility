// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef VECTOR_H
#define VECTOR_H

#include "azure_c_shared_utility/crt_abstractions.h"
#include "azure_c_shared_utility/umock_c_prod.h"
#include "azure_c_shared_utility/vector_types.h"

#ifdef __cplusplus
#include <cstddef>
extern "C"
{
#else
#include <stddef.h>
#include <stdbool.h>
#endif

/* creation */
DLLEXPORT MOCKABLE_FUNCTION(, VECTOR_HANDLE, VECTOR_create, size_t, elementSize);
DLLEXPORT MOCKABLE_FUNCTION(, VECTOR_HANDLE, VECTOR_move, VECTOR_HANDLE, handle);
DLLEXPORT MOCKABLE_FUNCTION(, void, VECTOR_destroy, VECTOR_HANDLE, handle);

/* insertion */
DLLEXPORT MOCKABLE_FUNCTION(, int, VECTOR_push_back, VECTOR_HANDLE, handle, const void*, elements, size_t, numElements);

/* removal */
DLLEXPORT MOCKABLE_FUNCTION(, void, VECTOR_erase, VECTOR_HANDLE, handle, void*, elements, size_t, numElements);
DLLEXPORT MOCKABLE_FUNCTION(, void, VECTOR_clear, VECTOR_HANDLE, handle);

/* access */
DLLEXPORT MOCKABLE_FUNCTION(, void*, VECTOR_element, VECTOR_HANDLE, handle, size_t, index);
DLLEXPORT MOCKABLE_FUNCTION(, void*, VECTOR_front, VECTOR_HANDLE, handle);
DLLEXPORT MOCKABLE_FUNCTION(, void*, VECTOR_back, VECTOR_HANDLE, handle);
DLLEXPORT MOCKABLE_FUNCTION(, void*, VECTOR_find_if, VECTOR_HANDLE, handle, PREDICATE_FUNCTION, pred, const void*, value);

/* capacity */
DLLEXPORT MOCKABLE_FUNCTION(, size_t, VECTOR_size, VECTOR_HANDLE, handle);

#ifdef __cplusplus
}
#endif

#endif /* VECTOR_H */
