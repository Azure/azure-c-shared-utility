// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <stdbool.h>
#include "azure_c_shared_utility/gballoc.h"
#include "azure_c_shared_utility/optimize_size.h"
#include "azure_c_shared_utility/xlogging.h"
#include "azure_c_shared_utility/circular_buffer.h"

static const size_t DEFAULT_CIRCULAR_BUFFER_CAPACITY = 1;

typedef struct CIRCULAR_BUFFER_TAG
{
    unsigned char* buffer;
    size_t capacity;          // Size of buffer.
    size_t start;             // Where first byte of data is.
    size_t length;            // Length of buffered data.
} CIRCULAR_BUFFER;

static int expand_buffer(CIRCULAR_BUFFER* cb, size_t additional_capacity)
{
    int result;
    size_t new_capacity = cb->capacity + additional_capacity;

    if ((cb->buffer = (unsigned char*)realloc(cb->buffer, new_capacity)) == NULL)
    {
        LogError("Failed to realloc internal data storage");
        result = __FAILURE__;
    }
    else
    {
        if ((cb->start + cb->length) > cb->capacity)
        {
            (void)memcpy(cb->buffer + cb->capacity, cb->buffer + cb->start, cb->capacity - cb->start);
            cb->start = cb->capacity;
        }

        cb->capacity = new_capacity;

        result = 0;
    }

    return result;
}

CIRCULAR_BUFFER_HANDLE circular_buffer_create(size_t capacity)
{
    CIRCULAR_BUFFER* result;

    if ((result = (CIRCULAR_BUFFER*)malloc(sizeof(CIRCULAR_BUFFER))) == NULL)
    {
        LogError("Failed creating circular buffer");
    }
    else
    {
        if (capacity == 0)
        {
            capacity = DEFAULT_CIRCULAR_BUFFER_CAPACITY;
        }

        if ((result->buffer = (unsigned char*)malloc(capacity)) == NULL)
        {
            LogError("Failed allocating circular buffer internal data storage");
            free(result);
            result = NULL;
        }
        else
        {
            result->capacity = capacity;
            result->start = 0;
            result->length = 0;
        }
    }

    return result;
}

void circular_buffer_destroy(CIRCULAR_BUFFER_HANDLE handle)
{
    if (handle == NULL)
    {
        LogError("Invalid argument (handle is NULL)");
    }
    else
    {
        free(handle->buffer);
        free(handle);
    }
}

int circular_buffer_write(CIRCULAR_BUFFER_HANDLE handle, const unsigned char* data, size_t length)
{
    int result;

    if (handle == NULL || data == NULL || length == 0)
    {
        LogError("Invalid argument (handle=%p, data=%p, length=%d)", handle, data, length);
        result = __FAILURE__;
    }
    else
    {
        CIRCULAR_BUFFER* cb = (CIRCULAR_BUFFER*)handle;
        size_t free_size = cb->capacity - cb->length;

        if ((free_size < length) && expand_buffer(cb, length - free_size) != 0)
        {
            LogError("Failed writting data (could not expand buffer)");
            result = __FAILURE__;
        }
        else
        {
            if ((cb->start + cb->length) < cb->capacity)
            {
                size_t free_end_block_size = cb->capacity - cb->start - cb->length;

                if (length <= free_end_block_size)
                {
                    (void)memcpy(cb->buffer + cb->start + cb->length, data, length);
                    cb->length += length;
                }
                else
                {
                    size_t first_write_length = free_end_block_size;
                    size_t second_write_length = length - first_write_length;

                    (void)memcpy(cb->buffer + cb->start + cb->length, data, first_write_length);
                    (void)memcpy(cb->buffer, data + first_write_length, second_write_length);

                    cb->length += (first_write_length + second_write_length);
                }
            }
            else
            {
                (void)memcpy(cb->buffer + (cb->start + cb->length) - cb->capacity, data, length);
                cb->length += length;
            }

            result = 0;
        }
    }

    return result;
}

size_t circular_buffer_read(CIRCULAR_BUFFER_HANDLE handle, unsigned char* data, size_t length)
{
    size_t result;

    if (handle == NULL || data == NULL || length == 0)
    {
        LogError("Invalid argument (handle=%p, data=%p, length=%d)", handle, data, length);
        result = 0;
    }
    else
    {
        CIRCULAR_BUFFER* cb = (CIRCULAR_BUFFER*)handle;

        if (cb->length == 0)
        {
            result = 0; // buffer is empty; no bytes read.
        }
        else if ((cb->start + cb->length) <= cb->capacity)
        {
            if (length < cb->length)
            {
                (void)memcpy(data, cb->buffer + cb->start, length);
                cb->start += length;
                cb->length -= length;
                result = length;
            }
            else
            {
                (void)memcpy(data, cb->buffer + cb->start, cb->length);
                result = cb->length;
                cb->start += cb->length;
                cb->length = 0;

                if (cb->start == cb->capacity)
                {
                    cb->start = 0;
                }
            }
        }
        else
        {
            size_t first_read_length = cb->capacity - cb->start;

            if (length < first_read_length)
            {
                (void)memcpy(data, cb->buffer + cb->start, length);
                cb->start += length;
                cb->length -= length;
                result = length;
            }
            else
            {
                size_t second_read_length = length - first_read_length;
                size_t start_block_length = cb->start + cb->length - cb->capacity;
                
                if (second_read_length > start_block_length)
                {
                    second_read_length = start_block_length;
                }

                (void)memcpy(data, cb->buffer + cb->start, first_read_length);
                (void)memcpy(data + first_read_length, cb->buffer, second_read_length);
                
                result = first_read_length + second_read_length;
                
                cb->start = second_read_length;
                cb->length -= result;
            }
        }
    }

    return result;
}

int circular_buffer_get_data_size(CIRCULAR_BUFFER_HANDLE handle, size_t* size)
{
    int result;

    if (handle == NULL || size == NULL)
    {
        LogError("Invalid argument (handle=%p, size=%p)", handle, size);
        result = __FAILURE__;
    }
    else
    {
        *size = handle->length;
        result = 0;
    }

    return result;
}
