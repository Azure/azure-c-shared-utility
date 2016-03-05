// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdint.h>
#include "testrunnerswitcher.h"

#define ENABLE_MOCKS

#include "umock_c.h"
#include "test_dependency.h"
#include "umockvalue_charptr.h"

BEGIN_TEST_SUITE(umock_c_unittests)

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    /* TODO: how shall we fail here? */
}

TEST_SUITE_INITIALIZE(suite_init)
{
    ASSERT_ARE_EQUAL(int, 0, umock_c_init(on_umock_c_error));
}

TEST_SUITE_CLEANUP(suite_cleanup)
{
    umock_c_deinit();
}

TEST_FUNCTION_INITIALIZE(test_function_init)
{
    ASSERT_ARE_EQUAL(int, 0, umock_c_reset_all_calls());
}

TEST_FUNCTION_CLEANUP(test_function_cleanup)
{
}

/* STRICT_EXPECTED_CALL */

/* Tests_SRS_UMOCK_C_01_013: [STRICT_EXPECTED_CALL shall record that a certain call is expected.] */
/* Tests_SRS_UMOCK_C_01_015: [The call argument shall be the complete function invocation.]*/
TEST_FUNCTION(STRICT_EXPECTED_CALL_without_an_actual_call_yields_a_missing_call)
{
    // arrange

    // act
    STRICT_EXPECTED_CALL(test_dependency_no_args());

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "[test_dependency_no_args()]", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_actual_calls());
}

/* Tests_SRS_UMOCK_C_01_013: [STRICT_EXPECTED_CALL shall record that a certain call is expected.] */
TEST_FUNCTION(two_STRICT_EXPECTED_CALL_without_an_actual_call_yields_2_missing_calls)
{
    // arrange

    // act
    STRICT_EXPECTED_CALL(test_dependency_no_args());
    STRICT_EXPECTED_CALL(test_dependency_no_args());

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "[test_dependency_no_args()][test_dependency_no_args()]", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_actual_calls());
}

/* Tests_SRS_UMOCK_C_01_014: [For each argument the argument value shall be stored for later comparison with actual calls.] */
TEST_FUNCTION(a_STRICT_EXPECTED_CALL_with_one_argument_without_an_actual_call_yields_1_missing_call)
{
    // arrange

    // act
    STRICT_EXPECTED_CALL(test_dependency_1_arg(42));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "[test_dependency_1_arg(42)]", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_actual_calls());
}

/* Tests_SRS_UMOCK_C_01_014: [For each argument the argument value shall be stored for later comparison with actual calls.] */
TEST_FUNCTION(a_STRICT_EXPECTED_CALL_with_2_arguments_without_an_actual_call_yields_1_missing_call)
{
    // arrange

    // act
    STRICT_EXPECTED_CALL(test_dependency_2_args(42, 43));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "[test_dependency_2_args(42,43)]", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_actual_calls());
}

/* Tests_SRS_UMOCK_C_01_013: [STRICT_EXPECTED_CALL shall record that a certain call is expected.] */
TEST_FUNCTION(two_different_STRICT_EXPECTED_CALL_instances_without_an_actual_call_yields_2_missing_calls)
{
    // arrange

    // act
    STRICT_EXPECTED_CALL(test_dependency_no_args());
    STRICT_EXPECTED_CALL(test_dependency_1_arg(42));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "[test_dependency_no_args()][test_dependency_1_arg(42)]", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_actual_calls());
}

/* Tests_SRS_UMOCK_C_01_115: [ umock_c shall compare calls in order. ]*/
TEST_FUNCTION(two_different_STRICT_EXPECTED_CALL_instances_without_an_actual_call_yields_2_missing_calls_with_order_preserved)
{
    // arrange

    // act
    STRICT_EXPECTED_CALL(test_dependency_1_arg(42));
    STRICT_EXPECTED_CALL(test_dependency_no_args());

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "[test_dependency_1_arg(42)][test_dependency_no_args()]", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_actual_calls());
}

END_TEST_SUITE(umock_c_unittests)
