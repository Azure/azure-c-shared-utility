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
*/

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
}

TEST_FUNCTION_CLEANUP(test_function_cleanup)
{
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

END_TEST_SUITE(umocktypes_unittests)
