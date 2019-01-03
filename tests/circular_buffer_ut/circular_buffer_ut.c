// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifdef __cplusplus
#include <cstdlib>
#include <cstddef>
#else
#include <stdlib.h>
#include <stddef.h>
#endif

static size_t currentmalloc_call = 0;
static size_t whenShallmalloc_fail = 0;

static size_t currentrealloc_call = 0;
static size_t whenShallrealloc_fail = 0;

void* my_gballoc_malloc(size_t size)
{
    void* result;
    currentmalloc_call++;
    if (whenShallmalloc_fail > 0)
    {
        if (currentmalloc_call == whenShallmalloc_fail)
        {
            result = NULL;
        }
        else
        {
            result = malloc(size);
        }
    }
    else
    {
        result = malloc(size);
    }
    return result;
}

void* my_gballoc_realloc(void* ptr, size_t size)
{
    void* result;
    currentrealloc_call++;
    if (whenShallrealloc_fail > 0)
    {
        if (currentrealloc_call == whenShallrealloc_fail)
        {
            result = NULL;
        }
        else
        {
            result = realloc(ptr, size);
        }
    }
    else
    {
        result = realloc(ptr, size);
    }

    return result;
}

void my_gballoc_free(void* ptr)
{
    free(ptr);
}


#include "umock_c.h"
#include "testrunnerswitcher.h"

#define ENABLE_MOCKS
#include "azure_c_shared_utility/gballoc.h"
#undef ENABLE_MOCKS

#include "azure_c_shared_utility/circular_buffer.h"

static TEST_MUTEX_HANDLE g_testByTest;

DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    ASSERT_FAIL("umock_c reported error :%s", ENUM_TO_STRING(UMOCK_C_ERROR_CODE, error_code));
}

BEGIN_TEST_SUITE(Circular_Buffer_UnitTests)

    TEST_SUITE_INITIALIZE(setsBufferTempSize)
    {
        g_testByTest = TEST_MUTEX_CREATE();
        ASSERT_IS_NOT_NULL(g_testByTest);

        umock_c_init(on_umock_c_error);

        REGISTER_GLOBAL_MOCK_HOOK(malloc, my_gballoc_malloc);
        REGISTER_GLOBAL_MOCK_HOOK(realloc, my_gballoc_realloc);
        REGISTER_GLOBAL_MOCK_HOOK(free, my_gballoc_free);
    }

    TEST_SUITE_CLEANUP(TestClassCleanup)
    {
        umock_c_deinit();

        TEST_MUTEX_DESTROY(g_testByTest);
    }

    TEST_FUNCTION_INITIALIZE(f)
    {
        if (TEST_MUTEX_ACQUIRE(g_testByTest))
        {
            ASSERT_FAIL("our mutex is ABANDONED. Failure in test framework");
        }

        umock_c_reset_all_calls();

        currentmalloc_call = 0;
        whenShallmalloc_fail = 0;

        currentrealloc_call = 0;
        whenShallrealloc_fail = 0;
    }

    TEST_FUNCTION_CLEANUP(cleans)
    {
        TEST_MUTEX_RELEASE(g_testByTest);
    }

    TEST_FUNCTION(circular_buffer_create_success)
    {
        // arrange
        CIRCULAR_BUFFER_HANDLE cbh;

        umock_c_reset_all_calls();
        STRICT_EXPECTED_CALL(malloc(IGNORED_NUM_ARG));
        STRICT_EXPECTED_CALL(malloc(10));

        // act
        cbh = circular_buffer_create(10);

        // assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_IS_NOT_NULL(cbh);

        // cleanup
        circular_buffer_destroy(cbh);
    }

    TEST_FUNCTION(circular_buffer_destroy_success)
    {
        // arrange
        CIRCULAR_BUFFER_HANDLE cbh;

        umock_c_reset_all_calls();
        STRICT_EXPECTED_CALL(malloc(IGNORED_NUM_ARG));
        STRICT_EXPECTED_CALL(malloc(10));
        cbh = circular_buffer_create(10);

        umock_c_reset_all_calls();
        STRICT_EXPECTED_CALL(free(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(free(IGNORED_PTR_ARG));

        // act
        circular_buffer_destroy(cbh);

        // assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        // cleanup
    }

    TEST_FUNCTION(circular_buffer_first_write_no_expand_success)
    {
        // arrange
        CIRCULAR_BUFFER_HANDLE cbh;
        const unsigned char data[] = "abcdefghi";

        umock_c_reset_all_calls();
        STRICT_EXPECTED_CALL(malloc(IGNORED_NUM_ARG));
        STRICT_EXPECTED_CALL(malloc(10));
        cbh = circular_buffer_create(10);

        umock_c_reset_all_calls();
        
        // act
        int result = circular_buffer_write(cbh, data, 9);

        // assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_ARE_EQUAL(int, 0, result);

        // cleanup
        circular_buffer_destroy(cbh);
    }

    TEST_FUNCTION(circular_buffer_first_write_with_expand_success)
    {
        // arrange
        CIRCULAR_BUFFER_HANDLE cbh;
        const unsigned char data[] = "abcdefghijkl";

        umock_c_reset_all_calls();
        STRICT_EXPECTED_CALL(malloc(IGNORED_NUM_ARG));
        STRICT_EXPECTED_CALL(malloc(10));
        cbh = circular_buffer_create(10);

        umock_c_reset_all_calls();
        STRICT_EXPECTED_CALL(realloc(IGNORED_PTR_ARG, 12));

        // act
        int result = circular_buffer_write(cbh, data, 12);

        // assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_ARE_EQUAL(int, 0, result);

        // cleanup
        circular_buffer_destroy(cbh);
    }

    TEST_FUNCTION(circular_buffer_read_no_overlap_success)
    {
        // arrange
        CIRCULAR_BUFFER_HANDLE cbh;
        const unsigned char data[] = "abcdefghi";
        unsigned char read_data[10];

        umock_c_reset_all_calls();
        STRICT_EXPECTED_CALL(malloc(IGNORED_NUM_ARG));
        STRICT_EXPECTED_CALL(malloc(10));
        cbh = circular_buffer_create(10);
        (void)circular_buffer_write(cbh, data, 9);

        umock_c_reset_all_calls();

        // act
        size_t nread = circular_buffer_read(cbh, read_data, 4);

        // assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_ARE_EQUAL(int, 4, nread);
        ASSERT_ARE_EQUAL(int, 0, memcmp(data, read_data, nread));

        // cleanup
        circular_buffer_destroy(cbh);
    }

    TEST_FUNCTION(circular_buffer_try_read_more_bytes_than_written_success)
    {
        // arrange
        CIRCULAR_BUFFER_HANDLE cbh;
        const unsigned char data[] = "abcdefghi";
        unsigned char read_data[10];

        umock_c_reset_all_calls();
        STRICT_EXPECTED_CALL(malloc(IGNORED_NUM_ARG));
        STRICT_EXPECTED_CALL(malloc(10));
        cbh = circular_buffer_create(10);
        (void)circular_buffer_write(cbh, data, 5);

        umock_c_reset_all_calls();

        // act
        size_t nread = circular_buffer_read(cbh, read_data, 9);

        // assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_ARE_EQUAL(int, 5, nread);
        ASSERT_ARE_EQUAL(int, 0, memcmp(data, read_data, nread));

        // cleanup
        circular_buffer_destroy(cbh);
    }

    TEST_FUNCTION(circular_buffer_write_with_overlap_success)
    {
        // arrange
        CIRCULAR_BUFFER_HANDLE cbh;
        const unsigned char data[] = "abcdefghi";
        unsigned char read_data1[10];

        umock_c_reset_all_calls();
        STRICT_EXPECTED_CALL(malloc(IGNORED_NUM_ARG));
        STRICT_EXPECTED_CALL(malloc(10));
        cbh = circular_buffer_create(10);
        (void)circular_buffer_write(cbh, data, 9);
        size_t nread1 = circular_buffer_read(cbh, read_data1, 4);

        umock_c_reset_all_calls();

        // act
        int result = circular_buffer_write(cbh, data, 5);

        // assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_ARE_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(int, 0, memcmp(data, read_data1, nread1));

        // cleanup
        circular_buffer_destroy(cbh);
    }

    TEST_FUNCTION(circular_buffer_read_with_overlap_success)
    {
        // arrange
        CIRCULAR_BUFFER_HANDLE cbh;
        const unsigned char data1[] = "abcdefghi";
        const unsigned char data2[] = "efghiabcde";
        unsigned char read_data1[10];
        unsigned char read_data2[10];

        umock_c_reset_all_calls();
        STRICT_EXPECTED_CALL(malloc(IGNORED_NUM_ARG));
        STRICT_EXPECTED_CALL(malloc(10));
        cbh = circular_buffer_create(10);
        (void)circular_buffer_write(cbh, data1, 9);
        size_t nread1 = circular_buffer_read(cbh, read_data1, 4);
        (void)circular_buffer_write(cbh, data1, 5);

        umock_c_reset_all_calls();

        // act
        size_t nread2 = circular_buffer_read(cbh, read_data2, 10);

        // assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_ARE_EQUAL(int, 10, nread2);
        ASSERT_ARE_EQUAL(int, 0, memcmp(data1, read_data1, nread1));
        ASSERT_ARE_EQUAL(int, 0, memcmp(data2, read_data2, nread2));

        // cleanup
        circular_buffer_destroy(cbh);
    }

    TEST_FUNCTION(circular_buffer_write_circular_with_expand_success)
    {
        // arrange
        CIRCULAR_BUFFER_HANDLE cbh;
        const unsigned char data1[] = "abcdefghijkl";
        unsigned char read_data1[10];

        umock_c_reset_all_calls();
        STRICT_EXPECTED_CALL(malloc(IGNORED_NUM_ARG));
        STRICT_EXPECTED_CALL(malloc(10));
        cbh = circular_buffer_create(10);

        (void)circular_buffer_write(cbh, data1, 9);
        (void)circular_buffer_read(cbh, read_data1, 4);

        umock_c_reset_all_calls();
        STRICT_EXPECTED_CALL(realloc(IGNORED_PTR_ARG, 17));

        // act
        int result = circular_buffer_write(cbh, data1, 12);

        // assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_ARE_EQUAL(int, 0, result);

        // cleanup
        circular_buffer_destroy(cbh);
    }

    TEST_FUNCTION(circular_buffer_multiple_write_read_success)
    {
        // arrange
        CIRCULAR_BUFFER_HANDLE cbh;
        const unsigned char write_data[] = "abcdefghijklabcdefghijklabcdefghijklabcdefghijklabcdefghijklabcdefghijklabcdefghijklabcdefghijkl";
        unsigned char read_data[15];
        size_t size;
        size_t nread;
        int write_result;
        int get_size_result;

        umock_c_reset_all_calls();
        STRICT_EXPECTED_CALL(malloc(IGNORED_NUM_ARG));
        STRICT_EXPECTED_CALL(malloc(10));
        cbh = circular_buffer_create(10);

        umock_c_reset_all_calls();
        
        write_result = circular_buffer_write(cbh, write_data, 5);
        nread = circular_buffer_read(cbh, read_data, 3);
        get_size_result = circular_buffer_get_data_size(cbh, &size);

        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_ARE_EQUAL(int, 0, write_result);
        ASSERT_ARE_EQUAL(int, 0, get_size_result);
        ASSERT_ARE_EQUAL(int, 3, nread);
        ASSERT_ARE_EQUAL(int, 2, size);
        ASSERT_ARE_EQUAL(int, 0, memcmp(read_data, write_data, nread));

        umock_c_reset_all_calls();
        STRICT_EXPECTED_CALL(realloc(IGNORED_PTR_ARG, 12));

        write_result = circular_buffer_write(cbh, write_data + 5, 10);
        nread = circular_buffer_read(cbh, read_data, 8);
        get_size_result = circular_buffer_get_data_size(cbh, &size);

        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_ARE_EQUAL(int, 0, write_result);
        ASSERT_ARE_EQUAL(int, 0, get_size_result);
        ASSERT_ARE_EQUAL(int, 8, nread);
        ASSERT_ARE_EQUAL(int, 4, size);
        ASSERT_ARE_EQUAL(int, 0, memcmp(read_data, write_data + 3, nread));

        umock_c_reset_all_calls();
        STRICT_EXPECTED_CALL(realloc(IGNORED_PTR_ARG, 13));

        write_result = circular_buffer_write(cbh, write_data + 15, 9);
        nread = circular_buffer_read(cbh, read_data, 15);
        get_size_result = circular_buffer_get_data_size(cbh, &size);

        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_ARE_EQUAL(int, 0, write_result);
        ASSERT_ARE_EQUAL(int, 0, get_size_result);
        ASSERT_ARE_EQUAL(int, 13, nread);
        ASSERT_ARE_EQUAL(int, 0, size);
        ASSERT_ARE_EQUAL(int, 0, memcmp(read_data, write_data + 11, 1));
        ASSERT_ARE_EQUAL(int, 0, memcmp(read_data + 1, write_data, nread - 1));

        // cleanup
        circular_buffer_destroy(cbh);
    }

END_TEST_SUITE(Circular_Buffer_UnitTests)
