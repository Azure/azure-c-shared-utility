// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdint.h>
#include <stdlib.h>
#include "testrunnerswitcher.h"
#include "umockcall.h"

extern "C"
{
    static size_t malloc_call_count;
    static size_t calloc_call_count;
    static size_t realloc_call_count;
    static size_t free_call_count;

    static size_t when_shall_malloc_fail;
    static size_t when_shall_calloc_fail;
    static size_t when_shall_realloc_fail;

    void* mock_malloc(size_t size)
    {
        void* result;
        malloc_call_count++;
        if (malloc_call_count == when_shall_malloc_fail)
        {
            result = NULL;
        }
        else
        {
            result = malloc(size);
        }
        return result;
    }

    void* mock_calloc(size_t nmemb, size_t size)
    {
        void* result;
        calloc_call_count++;
        if (calloc_call_count == when_shall_calloc_fail)
        {
            result = NULL;
        }
        else
        {
            result = calloc(nmemb, size);
        }
        return result;
    }

    void* mock_realloc(void* ptr, size_t size)
    {
        void* result;
        realloc_call_count++;
        if (realloc_call_count == when_shall_realloc_fail)
        {
            result = NULL;
        }
        else
        {
            result = realloc(ptr, size);
        }
        return result;
    }

    void mock_free(void* ptr)
    {
        free_call_count++;
        free(ptr);
    }

    void reset_malloc_calls(void)
    {
        malloc_call_count = 0;
        when_shall_malloc_fail = 0;
        calloc_call_count = 0;
        when_shall_calloc_fail = 0;
        realloc_call_count = 0;
        when_shall_realloc_fail = 0;
        free_call_count = 0;
    }
}

TEST_MUTEX_HANDLE test_mutex;

BEGIN_TEST_SUITE(umockcallrecorder_unittests)

TEST_SUITE_INITIALIZE(suite_init)
{
    test_mutex = TEST_MUTEX_CREATE();
    ASSERT_IS_NOT_NULL(test_mutex);
}

TEST_SUITE_CLEANUP(suite_cleanup)
{
    TEST_MUTEX_DESTROY(test_mutex);
}

TEST_FUNCTION_INITIALIZE(test_function_init)
{
    ASSERT_ARE_EQUAL(int, 0, TEST_MUTEX_ACQUIRE(test_mutex));

    reset_malloc_calls();
}

TEST_FUNCTION_CLEANUP(test_function_cleanup)
{
    TEST_MUTEX_RELEASE(test_mutex);
}

/* umockcallrecorder_create */

END_TEST_SUITE(umockcallrecorder_unittests)
