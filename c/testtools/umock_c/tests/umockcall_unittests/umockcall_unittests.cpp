// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdint.h>
#include <stdlib.h>
#include "testrunnerswitcher.h"
#include "umockcall.h"

/* TODO: test malloc failures */
/* TODO: serialize tests */

/* Not tested requirements */
/* SRS_UMOCKCALL_01_002: [ If allocating memory for the umock call instance fails, umockcall_create shall return NULL. ] */

typedef struct test_mock_call_data_free_CALL_TAG
{
    void* umockcall_data;
} test_mock_call_data_free_CALL;

static test_mock_call_data_free_CALL* test_mock_call_data_free_calls;
static size_t test_mock_call_data_free_call_count;
static int test_mock_call_data_are_equal_expected_result;

void test_mock_call_data_free(void* umockcall_data)
{
    test_mock_call_data_free_CALL* new_calls = (test_mock_call_data_free_CALL*)realloc(test_mock_call_data_free_calls, sizeof(test_mock_call_data_free_CALL) * (test_mock_call_data_free_call_count + 1));
    if (new_calls != NULL)
    {
        test_mock_call_data_free_calls = new_calls;
        test_mock_call_data_free_calls[test_mock_call_data_free_call_count].umockcall_data = umockcall_data;
        test_mock_call_data_free_call_count ++;
    }
}

char* test_mock_call_data_stringify(void* umockcall_data)
{
    (void)umockcall_data;
    return NULL;
}

int test_mock_call_data_are_equal(void* left, void* right)
{
    (void)left, right;
    return test_mock_call_data_are_equal_expected_result;
}

BEGIN_TEST_SUITE(umockcall_unittests)

TEST_SUITE_INITIALIZE(suite_init)
{
}

TEST_SUITE_CLEANUP(suite_cleanup)
{
}

TEST_FUNCTION_INITIALIZE(test_function_init)
{
    test_mock_call_data_are_equal_expected_result = 1;
}

TEST_FUNCTION_CLEANUP(test_function_cleanup)
{
    free(test_mock_call_data_free_calls);
    test_mock_call_data_free_calls = NULL;
    test_mock_call_data_free_call_count = 0;
}

/* umockcall_create */

/* Tests_SRS_UMOCKCALL_01_001: [ umockcall_create shall create a new instance of a umock call and on success it shall return a non-NULL handle to it. ] */
TEST_FUNCTION(umockcall_create_with_proper_args_succeeds)
{
    // arrange

    // act
    UMOCKCALL_HANDLE result = umockcall_create("test_function", (void*)0x4242, test_mock_call_data_free, test_mock_call_data_stringify, test_mock_call_data_are_equal);

    // assert
    ASSERT_IS_NOT_NULL(result);

    // cleanup
    umockcall_destroy(result);
}

/* Tests_SRS_UMOCKCALL_01_003: [ If any of the arguments are NULL, umockcall_create shall fail and return NULL. ] */
TEST_FUNCTION(umockcall_create_with_NULL_function_name_fails)
{
    // arrange

    // act
    UMOCKCALL_HANDLE result = umockcall_create(NULL, (void*)0x4242, test_mock_call_data_free, test_mock_call_data_stringify, test_mock_call_data_are_equal);

    // assert
    ASSERT_IS_NULL(result);
}

/* Tests_SRS_UMOCKCALL_01_003: [ If any of the arguments are NULL, umockcall_create shall fail and return NULL. ] */
TEST_FUNCTION(umockcall_create_with_NULL_call_data_fails)
{
    // arrange

    // act
    UMOCKCALL_HANDLE result = umockcall_create("test_function", NULL, test_mock_call_data_free, test_mock_call_data_stringify, test_mock_call_data_are_equal);

    // assert
    ASSERT_IS_NULL(result);
}

/* Tests_SRS_UMOCKCALL_01_003: [ If any of the arguments are NULL, umockcall_create shall fail and return NULL. ] */
TEST_FUNCTION(umockcall_create_with_NULL_call_data_free_function_fails)
{
    // arrange

    // act
    UMOCKCALL_HANDLE result = umockcall_create("test_function", (void*)0x4242, NULL, test_mock_call_data_stringify, test_mock_call_data_are_equal);

    // assert
    ASSERT_IS_NULL(result);
}

/* Tests_SRS_UMOCKCALL_01_003: [ If any of the arguments are NULL, umockcall_create shall fail and return NULL. ] */
TEST_FUNCTION(umockcall_create_with_NULL_call_data_stringify_function_fails)
{
    // arrange

    // act
    UMOCKCALL_HANDLE result = umockcall_create("test_function", (void*)0x4242, test_mock_call_data_free, NULL, test_mock_call_data_are_equal);

    // assert
    ASSERT_IS_NULL(result);
}

/* Tests_SRS_UMOCKCALL_01_003: [ If any of the arguments are NULL, umockcall_create shall fail and return NULL. ] */
TEST_FUNCTION(umockcall_create_with_NULL_call_data_are_equal_function_fails)
{
    // arrange

    // act
    UMOCKCALL_HANDLE result = umockcall_create("test_function", (void*)0x4242, test_mock_call_data_free, test_mock_call_data_stringify, NULL);

    // assert
    ASSERT_IS_NULL(result);
}

/* umockcall_destroy */

/* Tests_SRS_UMOCKCALL_01_004: [ umockcall_destroy shall free a previously allocated umock call instance. ] */
TEST_FUNCTION(umockcall_destroy_frees_call_data)
{
    // arrange
    UMOCKCALL_HANDLE call = umockcall_create("test_function", (void*)0x4242, test_mock_call_data_free, test_mock_call_data_stringify, test_mock_call_data_are_equal);

    // act
    umockcall_destroy(call);

    // assert
    ASSERT_ARE_EQUAL(size_t, 1, test_mock_call_data_free_call_count);
    ASSERT_ARE_EQUAL(void_ptr, (void*)0x4242, test_mock_call_data_free_calls[0].umockcall_data);
}

/* Tests_SRS_UMOCKCALL_01_005: [ If the umockcall argument is NULL then umockcall_destroy shall do nothing. ]*/
TEST_FUNCTION(umockcall_destroy_with_NULL_argument_does_nothing)
{
    // arrange

    // act
    umockcall_destroy(NULL);

    // assert
    ASSERT_ARE_EQUAL(size_t, 0, test_mock_call_data_free_call_count);
}

/* umockcall_are_equal */

/* Tests_SRS_UMOCKCALL_01_006: [ umockcall_are_equal shall compare the two mock calls and return whether they are equal or not. ] */
/* Tests_SRS_UMOCKCALL_01_007: [ If the 2 mock calls are equal umockcall_are_equal shall return 1. ]*/
TEST_FUNCTION(umockcall_are_equal_with_2_equal_calls_returns_1)
{
    // arrange
    UMOCKCALL_HANDLE call1 = umockcall_create("test_function", (void*)0x4242, test_mock_call_data_free, test_mock_call_data_stringify, test_mock_call_data_are_equal);
    UMOCKCALL_HANDLE call2 = umockcall_create("test_function", (void*)0x4242, test_mock_call_data_free, test_mock_call_data_stringify, test_mock_call_data_are_equal);

    // act
    int result = umockcall_are_equal(call1, call2);

    // assert
    ASSERT_ARE_EQUAL(int, 1, result);

    // cleanup
    umockcall_destroy(call1);
    umockcall_destroy(call2);
}

/* Tests_SRS_UMOCKCALL_01_006: [ umockcall_are_equal shall compare the two mock calls and return whether they are equal or not. ] */
TEST_FUNCTION(umockcall_are_equal_with_the_same_call_for_both_args_returns_1)
{
    // arrange
    UMOCKCALL_HANDLE call = umockcall_create("test_function", (void*)0x4242, test_mock_call_data_free, test_mock_call_data_stringify, test_mock_call_data_are_equal);

    // act
    int result = umockcall_are_equal(call, call);

    // assert
    ASSERT_ARE_EQUAL(int, 1, result);

    // cleanup
    umockcall_destroy(call);
}

END_TEST_SUITE(umockcall_unittests)
