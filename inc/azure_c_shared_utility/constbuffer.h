// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef CONSTBUFFER_H
#define CONSTBUFFER_H

#include "azure_c_shared_utility/buffer_.h"

#ifdef __cplusplus
#include <cstddef>
extern "C"
{
#else
#include <stddef.h>
#include <stdbool.h>
#endif

#include "umock_c/umock_c_prod.h"

/*this is the handle*/
typedef struct CONSTBUFFER_HANDLE_DATA_TAG* CONSTBUFFER_HANDLE;

/*this is what is returned when the content of the buffer needs access*/
typedef struct CONSTBUFFER_TAG
{
    const unsigned char* buffer;
    size_t size;
} CONSTBUFFER;

typedef void(*CONSTBUFFER_CUSTOM_FREE_FUNC)(void* context);

/*this creates a new constbuffer from a memory area*/
MOCKABLE_FUNCTION(, CONSTBUFFER_HANDLE, CONSTBUFFER_Create, const unsigned char*, source, size_t, size);

/*this creates a new constbuffer from an existing BUFFER_HANDLE*/
MOCKABLE_FUNCTION(, CONSTBUFFER_HANDLE, CONSTBUFFER_CreateFromBuffer, BUFFER_HANDLE, buffer);

MOCKABLE_FUNCTION(, CONSTBUFFER_HANDLE, CONSTBUFFER_CreateWithMoveMemory, unsigned char*, source, size_t, size);

MOCKABLE_FUNCTION(, CONSTBUFFER_HANDLE, CONSTBUFFER_CreateWithCustomFree, const unsigned char*, source, size_t, size, CONSTBUFFER_CUSTOM_FREE_FUNC, customFreeFunc, void*, customFreeFuncContext);

MOCKABLE_FUNCTION(, void, CONSTBUFFER_IncRef, CONSTBUFFER_HANDLE, constbufferHandle);

MOCKABLE_FUNCTION(, void, CONSTBUFFER_DecRef, CONSTBUFFER_HANDLE, constbufferHandle);

MOCKABLE_FUNCTION(, const CONSTBUFFER*, CONSTBUFFER_GetContent, CONSTBUFFER_HANDLE, constbufferHandle);

MOCKABLE_FUNCTION(, bool, CONSTBUFFER_HANDLE_contain_same, CONSTBUFFER_HANDLE, left, CONSTBUFFER_HANDLE, right);

#ifdef __cplusplus
}
#endif

#endif  /* CONSTBUFFER_H */
