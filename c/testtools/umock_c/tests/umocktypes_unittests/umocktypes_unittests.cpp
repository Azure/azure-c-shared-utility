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

static char* test_stringify_func_testtype(const void* value)
{
    (void)value;
    return NULL;
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
}

TEST_FUNCTION_CLEANUP(test_function_cleanup)
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

    // act
    int result = umocktypes_register_type("testtype", test_stringify_func_testtype, test_are_equal_func_testtype, test_copy_func_testtype, test_free_func_testtype);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(int, 1, umocktypename_normalize_call_count);
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
}

/* Tests_SRS_UMOCKTYPES_01_010: [ If the type has already been registered with the same function pointers then umocktypes_register_type shall succeed and return 0. ]*/
TEST_FUNCTION(umocktypes_register_type_2_times_on_the_same_type_with_the_same_functions_succeeds)
{
    // arrange
    (void)umocktypes_init();
    umocktypes_register_type("testtype", test_stringify_func_testtype, test_are_equal_func_testtype, test_copy_func_testtype, test_free_func_testtype);

    // act
    int result = umocktypes_register_type("testtype", test_stringify_func_testtype, test_are_equal_func_testtype, test_copy_func_testtype, test_free_func_testtype);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
}

/* Tests_SRS_UMOCKTYPES_01_011: [ If the type has already been registered but at least one of the function pointers is different, umocktypes_register_type shall fail and return a non-zero value. ]*/
TEST_FUNCTION(umocktypes_register_type_2_times_on_the_same_type_with_different_stringify_function_fails)
{
    // arrange
    (void)umocktypes_init();
    umocktypes_register_type("testtype", test_stringify_func_testtype, test_are_equal_func_testtype, test_copy_func_testtype, test_free_func_testtype);

    // act
    int result = umocktypes_register_type("testtype", test_stringify_func_testtype_2, test_are_equal_func_testtype, test_copy_func_testtype, test_free_func_testtype);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
}

/* Tests_SRS_UMOCKTYPES_01_011: [ If the type has already been registered but at least one of the function pointers is different, umocktypes_register_type shall fail and return a non-zero value. ]*/
TEST_FUNCTION(umocktypes_register_type_2_times_on_the_same_type_with_different_are_equal_function_fails)
{
    // arrange
    (void)umocktypes_init();
    umocktypes_register_type("testtype", test_stringify_func_testtype, test_are_equal_func_testtype, test_copy_func_testtype, test_free_func_testtype);

    // act
    int result = umocktypes_register_type("testtype", test_stringify_func_testtype, test_are_equal_func_testtype_2, test_copy_func_testtype, test_free_func_testtype);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
}

/* Tests_SRS_UMOCKTYPES_01_011: [ If the type has already been registered but at least one of the function pointers is different, umocktypes_register_type shall fail and return a non-zero value. ]*/
TEST_FUNCTION(umocktypes_register_type_2_times_on_the_same_type_with_different_copy_function_fails)
{
    // arrange
    (void)umocktypes_init();
    umocktypes_register_type("testtype", test_stringify_func_testtype, test_are_equal_func_testtype, test_copy_func_testtype, test_free_func_testtype);

    // act
    int result = umocktypes_register_type("testtype", test_stringify_func_testtype, test_are_equal_func_testtype, test_copy_func_testtype_2, test_free_func_testtype);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
}

/* Tests_SRS_UMOCKTYPES_01_011: [ If the type has already been registered but at least one of the function pointers is different, umocktypes_register_type shall fail and return a non-zero value. ]*/
TEST_FUNCTION(umocktypes_register_type_2_times_on_the_same_type_with_different_free_function_fails)
{
    // arrange
    (void)umocktypes_init();
    umocktypes_register_type("testtype", test_stringify_func_testtype, test_are_equal_func_testtype, test_copy_func_testtype, test_free_func_testtype);

    // act
    int result = umocktypes_register_type("testtype", test_stringify_func_testtype, test_are_equal_func_testtype, test_copy_func_testtype, test_free_func_testtype_2);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
}

/* Tests_SRS_UMOCKTYPES_01_034: [ Before registering, the type string shall be normalized. ] */
/* Tests_SRS_UMOCKTYPES_01_039: [ All extra spaces (more than 1 space between non-space characters) shall be removed. ] */
TEST_FUNCTION(umocktypes_register_type_with_2_types_that_have_the_same_normalized_form_but_an_extra_space_before_star_detects_that_this_is_the_same_type)
{
    // arrange
    (void)umocktypes_init();
    umocktypes_register_type("char *", test_stringify_func_testtype, test_are_equal_func_testtype, test_copy_func_testtype, test_free_func_testtype);

    // act
    int result = umocktypes_register_type("char  *", test_stringify_func_testtype, test_are_equal_func_testtype, test_copy_func_testtype, test_free_func_testtype_2);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
}

/* Tests_SRS_UMOCKTYPES_01_039: [ All extra spaces (more than 1 space between non-space characters) shall be removed. ] */
TEST_FUNCTION(umocktypes_register_type_with_2_types_that_have_the_same_normalized_form_but_2_extra_spaces_before_star_detects_that_this_is_the_same_type)
{
    // arrange
    (void)umocktypes_init();
    umocktypes_register_type("char  *", test_stringify_func_testtype, test_are_equal_func_testtype, test_copy_func_testtype, test_free_func_testtype);

    // act
    int result = umocktypes_register_type("char   *", test_stringify_func_testtype, test_are_equal_func_testtype, test_copy_func_testtype, test_free_func_testtype_2);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
}

/* Tests_SRS_UMOCKTYPES_01_034: [ Before registering, the type string shall be normalized. ] */
/* Tests_SRS_UMOCKTYPES_01_039: [ All extra spaces (more than 1 space between non-space characters) shall be removed. ] */
TEST_FUNCTION(umocktypes_register_type_with_2_types_that_have_the_same_normalized_form_and_no_extra_spaces_detects_that_this_is_the_same_type)
{
    // arrange
    (void)umocktypes_init();
    umocktypes_register_type("char*", test_stringify_func_testtype, test_are_equal_func_testtype, test_copy_func_testtype, test_free_func_testtype);

    // act
    int result = umocktypes_register_type("char*", test_stringify_func_testtype, test_are_equal_func_testtype, test_copy_func_testtype, test_free_func_testtype_2);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
}

/* Tests_SRS_UMOCKTYPES_01_034: [ Before registering, the type string shall be normalized. ] */
/* Tests_SRS_UMOCKTYPES_01_039: [ All extra spaces (more than 1 space between non-space characters) shall be removed. ] */
TEST_FUNCTION(umocktypes_register_type_with_2_types_that_have_the_same_normalized_form_and_an_extra_space_at_the_beginning_detects_that_this_is_the_same_type)
{
    // arrange
    (void)umocktypes_init();
    umocktypes_register_type("char*", test_stringify_func_testtype, test_are_equal_func_testtype, test_copy_func_testtype, test_free_func_testtype);

    // act
    int result = umocktypes_register_type(" char*", test_stringify_func_testtype, test_are_equal_func_testtype, test_copy_func_testtype, test_free_func_testtype_2);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
}

END_TEST_SUITE(umocktypes_unittests)
