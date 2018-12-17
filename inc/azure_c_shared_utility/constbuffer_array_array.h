// Copyright (C) Microsoft Corporation. All rights reserved.

#ifndef CONSTBUFFER_ARRAY_ARRAY_H
#define CONSTBUFFER_ARRAY_ARRAY_H

#include "azure_c_shared_utility/constbuffer_array.h"

#include "umock_c_prod.h"

#ifdef __cplusplus
#include <cstdint>
extern "C" {
#else
#include <stdint.h>
#endif

typedef struct CONSTBUFFER_ARRAY_ARRAY_HANDLE_DATA_TAG* CONSTBUFFER_ARRAY_ARRAY_HANDLE;

/*create*/
MOCKABLE_FUNCTION(, CONSTBUFFER_ARRAY_ARRAY_HANDLE, constbuffer_array_array_create, const CONSTBUFFER_ARRAY_HANDLE*, arrays, uint32_t, array_count);
MOCKABLE_FUNCTION(, CONSTBUFFER_ARRAY_ARRAY_HANDLE, constbuffer_array_array_create_empty);

MOCKABLE_FUNCTION(, void, constbuffer_array_array_inc_ref, CONSTBUFFER_ARRAY_ARRAY_HANDLE, constbuffer_array_array_handle);
MOCKABLE_FUNCTION(, void, constbuffer_array_array_dec_ref, CONSTBUFFER_ARRAY_ARRAY_HANDLE, constbuffer_array_array_handle);

/*add in front*/
MOCKABLE_FUNCTION(, CONSTBUFFER_ARRAY_ARRAY_HANDLE, constbuffer_array_array_add_front, CONSTBUFFER_ARRAY_ARRAY_HANDLE, constbuffer_array_array_handle, CONSTBUFFER_ARRAY_HANDLE, constbuffer_array_handle);

/*remove front*/
MOCKABLE_FUNCTION(, CONSTBUFFER_ARRAY_ARRAY_HANDLE, constbuffer_array_array_remove_front, CONSTBUFFER_ARRAY_ARRAY_HANDLE, constbuffer_array_array_handle, CONSTBUFFER_ARRAY_HANDLE *, constbuffer_array_handle);

/* getters */
MOCKABLE_FUNCTION(, int, constbuffer_array_array_get_array_count, CONSTBUFFER_ARRAY_ARRAY_HANDLE, constbuffer_array_array_handle, uint32_t*, array_count);
MOCKABLE_FUNCTION(, CONSTBUFFER_ARRAY_HANDLE, constbuffer_array_array_get_array, CONSTBUFFER_ARRAY_ARRAY_HANDLE, constbuffer_array_handle, uint32_t, array_index);
#ifdef __cplusplus
}
#endif

#endif /*CONSTBUFFER_ARRAY_ARRAY_H*/
