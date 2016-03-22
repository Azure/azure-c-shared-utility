// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdint.h>
#include <stdlib.h>
#include "testrunnerswitcher.h"
#include "umockcallrecorder.h"
#include "umocktypes.h"
#include "umocktypes_c.h"

#define ENABLE_MOCKS

#include "umock_c.h"

static size_t umocktypes_deinit_call_count;
static size_t umockcallrecorder_destroy_call_count;
static size_t umockcallrecorder_reset_all_calls_call_count;

UMOCKCALLRECORDER_HANDLE umockcallrecorder_create(void)
{
    return NULL;
}

void umockcallrecorder_destroy(UMOCKCALLRECORDER_HANDLE umock_call_recorder)
{
    (void)umock_call_recorder;
    umockcallrecorder_destroy_call_count++;
}

int umockcallrecorder_reset_all_calls(UMOCKCALLRECORDER_HANDLE umock_call_recorder)
{
    (void)umock_call_recorder;
    umockcallrecorder_reset_all_calls_call_count++;
    return 0;
}

int umockcallrecorder_add_expected_call(UMOCKCALLRECORDER_HANDLE umock_call_recorder, UMOCKCALL_HANDLE mock_call)
{
    (void)umock_call_recorder, mock_call;
    return 0;
}

int umockcallrecorder_add_actual_call(UMOCKCALLRECORDER_HANDLE umock_call_recorder, UMOCKCALL_HANDLE mock_call, UMOCKCALL_HANDLE* matched_call)
{
    (void)umock_call_recorder, mock_call;
    return 0;
}

const char* umockcallrecorder_get_actual_calls(UMOCKCALLRECORDER_HANDLE umock_call_recorder)
{
    (void)umock_call_recorder;
    return NULL;
}

const char* umockcallrecorder_get_expected_calls(UMOCKCALLRECORDER_HANDLE umock_call_recorder)
{
    (void)umock_call_recorder;
    return NULL;
}

UMOCKCALL_HANDLE umockcallrecorder_get_last_expected_call(UMOCKCALLRECORDER_HANDLE umock_call_recorder)
{
    (void)umock_call_recorder;
    return NULL;
}

int umocktypes_init(void)
{
    return 0;
}

void umocktypes_deinit(void)
{
    umocktypes_deinit_call_count++;
}

int umocktypes_c_register_types(void)
{
    return 0;
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

    umocktypes_deinit_call_count = 0;
    umockcallrecorder_destroy_call_count = 0;
    umockcallrecorder_reset_all_calls_call_count = 0;
}

TEST_FUNCTION_CLEANUP(test_function_cleanup)
{
    TEST_MUTEX_RELEASE(test_mutex);
}

/* umock_c_deinit */

/* Tests_SRS_UMOCK_C_01_010: [ If the module is not initialized, umock_c_deinit shall do nothing. ] */
TEST_FUNCTION(umock_c_deinit_when_not_initialized_does_nothing)
{
    // arrange

    // act
    umock_c_deinit();

    // assert
    ASSERT_ARE_EQUAL(size_t, 0, umocktypes_deinit_call_count);
    ASSERT_ARE_EQUAL(size_t, 0, umockcallrecorder_destroy_call_count);
}

/* umock_c_reset_all_calls */

/* Tests_SRS_UMOCK_C_01_012: [ If the module is not initialized, umock_c_reset_all_calls shall do nothing. ]*/
TEST_FUNCTION(umock_c_reset_all_calls_when_not_initialized_does_nothing)
{
    // arrange

    // act
    umock_c_reset_all_calls();

    // assert
    ASSERT_ARE_EQUAL(size_t, 0, umockcallrecorder_reset_all_calls_call_count);
}

END_TEST_SUITE(umock_c_without_init_unittests)
