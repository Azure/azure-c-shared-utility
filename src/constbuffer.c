// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>
#include "azure_c_shared_utility/gballoc.h"
#include "azure_c_shared_utility/constbuffer.h"
#include "azure_c_shared_utility/xlogging.h"
#include "azure_c_shared_utility/refcount.h"

typedef struct CONSTBUFFER_HANDLE_DATA_TAG
{
    CONSTBUFFER alias;
    COUNT_TYPE count;
    bool memory_moved;
} CONSTBUFFER_HANDLE_DATA;

static CONSTBUFFER_HANDLE CONSTBUFFER_Create_Internal(const unsigned char* source, size_t size)
{
    CONSTBUFFER_HANDLE result;
    /*Codes_SRS_CONSTBUFFER_02_005: [The non-NULL handle returned by CONSTBUFFER_Create shall have its ref count set to "1".]*/
    /*Codes_SRS_CONSTBUFFER_02_010: [The non-NULL handle returned by CONSTBUFFER_CreateFromBuffer shall have its ref count set to "1".]*/
    result = (CONSTBUFFER_HANDLE)malloc(sizeof(CONSTBUFFER_HANDLE_DATA) + size);
    if (result == NULL)
    {
        /*Codes_SRS_CONSTBUFFER_02_003: [If creating the copy fails then CONSTBUFFER_Create shall return NULL.]*/
        /*Codes_SRS_CONSTBUFFER_02_008: [If copying the content fails, then CONSTBUFFER_CreateFromBuffer shall fail and return NULL.] */
        LogError("unable to malloc");
        /*return as is*/
    }
    else
    {
        INIT_REF_VAR(result->count);

        /*Codes_SRS_CONSTBUFFER_02_002: [Otherwise, CONSTBUFFER_Create shall create a copy of the memory area pointed to by source having size bytes.]*/
        result->alias.size = size;
        if (size == 0)
        {
            result->alias.buffer = NULL;
        }
        else
        {
            unsigned char* temp = (void*)(result + 1);
            /*Codes_SRS_CONSTBUFFER_02_004: [Otherwise CONSTBUFFER_Create shall return a non-NULL handle.]*/
            /*Codes_SRS_CONSTBUFFER_02_007: [Otherwise, CONSTBUFFER_CreateFromBuffer shall copy the content of buffer.]*/
            /*Codes_SRS_CONSTBUFFER_02_009: [Otherwise, CONSTBUFFER_CreateFromBuffer shall return a non-NULL handle.]*/
            (void)memcpy(temp, source, size);
            result->alias.buffer = temp;
        }

        result->memory_moved = false;
    }
    return result;
}

CONSTBUFFER_HANDLE CONSTBUFFER_Create(const unsigned char* source, size_t size)
{
    CONSTBUFFER_HANDLE result;
    /*Codes_SRS_CONSTBUFFER_02_001: [If source is NULL and size is different than 0 then CONSTBUFFER_Create shall fail and return NULL.]*/
    if (
        (source == NULL) &&
        (size != 0)
        )
    {
        LogError("invalid arguments passes to CONSTBUFFER_Create");
        result = NULL;
    }
    else
    {
        result = CONSTBUFFER_Create_Internal(source, size);
    }
    return result;
}

/*this creates a new constbuffer from an existing BUFFER_HANDLE*/
CONSTBUFFER_HANDLE CONSTBUFFER_CreateFromBuffer(BUFFER_HANDLE buffer)
{
    CONSTBUFFER_HANDLE result;
    /*Codes_SRS_CONSTBUFFER_02_006: [If buffer is NULL then CONSTBUFFER_CreateFromBuffer shall fail and return NULL.]*/
    if (buffer == NULL)
    {
        LogError("invalid arg passed to CONSTBUFFER_CreateFromBuffer");
        result = NULL;
    }
    else
    {
        size_t length = BUFFER_length(buffer);
        unsigned char* rawBuffer = BUFFER_u_char(buffer);
        result = CONSTBUFFER_Create_Internal(rawBuffer, length);
    }
    return result;
}

CONSTBUFFER_HANDLE CONSTBUFFER_CreateWithMoveMemory(unsigned char* source, size_t size)
{
    CONSTBUFFER_HANDLE result;

    /* Codes_SRS_CONSTBUFFER_01_001: [ If source is NULL and size is different than 0 then CONSTBUFFER_Create shall fail and return NULL. ]*/
    if ((source == NULL) && (size > 0))
    {
        LogError("Invalid arguments: unsigned char* source=%p, size_t size=%u", source, (unsigned int)size);
        result = NULL;
    }
    else
    {
        result = (CONSTBUFFER_HANDLE)malloc(sizeof(CONSTBUFFER_HANDLE_DATA));
        if (result == NULL)
        {
            /* Codes_SRS_CONSTBUFFER_01_005: [ If any error occurs, `CONSTBUFFER_CreateWithMoveMemory` shall fail and return NULL. ]*/
            LogError("malloc failed");
        }
        else
        {
            /* Codes_SRS_CONSTBUFFER_01_004: [ If `source` is non-NULL and `size` is 0, the `source` pointer shall be owned (and freed) by the newly created instance of const buffer. ]*/
            /* Codes_SRS_CONSTBUFFER_01_002: [ Otherwise, CONSTBUFFER_Create shall store the source and size and return a non-NULL handle to the newly created const buffer. ]*/
            result->alias.buffer = source;
            result->alias.size = size;
            result->memory_moved = true;

            /* Codes_SRS_CONSTBUFFER_01_003: [ The non-NULL handle returned by CONSTBUFFER_CreateWithMoveMemory shall have its ref count set to "1". ]*/
            INIT_REF_VAR(result->count);
        }
    }

    return result;
}

CONSTBUFFER_HANDLE CONSTBUFFER_Clone(CONSTBUFFER_HANDLE constbufferHandle)
{
    if (constbufferHandle == NULL)
    {
        /*Codes_SRS_CONSTBUFFER_02_013: [If constbufferHandle is NULL then CONSTBUFFER_Clone shall fail and return NULL.]*/
        LogError("invalid arg");
    }
    else
    {
        /*Codes_SRS_CONSTBUFFER_02_014: [Otherwise, CONSTBUFFER_Clone shall increment the reference count and return constbufferHandle.]*/
        INC_REF_VAR(constbufferHandle->count);
    }
    return constbufferHandle;
}

const CONSTBUFFER* CONSTBUFFER_GetContent(CONSTBUFFER_HANDLE constbufferHandle)
{
    const CONSTBUFFER* result;
    if (constbufferHandle == NULL)
    {
        /*Codes_SRS_CONSTBUFFER_02_011: [If constbufferHandle is NULL then CONSTBUFFER_GetContent shall return NULL.]*/
        result = NULL;
        LogError("invalid arg");
    }
    else
    {
        /*Codes_SRS_CONSTBUFFER_02_012: [Otherwise, CONSTBUFFER_GetContent shall return a const CONSTBUFFER* that matches byte by byte the original bytes used to created the const buffer and has the same length.]*/
        result = &(constbufferHandle->alias);
    }
    return result;
}

void CONSTBUFFER_Destroy(CONSTBUFFER_HANDLE constbufferHandle)
{
    /*Codes_SRS_CONSTBUFFER_02_015: [If constbufferHandle is NULL then CONSTBUFFER_Destroy shall do nothing.]*/
    if (constbufferHandle != NULL)
    {
        /*Codes_SRS_CONSTBUFFER_02_016: [Otherwise, CONSTBUFFER_Destroy shall decrement the refcount on the constbufferHandle handle.]*/
        if (DEC_REF_VAR(constbufferHandle->count) == DEC_RETURN_ZERO)
        {
            if (constbufferHandle->memory_moved)
            {
                free((void*)constbufferHandle->alias.buffer);
            }

            /*Codes_SRS_CONSTBUFFER_02_017: [If the refcount reaches zero, then CONSTBUFFER_Destroy shall deallocate all resources used by the CONSTBUFFER_HANDLE.]*/
            free(constbufferHandle);
        }
    }
}
