// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdint.h>
#include <stdlib.h>
#include "testrunnerswitcher.h"

/* TODO:
- Switch to .c
- serialize tests
*/

#define ENABLE_MOCKS

#include "umock_c.h"

typedef struct test_on_umock_c_error_CALL_TAG
{
    UMOCK_C_ERROR_CODE error_code;
} test_on_umock_c_error_CALL;

static test_on_umock_c_error_CALL* test_on_umock_c_error_calls;
static size_t test_on_umock_c_error_call_count;

static void test_on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    test_on_umock_c_error_CALL* new_calls = (test_on_umock_c_error_CALL*)realloc(test_on_umock_c_error_calls, sizeof(test_on_umock_c_error_CALL) * (test_on_umock_c_error_call_count + 1));
    if (new_calls != NULL)
    {
        test_on_umock_c_error_calls = new_calls;
        test_on_umock_c_error_calls[test_on_umock_c_error_call_count].error_code = error_code;
        test_on_umock_c_error_call_count++;
    }
}

TEST_MUTEX_HANDLE test_mutex;

BEGIN_TEST_SUITE(umock_c_without_init_unittests)

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
}

TEST_FUNCTION_CLEANUP(test_function_cleanup)
{
    TEST_MUTEX_RELEASE(test_mutex);
}

/* umock_c_deinit */

/* Tests_SRS_UMOCK_C_01_012: [If umock_c was not initialized, umock_c_deinit shall do nothing.] */
TEST_FUNCTION(umock_c_deinit_when_not_initialized_does_nothing)
{
    // arrange

    // act
    umock_c_deinit();

    // assert
    // no explicit assert
}

END_TEST_SUITE(umock_c_without_init_unittests)
