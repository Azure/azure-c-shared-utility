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

static UMOCKCALL_HANDLE test_expected_call = (UMOCKCALL_HANDLE)0x4242;
static UMOCKCALL_HANDLE test_actual_call = (UMOCKCALL_HANDLE)0x4243;
static UMOCKCALLRECORDER_HANDLE test_call_recorder = (UMOCKCALLRECORDER_HANDLE)0x4244;

static size_t umocktypes_init_call_count;
static int umocktypes_init_call_result;

static size_t umocktypes_c_register_types_call_count;
static int umocktypes_c_register_types_call_result;

static size_t umockcallrecorder_create_call_count;
static UMOCKCALLRECORDER_HANDLE umockcallrecorder_create_call_result;

static size_t umocktypes_deinit_call_count;

typedef struct umockcallrecorder_destroy_CALL_TAG
{
    UMOCKCALLRECORDER_HANDLE umock_call_recorder;
} umockcallrecorder_destroy_CALL;

static umockcallrecorder_destroy_CALL* umockcallrecorder_destroy_calls;
static size_t umockcallrecorder_destroy_call_count;

typedef struct umockcallrecorder_reset_all_calls_CALL_TAG
{
    UMOCKCALLRECORDER_HANDLE umock_call_recorder;
} umockcallrecorder_reset_all_calls_CALL;

static umockcallrecorder_reset_all_calls_CALL* umockcallrecorder_reset_all_calls_calls;
static size_t umockcallrecorder_reset_all_calls_call_count;

static size_t umockcallrecorder_get_actual_calls_call_count;
static size_t umockcallrecorder_get_expected_calls_call_count;
static size_t umock_c_get_last_expected_call_call_count;
static size_t umock_c_add_expected_call_call_count;
static size_t umock_c_add_actual_call_call_count;

UMOCKCALLRECORDER_HANDLE umockcallrecorder_create(void)
{
    umockcallrecorder_create_call_count++;
    return umockcallrecorder_create_call_result;
}

void umockcallrecorder_destroy(UMOCKCALLRECORDER_HANDLE umock_call_recorder)
{
    umockcallrecorder_destroy_CALL* new_calls = (umockcallrecorder_destroy_CALL*)realloc(umockcallrecorder_destroy_calls, sizeof(umockcallrecorder_destroy_CALL) * (umockcallrecorder_destroy_call_count + 1));
    if (new_calls != NULL)
    {
        umockcallrecorder_destroy_calls = new_calls;
        umockcallrecorder_destroy_calls[umockcallrecorder_destroy_call_count].umock_call_recorder = umock_call_recorder;
        umockcallrecorder_destroy_call_count++;
    }
}

int umockcallrecorder_reset_all_calls(UMOCKCALLRECORDER_HANDLE umock_call_recorder)
{
    umockcallrecorder_reset_all_calls_CALL* new_calls = (umockcallrecorder_reset_all_calls_CALL*)realloc(umockcallrecorder_reset_all_calls_calls, sizeof(umockcallrecorder_reset_all_calls_CALL) * (umockcallrecorder_reset_all_calls_call_count + 1));
    if (new_calls != NULL)
    {
        umockcallrecorder_reset_all_calls_calls = new_calls;
        umockcallrecorder_reset_all_calls_calls[umockcallrecorder_reset_all_calls_call_count].umock_call_recorder = umock_call_recorder;
        umockcallrecorder_reset_all_calls_call_count++;
    }
    return 0;
}

int umockcallrecorder_add_expected_call(UMOCKCALLRECORDER_HANDLE umock_call_recorder, UMOCKCALL_HANDLE mock_call)
{
    (void)umock_call_recorder, mock_call;
    umock_c_add_expected_call_call_count++;
    return 0;
}

int umockcallrecorder_add_actual_call(UMOCKCALLRECORDER_HANDLE umock_call_recorder, UMOCKCALL_HANDLE mock_call, UMOCKCALL_HANDLE* matched_call)
{
    (void)umock_call_recorder, mock_call;
    umock_c_add_actual_call_call_count++;
    return 0;
}

const char* umockcallrecorder_get_actual_calls(UMOCKCALLRECORDER_HANDLE umock_call_recorder)
{
    (void)umock_call_recorder;
    umockcallrecorder_get_actual_calls_call_count++;
    return NULL;
}

const char* umockcallrecorder_get_expected_calls(UMOCKCALLRECORDER_HANDLE umock_call_recorder)
{
    (void)umock_call_recorder;
    umockcallrecorder_get_expected_calls_call_count++;
    return NULL;
}

UMOCKCALL_HANDLE umockcallrecorder_get_last_expected_call(UMOCKCALLRECORDER_HANDLE umock_call_recorder)
{
    (void)umock_call_recorder;
    umock_c_get_last_expected_call_call_count++;
    return NULL;
}

int umocktypes_init(void)
{
    umocktypes_init_call_count++;
    return umocktypes_init_call_result;
}

void umocktypes_deinit(void)
{
    umocktypes_deinit_call_count++;
}

int umocktypes_c_register_types(void)
{
    umocktypes_c_register_types_call_count++;
    return umocktypes_c_register_types_call_result;
}

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

void reset_all_calls(void)
{
    umocktypes_init_call_count = 0;
    umocktypes_init_call_result = 0;

    umocktypes_c_register_types_call_count = 0;
    umocktypes_c_register_types_call_result = 0;

    umockcallrecorder_create_call_count = 0;
    umockcallrecorder_create_call_result = test_call_recorder;

    umocktypes_deinit_call_count = 0;

    if (umockcallrecorder_destroy_calls != NULL)
    {
        free(umockcallrecorder_destroy_calls);
    }
    umockcallrecorder_destroy_calls = NULL;
    umockcallrecorder_destroy_call_count = 0;

    if (umockcallrecorder_reset_all_calls_calls != NULL)
    {
        free(umockcallrecorder_reset_all_calls_calls);
    }
    umockcallrecorder_reset_all_calls_calls = NULL;
    umockcallrecorder_reset_all_calls_call_count = 0;

    umockcallrecorder_get_actual_calls_call_count = 0;
    umockcallrecorder_get_expected_calls_call_count = 0;
    umock_c_get_last_expected_call_call_count = 0;
    umock_c_add_expected_call_call_count = 0;
    umock_c_add_actual_call_call_count = 0;

    if (test_on_umock_c_error_calls != NULL)
    {
        free(test_on_umock_c_error_calls);
    }
    test_on_umock_c_error_calls = NULL;
    test_on_umock_c_error_call_count = 0;
}

TEST_MUTEX_HANDLE test_mutex;

BEGIN_TEST_SUITE(umock_c_unittests)

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

    reset_all_calls();
    umock_c_deinit();
}

TEST_FUNCTION_CLEANUP(test_function_cleanup)
{
    reset_all_calls();

    TEST_MUTEX_RELEASE(test_mutex);
}

/* umock_deinit */

/* Tests_SRS_UMOCK_C_01_001: [umock_c_init shall initialize the umock library.] */
/* Tests_SRS_UMOCK_C_01_023: [ umock_c_init shall initialize the umock types by calling umocktypes_init. ]*/
/* Tests_SRS_UMOCK_C_01_004: [ On success, umock_c_init shall return 0. ]*/
/* Tests_SRS_UMOCK_C_01_002: [ umock_c_init shall register the C naive types by calling umocktypes_c_register_types. ]*/
/* Tests_SRS_UMOCK_C_01_003: [ umock_c_init shall create a call recorder by calling umockcallrecorder_create. ]*/
/* Tests_SRS_UMOCK_C_01_006: [ The on_umock_c_error callback shall be stored to be used for later error callbacks. ]*/
TEST_FUNCTION(when_all_calls_succeed_umock_c_init_succeeds)
{
    // arrange

    // act
    int result = umock_c_init(test_on_umock_c_error);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(size_t, 1, umocktypes_init_call_count);
    ASSERT_ARE_EQUAL(size_t, 1, umocktypes_c_register_types_call_count);
    ASSERT_ARE_EQUAL(size_t, 1, umockcallrecorder_create_call_count);
}

/* Tests_SRS_UMOCK_C_01_005: [ If any of the calls fails, umock_c_init shall fail and return a non-zero value. ]*/
TEST_FUNCTION(when_umocktypes_init_fails_then_umock_c_init_fails)
{
    // arrange
    umocktypes_init_call_result = 1;

    // act
    int result = umock_c_init(test_on_umock_c_error);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(size_t, 1, umocktypes_init_call_count);
    ASSERT_ARE_EQUAL(size_t, 0, umocktypes_c_register_types_call_count);
    ASSERT_ARE_EQUAL(size_t, 0, umockcallrecorder_create_call_count);
}

/* Tests_SRS_UMOCK_C_01_005: [ If any of the calls fails, umock_c_init shall fail and return a non-zero value. ]*/
TEST_FUNCTION(when_umocktypes_c_register_types_fails_then_umock_c_init_fails)
{
    // arrange
    umocktypes_c_register_types_call_result = 1;

    // act
    int result = umock_c_init(test_on_umock_c_error);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(size_t, 1, umocktypes_init_call_count);
    ASSERT_ARE_EQUAL(size_t, 1, umocktypes_c_register_types_call_count);
    ASSERT_ARE_EQUAL(size_t, 0, umockcallrecorder_create_call_count);
}

/* Tests_SRS_UMOCK_C_01_005: [ If any of the calls fails, umock_c_init shall fail and return a non-zero value. ]*/
TEST_FUNCTION(when_creating_the_call_recorder_fails_then_umock_c_init_fails)
{
    // arrange
    umockcallrecorder_create_call_result = NULL;

    // act
    int result = umock_c_init(test_on_umock_c_error);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(size_t, 1, umocktypes_init_call_count);
    ASSERT_ARE_EQUAL(size_t, 1, umocktypes_c_register_types_call_count);
    ASSERT_ARE_EQUAL(size_t, 1, umockcallrecorder_create_call_count);
}

/* Tests_SRS_UMOCK_C_01_007: [ umock_c_init when umock is already initialized shall fail and return a non-zero value. ]*/
TEST_FUNCTION(umock_c_init_when_already_initialized_fails)
{
    // arrange
    umock_c_init(test_on_umock_c_error);

    // act
    int result = umock_c_init(test_on_umock_c_error);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
}

/* Tests_SRS_UMOCK_C_01_024: [ on_umock_c_error shall be optional. ]*/
TEST_FUNCTION(umock_c_init_with_NULL_callback_succeeds)
{
    // arrange

    // act
    int result = umock_c_init(NULL);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(size_t, 1, umocktypes_init_call_count);
    ASSERT_ARE_EQUAL(size_t, 1, umocktypes_c_register_types_call_count);
    ASSERT_ARE_EQUAL(size_t, 1, umockcallrecorder_create_call_count);
}

/* umock_c_deinit */

/* Tests_SRS_UMOCK_C_01_008: [ umock_c_deinit shall deinitialize the umock types by calling umocktypes_deinit. ]*/
/* Tests_SRS_UMOCK_C_01_009: [ umock_c_deinit shall free the call recorder created in umock_c_init. ]*/
TEST_FUNCTION(umock_c_deinit_deinitializes_types_and_destroys_call_recorder)
{
    // arrange
    (void)umock_c_init(NULL);

    // act
    umock_c_deinit();

    // assert
    ASSERT_ARE_EQUAL(size_t, 1, umocktypes_deinit_call_count);
    ASSERT_ARE_EQUAL(size_t, 1, umockcallrecorder_destroy_call_count);
    ASSERT_ARE_EQUAL(void_ptr, test_call_recorder, umockcallrecorder_destroy_calls[0].umock_call_recorder);
}

/* Tests_SRS_UMOCK_C_01_008: [ umock_c_deinit shall deinitialize the umock types by calling umocktypes_deinit. ]*/
/* Tests_SRS_UMOCK_C_01_009: [ umock_c_deinit shall free the call recorder created in umock_c_init. ]*/
TEST_FUNCTION(umock_c_deinit_when_not_initialized_does_nothing)
{
    // arrange
    (void)umock_c_init(NULL);
    umock_c_deinit();
    reset_all_calls();

    // act
    umock_c_deinit();

    // assert
    ASSERT_ARE_EQUAL(size_t, 0, umocktypes_deinit_call_count);
    ASSERT_ARE_EQUAL(size_t, 0, umockcallrecorder_destroy_call_count);
}

/* umock_c_reset_all_calls */

/* Tests_SRS_UMOCK_C_01_011: [ umock_c_reset_all_calls shall reset all calls by calling umockcallrecorder_reset_all_calls on the call recorder created in umock_c_init. ]*/
TEST_FUNCTION(umock_c_reset_all_calls_calls_the_call_recorder_reset_all_calls)
{
    // arrange
    (void)umock_c_init(NULL);

    // act
    umock_c_reset_all_calls();

    // assert
    ASSERT_ARE_EQUAL(size_t, 1, umockcallrecorder_reset_all_calls_call_count);
    ASSERT_ARE_EQUAL(void_ptr, test_call_recorder, umockcallrecorder_reset_all_calls_calls[0].umock_call_recorder);
}

END_TEST_SUITE(umock_c_unittests)
