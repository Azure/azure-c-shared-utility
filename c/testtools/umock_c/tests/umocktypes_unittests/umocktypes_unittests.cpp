// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdint.h>
#include <stdlib.h>
#include "testrunnerswitcher.h"
#include "umocktypes.h"

/* TODO:
- serialize tests
- test failures of malloc
- add deinit tests as separate project
- umockc unit test project should not include the full umockc
*/

typedef struct umocktypename_normalize_CALL_TAG
{
    char* type_name;
} umocktypename_normalize_CALL;

static umocktypename_normalize_CALL* umocktypename_normalize_calls;
static size_t umocktypename_normalize_call_count;
static const char* umocktypename_normalize_call_result;

typedef struct test_stringify_func_testtype_CALL_TAG
{
    const void* value;
} test_stringify_func_testtype_CALL;

static test_stringify_func_testtype_CALL* test_stringify_func_testtype_calls;
static size_t test_stringify_func_testtype_call_count;
static const char* test_stringify_func_testtype_call_result;

extern "C"
{
    char* umocktypename_normalize(const char* type_name)
    {
        char* result;

        umocktypename_normalize_CALL* new_calls = (umocktypename_normalize_CALL*)realloc(umocktypename_normalize_calls, sizeof(umocktypename_normalize_CALL) * (umocktypename_normalize_call_count + 1));
        if (new_calls != NULL)
        {
            size_t type_name_length = strlen(type_name);
            umocktypename_normalize_calls = new_calls;
            umocktypename_normalize_calls[umocktypename_normalize_call_count].type_name = (char*)malloc(type_name_length + 1);
            (void)memcpy(umocktypename_normalize_calls[umocktypename_normalize_call_count].type_name, type_name, type_name_length + 1);
            umocktypename_normalize_call_count++;
        }

        if (umocktypename_normalize_call_result != NULL)
        {
            size_t result_length = strlen(umocktypename_normalize_call_result);
            result = (char*)malloc(result_length + 1);
            (void)memcpy(result, umocktypename_normalize_call_result, result_length + 1);
        }
        else
        {
            result = NULL;
        }

        return result;
    }
}

static char* test_stringify_func_testtype(const void* value)
{
    char* result;
    test_stringify_func_testtype_CALL* new_calls = (test_stringify_func_testtype_CALL*)realloc(test_stringify_func_testtype_calls, sizeof(test_stringify_func_testtype_CALL) * (test_stringify_func_testtype_call_count + 1));
    if (new_calls != NULL)
    {
        test_stringify_func_testtype_calls = new_calls;
        test_stringify_func_testtype_calls[test_stringify_func_testtype_call_count].value = value;
        test_stringify_func_testtype_call_count++;
    }

    if (test_stringify_func_testtype_call_result != NULL)
    {
        size_t result_length = strlen(test_stringify_func_testtype_call_result);
        result = (char*)malloc(result_length + 1);
        (void)memcpy(result, test_stringify_func_testtype_call_result, result_length + 1);
    }
    else
    {
        result = NULL;
    }

    return result;
}

static int test_copy_func_testtype(void* destination, const void* source)
{
    (void)destination, source;
    return 0;
}

void test_free_func_testtype(void* value)
{
    (void)value;
}

int test_are_equal_func_testtype(const void* left, const void* right)
{
    (void)left, right;
    return 0;
}

static char* test_stringify_func_testtype_2(const void* value)
{
    (void)value;
    return NULL;
}

static int test_copy_func_testtype_2(void* destination, const void* source)
{
    (void)destination, source;
    return 0;
}

void test_free_func_testtype_2(void* value)
{
    (void)value;
}

int test_are_equal_func_testtype_2(const void* left, const void* right)
{
    (void)left, right;
    return 0;
}

static const void* test_value_1 = (void*)0x4242;
static const void* test_value_2 = (void*)0x4243;

void reset_umocktypename_normalize_calls(void)
{
    if (umocktypename_normalize_calls != NULL)
    {
        size_t i;
        for (i = 0; i < umocktypename_normalize_call_count; i++)
        {
            free(umocktypename_normalize_calls[i].type_name);
        }

        free(umocktypename_normalize_calls);
        umocktypename_normalize_calls = NULL;
    }
    umocktypename_normalize_call_count = NULL;
}

void reset_test_stringify_testtype_calls(void)
{
    if (test_stringify_func_testtype_calls != NULL)
    {
        free(test_stringify_func_testtype_calls);
        test_stringify_func_testtype_calls = NULL;
    }
    test_stringify_func_testtype_call_count = NULL;
}

BEGIN_TEST_SUITE(umocktypes_unittests)

TEST_SUITE_INITIALIZE(suite_init)
{
}

TEST_SUITE_CLEANUP(suite_cleanup)
{
}

TEST_FUNCTION_INITIALIZE(test_function_init)
{
    umocktypename_normalize_calls = NULL;
    umocktypename_normalize_call_count = 0;
    umocktypename_normalize_call_result = NULL;

    test_stringify_func_testtype_calls = NULL;
    test_stringify_func_testtype_call_count = 0;
    test_stringify_func_testtype_call_result = NULL;
}

TEST_FUNCTION_CLEANUP(test_function_cleanup)
{
    reset_umocktypename_normalize_calls();
    reset_test_stringify_testtype_calls();

    umocktypes_deinit();
}

/* umocktypes_init */

/* Tests_SRS_UMOCKTYPES_01_001: [ umocktypes_init shall initialize the umocktypes module. ] */
/* Tests_SRS_UMOCKTYPES_01_003: [ On success umocktypes_init shall return 0. ]*/
TEST_FUNCTION(umocktypes_init_initializes_the_module)
{
    // arrange

    // act
    int result = umocktypes_init();

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
}

/* Tests_SRS_UMOCKTYPES_01_002: [ After initialization the list of registered type shall be empty. ] */
TEST_FUNCTION(after_umocktypes_init_no_type_shall_be_registered)
{
    // arrange
    (void)umocktypes_init();
    (void)umocktypes_register_type("testtype", test_stringify_func_testtype, test_are_equal_func_testtype, test_copy_func_testtype, test_free_func_testtype);
    umocktypes_deinit();
    (void)umocktypes_init();

    // act
    int result = umocktypes_are_equal("testtype", test_value_1, test_value_2);

    // assert
    ASSERT_ARE_EQUAL(int, -1, result);
}

/* Tests_SRS_UMOCKTYPES_01_004: [ umocktypes_init after another umocktypes_init without deinitializing the module shall fail and return a non-zero value. ]*/
TEST_FUNCTION(umocktypes_init_after_umocktypes_init_fails)
{
    // arrange
    (void)umocktypes_init();

    // act
    int result = umocktypes_init();

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
}

/* umocktypes_deinit */

/* Tests_SRS_UMOCKTYPES_01_005: [ umocktypes_deinit shall free all resources associated with the registered types and shall leave the module in a state where another init is possible. ]*/
TEST_FUNCTION(umocktypes_deinit_after_umocktypes_init_deinitializes_the_module)
{
    // arrange
    (void)umocktypes_init();
    (void)umocktypes_register_type("testtype1", test_stringify_func_testtype, test_are_equal_func_testtype, test_copy_func_testtype, test_free_func_testtype);
    (void)umocktypes_register_type("testtype2", test_stringify_func_testtype, test_are_equal_func_testtype, test_copy_func_testtype, test_free_func_testtype);

    // act
    umocktypes_deinit();

    // assert
    // no explicit assert
}

/* Tests_SRS_UMOCKTYPES_01_005: [ umocktypes_deinit shall free all resources associated with the registered types and shall leave the module in a state where another init is possible. ]*/
TEST_FUNCTION(umocktypes_deinit_after_umocktypes_init_when_no_types_registered_deinitializes_the_module)
{
    // arrange
    (void)umocktypes_init();

    // act
    umocktypes_deinit();

    // assert
    // no explicit assert
}

/* Tests_SRS_UMOCKTYPES_01_040: [ An umocktypes_init call after deinit shall succeed provided all underlying calls succeed. ]*/
TEST_FUNCTION(umocktypes_init_after_umocktypes_deinit_succeeds)
{
    // arrange
    (void)umocktypes_init();
    (void)umocktypes_register_type("testtype1", test_stringify_func_testtype, test_are_equal_func_testtype, test_copy_func_testtype, test_free_func_testtype);
    (void)umocktypes_register_type("testtype2", test_stringify_func_testtype, test_are_equal_func_testtype, test_copy_func_testtype, test_free_func_testtype);
    umocktypes_deinit();

    // act
    int result = umocktypes_init();

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
}

/* Tests_SRS_UMOCKTYPES_01_006: [ If the module was not initialized, umocktypes_deinit shall do nothing. ]*/
TEST_FUNCTION(umocktypes_deinit_if_the_module_was_not_initialized_shall_do_nothing)
{
    // arrange

    // act
    umocktypes_deinit();

    // assert
    // no explicit assert
}

/* umocktypes_register_type */

/* Tests_SRS_UMOCKTYPES_01_007: [ umocktypes_register_type shall register an interface made out of the stringify, are equal, copy and free functions for the type identified by the argument type. ] */
/* Tests_SRS_UMOCKTYPES_01_008: [ On success umocktypes_register_type shall return 0. ]*/
/* Tests_SRS_UMOCKTYPES_01_034: [ Before registering, the type string shall be normalized. ] */
TEST_FUNCTION(umocktypes_register_type_when_module_is_initialized_succeeds)
{
    // arrange
    (void)umocktypes_init();

    umocktypename_normalize_call_result = "testtype";

    // act
    int result = umocktypes_register_type("testtype", test_stringify_func_testtype, test_are_equal_func_testtype, test_copy_func_testtype, test_free_func_testtype);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(int, 1, umocktypename_normalize_call_count);
    ASSERT_ARE_EQUAL(char_ptr, "testtype", umocktypename_normalize_calls[0].type_name);
}

/* Tests_SRS_UMOCKTYPES_01_009: [ If any of the arguments is NULL, umocktypes_register_type shall fail and return a non-zero value. ]*/
TEST_FUNCTION(umocktypes_register_type_with_NULL_type_fails)
{
    // arrange
    (void)umocktypes_init();

    // act
    int result = umocktypes_register_type(NULL, test_stringify_func_testtype, test_are_equal_func_testtype, test_copy_func_testtype, test_free_func_testtype);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(int, 0, umocktypename_normalize_call_count);
}

/* Tests_SRS_UMOCKTYPES_01_009: [ If any of the arguments is NULL, umocktypes_register_type shall fail and return a non-zero value. ]*/
TEST_FUNCTION(umocktypes_register_type_with_NULL_stringify_function_fails)
{
    // arrange
    (void)umocktypes_init();

    // act
    int result = umocktypes_register_type("testtype", NULL, test_are_equal_func_testtype, test_copy_func_testtype, test_free_func_testtype);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(int, 0, umocktypename_normalize_call_count);
}

/* Tests_SRS_UMOCKTYPES_01_009: [ If any of the arguments is NULL, umocktypes_register_type shall fail and return a non-zero value. ]*/
TEST_FUNCTION(umocktypes_register_type_with_NULL_are_equal_function_fails)
{
    // arrange
    (void)umocktypes_init();

    // act
    int result = umocktypes_register_type("testtype", test_stringify_func_testtype, NULL, test_copy_func_testtype, test_free_func_testtype);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(int, 0, umocktypename_normalize_call_count);
}

/* Tests_SRS_UMOCKTYPES_01_009: [ If any of the arguments is NULL, umocktypes_register_type shall fail and return a non-zero value. ]*/
TEST_FUNCTION(umocktypes_register_type_with_NULL_copy_function_fails)
{
    // arrange
    (void)umocktypes_init();

    // act
    int result = umocktypes_register_type("testtype", test_stringify_func_testtype, test_are_equal_func_testtype, NULL, test_free_func_testtype);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(int, 0, umocktypename_normalize_call_count);
}

/* Tests_SRS_UMOCKTYPES_01_009: [ If any of the arguments is NULL, umocktypes_register_type shall fail and return a non-zero value. ]*/
TEST_FUNCTION(umocktypes_register_type_with_NULL_free_function_fails)
{
    // arrange
    (void)umocktypes_init();

    // act
    int result = umocktypes_register_type("testtype", test_stringify_func_testtype, test_are_equal_func_testtype, test_copy_func_testtype, NULL);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(int, 0, umocktypename_normalize_call_count);
}

/* Tests_SRS_UMOCKTYPES_01_010: [ If the type has already been registered with the same function pointers then umocktypes_register_type shall succeed and return 0. ]*/
TEST_FUNCTION(umocktypes_register_type_2_times_on_the_same_type_with_the_same_functions_succeeds)
{
    // arrange
    (void)umocktypes_init();

    umocktypename_normalize_call_result = "testtype";

    (void)umocktypes_register_type("testtype", test_stringify_func_testtype, test_are_equal_func_testtype, test_copy_func_testtype, test_free_func_testtype);
    reset_umocktypename_normalize_calls();

    // act
    int result = umocktypes_register_type("testtype", test_stringify_func_testtype, test_are_equal_func_testtype, test_copy_func_testtype, test_free_func_testtype);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(int, 1, umocktypename_normalize_call_count);
    ASSERT_ARE_EQUAL(char_ptr, "testtype", umocktypename_normalize_calls[0].type_name);
}

/* Tests_SRS_UMOCKTYPES_01_011: [ If the type has already been registered but at least one of the function pointers is different, umocktypes_register_type shall fail and return a non-zero value. ]*/
TEST_FUNCTION(umocktypes_register_type_2_times_on_the_same_type_with_different_stringify_function_fails)
{
    // arrange
    (void)umocktypes_init();

    umocktypename_normalize_call_result = "testtype";

    (void)umocktypes_register_type("testtype", test_stringify_func_testtype, test_are_equal_func_testtype, test_copy_func_testtype, test_free_func_testtype);
    reset_umocktypename_normalize_calls();

    // act
    int result = umocktypes_register_type("testtype", test_stringify_func_testtype_2, test_are_equal_func_testtype, test_copy_func_testtype, test_free_func_testtype);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(int, 1, umocktypename_normalize_call_count);
    ASSERT_ARE_EQUAL(char_ptr, "testtype", umocktypename_normalize_calls[0].type_name);
}

/* Tests_SRS_UMOCKTYPES_01_011: [ If the type has already been registered but at least one of the function pointers is different, umocktypes_register_type shall fail and return a non-zero value. ]*/
TEST_FUNCTION(umocktypes_register_type_2_times_on_the_same_type_with_different_are_equal_function_fails)
{
    // arrange
    (void)umocktypes_init();

    umocktypename_normalize_call_result = "testtype";

    (void)umocktypes_register_type("testtype", test_stringify_func_testtype, test_are_equal_func_testtype, test_copy_func_testtype, test_free_func_testtype);
    reset_umocktypename_normalize_calls();

    // act
    int result = umocktypes_register_type("testtype", test_stringify_func_testtype, test_are_equal_func_testtype_2, test_copy_func_testtype, test_free_func_testtype);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(int, 1, umocktypename_normalize_call_count);
    ASSERT_ARE_EQUAL(char_ptr, "testtype", umocktypename_normalize_calls[0].type_name);
}

/* Tests_SRS_UMOCKTYPES_01_011: [ If the type has already been registered but at least one of the function pointers is different, umocktypes_register_type shall fail and return a non-zero value. ]*/
TEST_FUNCTION(umocktypes_register_type_2_times_on_the_same_type_with_different_copy_function_fails)
{
    // arrange
    (void)umocktypes_init();

    umocktypename_normalize_call_result = "testtype";

    (void)umocktypes_register_type("testtype", test_stringify_func_testtype, test_are_equal_func_testtype, test_copy_func_testtype, test_free_func_testtype);
    reset_umocktypename_normalize_calls();

    // act
    int result = umocktypes_register_type("testtype", test_stringify_func_testtype, test_are_equal_func_testtype, test_copy_func_testtype_2, test_free_func_testtype);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(int, 1, umocktypename_normalize_call_count);
    ASSERT_ARE_EQUAL(char_ptr, "testtype", umocktypename_normalize_calls[0].type_name);
}

/* Tests_SRS_UMOCKTYPES_01_011: [ If the type has already been registered but at least one of the function pointers is different, umocktypes_register_type shall fail and return a non-zero value. ]*/
TEST_FUNCTION(umocktypes_register_type_2_times_on_the_same_type_with_different_free_function_fails)
{
    // arrange
    (void)umocktypes_init();

    umocktypename_normalize_call_result = "testtype";

    (void)umocktypes_register_type("testtype", test_stringify_func_testtype, test_are_equal_func_testtype, test_copy_func_testtype, test_free_func_testtype);
    reset_umocktypename_normalize_calls();

    // act
    int result = umocktypes_register_type("testtype", test_stringify_func_testtype, test_are_equal_func_testtype, test_copy_func_testtype, test_free_func_testtype_2);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(int, 1, umocktypename_normalize_call_count);
    ASSERT_ARE_EQUAL(char_ptr, "testtype", umocktypename_normalize_calls[0].type_name);
}

/* Tests_SRS_UMOCKTYPES_01_045: [ If normalizing the typename fails, umocktypes_register_type shall fail and return a non-zero value. ] */
TEST_FUNCTION(when_normalizing_the_type_fails_umocktypes_register_type_fails)
{
    // arrange
    (void)umocktypes_init();

    umocktypename_normalize_call_result = NULL;

    // act
    int result = umocktypes_register_type("char  *", test_stringify_func_testtype, test_are_equal_func_testtype, test_copy_func_testtype, test_free_func_testtype);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(int, 1, umocktypename_normalize_call_count);
    ASSERT_ARE_EQUAL(char_ptr, "char  *", umocktypename_normalize_calls[0].type_name);
}

/* umocktypes_stringify */

/* Tests_SRS_UMOCKTYPES_01_013: [ umocktypes_stringify shall return a char\* with the string representation of the value argument. ]*/
/* Tests_SRS_UMOCKTYPES_01_014: [ The string representation shall be obtained by calling the stringify function registered for the type identified by the argument type. ]*/
TEST_FUNCTION(umocktypes_stringify_calls_the_underlying_stringify)
{
    // arrange
    (void)umocktypes_init();
    umocktypename_normalize_call_result = "char*";
    test_stringify_func_testtype_call_result = "blahblah";
    (void)umocktypes_register_type("char *", test_stringify_func_testtype, test_are_equal_func_testtype, test_copy_func_testtype, test_free_func_testtype);
    reset_umocktypename_normalize_calls();

    // act
    char* result = umocktypes_stringify("char *", test_value_1);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "blahblah", result);
    ASSERT_ARE_EQUAL(int, 1, umocktypename_normalize_call_count);
    ASSERT_ARE_EQUAL(char_ptr, "char *", umocktypename_normalize_calls[0].type_name);
    ASSERT_ARE_EQUAL(int, 1, test_stringify_func_testtype_call_count);
    ASSERT_ARE_EQUAL(void_ptr, test_value_1, test_stringify_func_testtype_calls[0].value);

    // cleanup
    free(result);
}

END_TEST_SUITE(umocktypes_unittests)
