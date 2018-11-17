// Copyright (C) Microsoft Corporation. All rights reserved.

#include <stdlib.h>
#include <stdbool.h>
#include <inttypes.h>

#include "azure_c_shared_utility/gballoc.h"
#include "azure_c_shared_utility/xlogging.h"
#include "azure_c_shared_utility/optimize_size.h"

#include "azure_c_shared_utility/constbuffer.h"
#include "azure_c_shared_utility/constbuffer_array.h"
#include "azure_c_shared_utility/refcount.h"

typedef struct CONSTBUFFER_ARRAY_HANDLE_DATA_TAG
{
    uint32_t nBuffers;
#ifdef _MSC_VER
    /*warning C4200: nonstandard extension used: zero-sized array in struct/union : looks very standard in C99 and it is called flexible array. Documentation-wise is a flexible array, but called "unsized" in Microsoft's docs*/ /*https://msdn.microsoft.com/en-us/library/b6fae073.aspx*/
#pragma warning(disable:4200)
#endif
    CONSTBUFFER_HANDLE buffers[];
} CONSTBUFFER_ARRAY_HANDLE_DATA;

DEFINE_REFCOUNT_TYPE(CONSTBUFFER_ARRAY_HANDLE_DATA);

CONSTBUFFER_ARRAY_HANDLE constbuffer_array_create(const CONSTBUFFER_HANDLE* buffers, uint32_t buffer_count)
{
    CONSTBUFFER_ARRAY_HANDLE result;

    if (
        /* Codes_SRS_CONSTBUFFER_ARRAY_01_012: [ If `buffers` is NULL and `buffer_count` is not 0, `constbuffer_array_create` shall fail and return NULL. ]*/
        (buffers == NULL) && (buffer_count != 0)
        )
    {
        LogError("Invalid arguments: const CONSTBUFFER_HANDLE* buffers=%p, uint32_t buffer_count=%" PRIu32,
            buffers, buffer_count);
    }
    else
    {
        /* Codes_SRS_CONSTBUFFER_ARRAY_01_009: [ `constbuffer_array_create` shall allocate memory for a new `CONSTBUFFER_ARRAY_HANDLE` that can hold `buffer_count` buffers. ]*/
        result = REFCOUNT_TYPE_CREATE_WITH_EXTRA_SIZE(CONSTBUFFER_ARRAY_HANDLE_DATA, buffer_count * sizeof(CONSTBUFFER_HANDLE));
        if (result == NULL)
        {
            /* Codes_SRS_CONSTBUFFER_ARRAY_01_014: [ If any error occurs, `constbuffer_array_create` shall fail and return NULL. ]*/
            LogError("failure in allocating const buffer array");
        }
        else
        {
            uint32_t i;
            for (i = 0; i < buffer_count; i++)
            {
                /* Codes_SRS_CONSTBUFFER_ARRAY_01_010: [ `constbuffer_array_create` shall clone the buffers in `buffers` and store them. ]*/
                result->buffers[i] = CONSTBUFFER_Clone(buffers[i]);
                if (result->buffers[i] == NULL)
                {
                    /* Codes_SRS_CONSTBUFFER_ARRAY_01_014: [ If any error occurs, `constbuffer_array_create` shall fail and return NULL. ]*/
                    LogError("Failed cloning buffer at index %" PRIu32, i);
                    break;
                }
            }

            if (i < buffer_count)
            {
                uint32_t j;
                LogError("Failed creating const buffer array");

                for (j = 0; j < i; j++)
                {
                    CONSTBUFFER_Destroy(result->buffers[j]);
                }
            }
            else
            {
                result->nBuffers = buffer_count;

                /* Codes_SRS_CONSTBUFFER_ARRAY_01_011: [ On success `constbuffer_array_create` shall return a non-NULL handle. ]*/
                goto all_ok;
            }

            REFCOUNT_TYPE_DESTROY(CONSTBUFFER_ARRAY_HANDLE_DATA, result);
        }
    }

    result = NULL;

all_ok:
    return result;
}

CONSTBUFFER_ARRAY_HANDLE constbuffer_array_create_empty(void)
{
    CONSTBUFFER_ARRAY_HANDLE result;

    /*Codes_SRS_CONSTBUFFER_ARRAY_02_004: [ constbuffer_array_create_empty shall allocate memory for a new CONSTBUFFER_ARRAY_HANDLE. ]*/
    result = REFCOUNT_TYPE_CREATE(CONSTBUFFER_ARRAY_HANDLE_DATA); /*explicit 0*/
    if (result == NULL)
    {
        /*Codes_SRS_CONSTBUFFER_ARRAY_02_001: [ If are any failure is encountered, `constbuffer_array_create_empty` shall fail and return `NULL`. ]*/
        LogError("failure allocating const buffer array");
        /*return as is*/
    }
    else
    {
        /*Codes_SRS_CONSTBUFFER_ARRAY_02_041: [ constbuffer_array_create_empty shall succeed and return a non-NULL value. ]*/
        result->nBuffers = 0;
    }
    return result;
}

CONSTBUFFER_ARRAY_HANDLE constbuffer_array_add_front(CONSTBUFFER_ARRAY_HANDLE constbuffer_array_handle, CONSTBUFFER_HANDLE constbuffer_handle)
{
    CONSTBUFFER_ARRAY_HANDLE result;
    if (
        /*Codes_SRS_CONSTBUFFER_ARRAY_02_006: [ If constbuffer_array_handle is NULL then constbuffer_array_add_front shall fail and return NULL ]*/
        (constbuffer_array_handle == NULL) ||
        /*Codes_SRS_CONSTBUFFER_ARRAY_02_007: [ If constbuffer_handle is NULL then constbuffer_array_add_front shall fail and return NULL ]*/
        (constbuffer_handle == NULL)
        )
    {
        LogError("invalid arguments CONSTBUFFER_ARRAY_HANDLE constbuffer_array_handle=%p, CONSTBUFFER_HANDLE constbuffer_handle=%p", constbuffer_array_handle, constbuffer_handle);
    }
    else
    {
        /*Codes_SRS_CONSTBUFFER_ARRAY_02_042: [ constbuffer_array_add_front shall allocate enough memory to hold all of constbuffer_array_handle existing CONSTBUFFER_HANDLE and constbuffer_handle. ]*/
        result = REFCOUNT_TYPE_CREATE_WITH_EXTRA_SIZE(CONSTBUFFER_ARRAY_HANDLE_DATA, (constbuffer_array_handle->nBuffers + 1) * sizeof(CONSTBUFFER_HANDLE));
        if (result == NULL)
        {
            /*Codes_SRS_CONSTBUFFER_ARRAY_02_011: [ If there any failures constbuffer_array_add_front shall fail and return NULL. ]*/
            LogError("failure in malloc");
            /*return as is*/
        }
        else
        {
            /*Codes_SRS_CONSTBUFFER_ARRAY_02_043: [ constbuffer_array_add_front shall copy constbuffer_handle and all of constbuffer_array_handle existing CONSTBUFFER_HANDLE. ]*/
            /*Codes_SRS_CONSTBUFFER_ARRAY_02_044: [ constbuffer_array_add_front shall inc_ref all the CONSTBUFFER_HANDLE it had copied. ]*/
            result->nBuffers = constbuffer_array_handle->nBuffers + 1;
            if ((result->buffers[0] = CONSTBUFFER_Clone(constbuffer_handle)) == NULL)
            {
                /*Codes_SRS_CONSTBUFFER_ARRAY_02_011: [ If there any failures constbuffer_array_add_front shall fail and return NULL. ]*/
                LogError("failure in CONSTBUFFER_Clone");
            }
            else
            {
                uint32_t i;
                for (i = 1; i < result->nBuffers; i++)
                {
                    result->buffers[i] = CONSTBUFFER_Clone(constbuffer_array_handle->buffers[i - 1]);
                    if (result->buffers[i] == NULL)
                    {
                        /*Codes_SRS_CONSTBUFFER_ARRAY_02_011: [ If there any failures constbuffer_array_add_front shall fail and return NULL. ]*/
                        LogError("failure in CONSTBUFFER_Clone");
                        break;
                    }
                }

                if (i != result->nBuffers)
                {
                    uint32_t j;
                    for (j = 1; j < i; j++)
                    {
                        CONSTBUFFER_Destroy(result->buffers[j]);
                    }
                }
                else
                {
                    /*Codes_SRS_CONSTBUFFER_ARRAY_02_010: [ constbuffer_array_add_front shall succeed and return a non-NULL value. ]*/
                    goto allOk;
                }
                CONSTBUFFER_Destroy(result->buffers[0]);
            }

            REFCOUNT_TYPE_DESTROY(CONSTBUFFER_ARRAY_HANDLE_DATA, result);
        }
    }
    /*Codes_SRS_CONSTBUFFER_ARRAY_02_011: [ If there any failures constbuffer_array_add_front shall fail and return NULL. ]*/
    result = NULL;
allOk:;
    return result;
}

CONSTBUFFER_ARRAY_HANDLE constbuffer_array_remove_front(CONSTBUFFER_ARRAY_HANDLE constbuffer_array_handle, CONSTBUFFER_HANDLE* constbuffer_handle)
{
    CONSTBUFFER_ARRAY_HANDLE result;
    if (
        /*Codes_SRS_CONSTBUFFER_ARRAY_02_012: [ If constbuffer_array_handle is NULL then constbuffer_array_remove_front shall fail and return NULL. ]*/
        (constbuffer_array_handle == NULL) || 
        /*Codes_SRS_CONSTBUFFER_ARRAY_02_045: [ If constbuffer_handle is NULL then constbuffer_array_remove_front shall fail and return NULL. ]*/
        (constbuffer_handle == NULL)
        )
    {
        /*Codes_SRS_CONSTBUFFER_ARRAY_02_036: [ If there are any failures then constbuffer_array_remove_front shall fail and return NULL. ]*/
        LogError("invalid arguments CONSTBUFFER_ARRAY_HANDLE constbuffer_array_handle=%p, CONSTBUFFER_HANDLE* constbuffer_handle=%p", constbuffer_array_handle, constbuffer_handle);
    }
    else
    {
        /*Codes_SRS_CONSTBUFFER_ARRAY_02_002: [ constbuffer_array_remove_front shall fail when called on a newly constructed CONSTBUFFER_ARRAY_HANDLE. ]*/
        /*Codes_SRS_CONSTBUFFER_ARRAY_02_013: [ If there is no front CONSTBUFFER_HANDLE then constbuffer_array_remove_front shall fail and return NULL. ]*/
        if (constbuffer_array_handle->nBuffers == 0)
        {
            /*Codes_SRS_CONSTBUFFER_ARRAY_02_036: [ If there are any failures then constbuffer_array_remove_front shall fail and return NULL. ]*/
            LogError("cannot remove from that which does not have");
        }
        else
        {
            /*Codes_SRS_CONSTBUFFER_ARRAY_02_046: [ constbuffer_array_remove_front shall allocate memory to hold all of constbuffer_array_handle CONSTBUFFER_HANDLEs except the front one. ]*/
            result = REFCOUNT_TYPE_CREATE_WITH_EXTRA_SIZE(CONSTBUFFER_ARRAY_HANDLE_DATA, (constbuffer_array_handle->nBuffers - 1) * sizeof(CONSTBUFFER_HANDLE));
            if (result == NULL)
            {
                /*Codes_SRS_CONSTBUFFER_ARRAY_02_036: [ If there are any failures then constbuffer_array_remove_front shall fail and return NULL. ]*/
                LogError("failure in malloc");
                /*return as is*/
            }
            else
            {
                /* Codes_SRS_CONSTBUFFER_ARRAY_01_001: [ `constbuffer_array_remove_front` shall inc_ref the removed buffer. ]*/
                CONSTBUFFER_HANDLE clonedFrontBuffer = CONSTBUFFER_Clone(constbuffer_array_handle->buffers[0]);
                if (clonedFrontBuffer == NULL)
                {
                    /*Codes_SRS_CONSTBUFFER_ARRAY_02_036: [ If there are any failures then constbuffer_array_remove_front shall fail and return NULL. ]*/
                    LogError("failure in CONSTBUFFER_Clone");
                }
                else
                {
                    uint32_t i;
                    result->nBuffers = constbuffer_array_handle->nBuffers - 1;

                    /*Codes_SRS_CONSTBUFFER_ARRAY_02_047: [ constbuffer_array_remove_front shall copy all of constbuffer_array_handle CONSTBUFFER_HANDLEs except the front one. ]*/
                    /*Codes_SRS_CONSTBUFFER_ARRAY_02_048: [ constbuffer_array_remove_front shall inc_ref all the copied CONSTBUFFER_HANDLEs. ]*/
                    for (i = 1; i < constbuffer_array_handle->nBuffers; i++)
                    {
                        if ((result->buffers[i - 1] = CONSTBUFFER_Clone(constbuffer_array_handle->buffers[i])) == NULL)
                        {
                            /*Codes_SRS_CONSTBUFFER_ARRAY_02_036: [ If there are any failures then constbuffer_array_remove_front shall fail and return NULL. ]*/
                            LogError("failure in CONSTBUFFER_Clone");
                            break;
                        }
                    }

                    if (i != constbuffer_array_handle->nBuffers)
                    {
                        uint32_t j;

                        for (j = 1; j < i; j++)
                        {
                            CONSTBUFFER_Destroy(result->buffers[j - 1]);
                        }
                    }
                    else
                    {
                        /*Codes_SRS_CONSTBUFFER_ARRAY_02_049: [ constbuffer_array_remove_front shall succeed, write in constbuffer_handle the front handle and return a non-NULL value. ]*/
                        *constbuffer_handle = clonedFrontBuffer;
                        goto allOk;
                    }

                    CONSTBUFFER_Destroy(clonedFrontBuffer);
                }

                REFCOUNT_TYPE_DESTROY(CONSTBUFFER_ARRAY_HANDLE_DATA, result);
            }
        }
    }
    /*Codes_SRS_CONSTBUFFER_ARRAY_02_036: [ If there are any failures then constbuffer_array_remove_front shall fail and return NULL. ]*/
    result = NULL;
allOk:;
    return result;
}

int constbuffer_array_get_buffer_count(CONSTBUFFER_ARRAY_HANDLE constbuffer_array_handle, uint32_t* buffer_count)
{
    int result;

    if (
        /* Codes_SRS_CONSTBUFFER_ARRAY_01_003: [ If `constbuffer_array_handle` is NULL, `constbuffer_array_get_buffer_count` shall fail and return a non-zero value. ]*/
        (constbuffer_array_handle == NULL) ||
        /* Codes_SRS_CONSTBUFFER_ARRAY_01_004: [ If `buffer_count` is NULL, `constbuffer_array_get_buffer_count` shall fail and return a non-zero value. ]*/
        (buffer_count == NULL)
        )
    {
        LogError("Invalid arguments: CONSTBUFFER_ARRAY_HANDLE constbuffer_array_handle=%p, uint32_t* buffer_count=%p",
            constbuffer_array_handle, buffer_count);
        result = __FAILURE__;
    }
    else
    {
        /* Codes_SRS_CONSTBUFFER_ARRAY_01_002: [ On success, `constbuffer_array_get_buffer_count` shall return 0 and write the buffer count in `buffer_count`. ]*/
        *buffer_count = constbuffer_array_handle->nBuffers;

        result = 0;
    }

    return result;
}

CONSTBUFFER_HANDLE constbuffer_array_get_buffer(CONSTBUFFER_ARRAY_HANDLE constbuffer_array_handle, uint32_t buffer_index)
{
    CONSTBUFFER_HANDLE result;

    if (
        /* Codes_SRS_CONSTBUFFER_ARRAY_01_007: [ If `constbuffer_array_handle` is NULL, `constbuffer_array_get_buffer` shall fail and return NULL. ]*/
        (constbuffer_array_handle == NULL) ||
        (buffer_index >= constbuffer_array_handle->nBuffers)
        )
    {
        LogError("Invalid arguments: CONSTBUFFER_ARRAY_HANDLE constbuffer_array_handle=%p, uint32_t buffer_index=%" PRIu32,
            constbuffer_array_handle, buffer_index);
    }
    else
    {
        /* Codes_SRS_CONSTBUFFER_ARRAY_01_006: [ The returned handle shall have its reference count incremented. ]*/
        result = CONSTBUFFER_Clone(constbuffer_array_handle->buffers[buffer_index]);
        if (result == NULL)
        {
            /* Codes_SRS_CONSTBUFFER_ARRAY_01_015: [ If any error occurs, `constbuffer_array_get_buffer` shall fail and return NULL. ]*/
            LogError("Cloning CONST buffer failed");
        }
        else
        {
            /* Codes_SRS_CONSTBUFFER_ARRAY_01_005: [ On success, `constbuffer_array_get_buffer` shall return a non-NULL handle to the `buffer_index`-th const buffer in the array. ]*/
            goto all_ok;
        }
    }

    result = NULL;

all_ok:
    return result;
}

void constbuffer_array_inc_ref(CONSTBUFFER_ARRAY_HANDLE constbuffer_array_handle)
{
    if (constbuffer_array_handle == NULL)
    {
        /* Codes_SRS_CONSTBUFFER_ARRAY_01_017: [ If `constbuffer_array_handle` is `NULL` then `constbuffer_array_inc_ref` shall return. ]*/
        LogError("invalid argument CONSTBUFFER_ARRAY_HANDLE constbuffer_array_handle=%p", constbuffer_array_handle);
    }
    else
    {
        /* Codes_SRS_CONSTBUFFER_ARRAY_01_018: [ Otherwise `constbuffer_array_inc_ref` shall increment the reference count for `constbuffer_array_handle`. ]*/
        INC_REF(CONSTBUFFER_ARRAY_HANDLE_DATA, constbuffer_array_handle);
    }
}

void constbuffer_array_dec_ref(CONSTBUFFER_ARRAY_HANDLE constbuffer_array_handle)
{
    if (constbuffer_array_handle == NULL)
    {
        /*Codes_SRS_CONSTBUFFER_ARRAY_02_039: [ If constbuffer_array_handle is NULL then constbuffer_array_dec_ref shall return. ]*/
        LogError("invalid argument CONSTBUFFER_ARRAY_HANDLE constbuffer_array_handle=%p", constbuffer_array_handle);
    }
    else
    {
        /* Codes_SRS_CONSTBUFFER_ARRAY_01_016: [ Otherwise `constbuffer_array_dec_ref` shall decrement the reference count for `constbuffer_array_handle`. ]*/
        if (DEC_REF(CONSTBUFFER_ARRAY_HANDLE_DATA, constbuffer_array_handle) == DEC_RETURN_ZERO)
        {
            uint32_t i;

            /*Codes_SRS_CONSTBUFFER_ARRAY_02_038: [ If the reference count reaches 0, `constbuffer_array_dec_ref` shall free all used resources. ]*/
            for (i = 0; i < constbuffer_array_handle->nBuffers; i++)
            {
                CONSTBUFFER_Destroy(constbuffer_array_handle->buffers[i]);
            }

            REFCOUNT_TYPE_DESTROY(CONSTBUFFER_ARRAY_HANDLE_DATA, constbuffer_array_handle);
        }
    }
}
