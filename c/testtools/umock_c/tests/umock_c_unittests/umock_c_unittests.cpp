// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdint.h>
#include "testrunnerswitcher.h"

/* TODO:
- Switch to .c
- Add tests for failure cases for argument indexes
- Make it clear that ENABLE_MOCKS has tobe defined after including the unit under test header
*/

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

/* EXPECTED_CALL */

/* Tests_SRS_UMOCK_C_01_016: [EXPECTED_CALL shall record that a certain call is expected.] */
/* Tests_SRS_UMOCK_C_01_018: [The call argument shall be the complete function invocation.] */
TEST_FUNCTION(EXPECTED_CALL_without_an_actual_call_yields_a_missing_call)
{
    // arrange

    // act
    EXPECTED_CALL(test_dependency_no_args());

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "[test_dependency_no_args()]", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_actual_calls());
}

/* Tests_SRS_UMOCK_C_01_016: [EXPECTED_CALL shall record that a certain call is expected.] */
TEST_FUNCTION(two_EXPECTED_CALL_without_an_actual_call_yields_2_missing_calls)
{
    // arrange

    // act
    EXPECTED_CALL(test_dependency_no_args());
    EXPECTED_CALL(test_dependency_no_args());

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "[test_dependency_no_args()][test_dependency_no_args()]", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_actual_calls());
}

/* Tests_SRS_UMOCK_C_01_017: [No arguments shall be saved by default, unless other modifiers state it.] */
TEST_FUNCTION(an_EXPECTED_CALL_with_one_argument_without_an_actual_call_yields_1_missing_call)
{
    // arrange

    // act
    EXPECTED_CALL(test_dependency_1_arg(42));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "[test_dependency_1_arg(42)]", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_actual_calls());
}

/* Tests_SRS_UMOCK_C_01_017: [No arguments shall be saved by default, unless other modifiers state it.] */
TEST_FUNCTION(an_EXPECTED_CALL_with_2_arguments_without_an_actual_call_yields_1_missing_call)
{
    // arrange

    // act
    EXPECTED_CALL(test_dependency_2_args(42, 43));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "[test_dependency_2_args(42,43)]", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_actual_calls());
}

/* Tests_SRS_UMOCK_C_01_016: [EXPECTED_CALL shall record that a certain call is expected.] */
TEST_FUNCTION(two_different_EXPECTED_CALL_instances_without_an_actual_call_yields_2_missing_calls)
{
    // arrange

    // act
    EXPECTED_CALL(test_dependency_no_args());
    EXPECTED_CALL(test_dependency_1_arg(42));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "[test_dependency_no_args()][test_dependency_1_arg(42)]", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_actual_calls());
}

/* Tests_SRS_UMOCK_C_01_115: [ umock_c shall compare calls in order. ]*/
TEST_FUNCTION(two_different_EXPECTED_CALL_instances_without_an_actual_call_yields_2_missing_calls_with_order_preserved)
{
    // arrange

    // act
    EXPECTED_CALL(test_dependency_1_arg(42));
    EXPECTED_CALL(test_dependency_no_args());

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "[test_dependency_1_arg(42)][test_dependency_no_args()]", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_actual_calls());
}

/* Tests_SRS_UMOCK_C_01_017: [No arguments shall be saved by default, unless other modifiers state it.]*/
TEST_FUNCTION(EXPECTED_CALL_does_not_compare_arguments)
{
    // arrange

    // act
    EXPECTED_CALL(test_dependency_1_arg(42));

    test_dependency_1_arg(43);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_actual_calls());
}

/* Tests_SRS_UMOCK_C_01_017: [No arguments shall be saved by default, unless other modifiers state it.]*/
TEST_FUNCTION(EXPECTED_CALL_with_2_args_does_not_compare_arguments)
{
    // arrange

    // act
    EXPECTED_CALL(test_dependency_2_args(42,43));

    test_dependency_2_args(43, 44);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_actual_calls());
}

/* Call modifiers */

/* Tests_SRS_UMOCK_C_01_074: [When an expected call is recorded a call modifier interface in the form of a structure containing function pointers shall be returned to the caller.] */
TEST_FUNCTION(STRICT_EXPECTED_CALL_allows_call_modifiers)
{
    // arrange

    // act
    STRICT_EXPECTED_CALL(test_dependency_2_args(42, 43))
        .ValidateAllArguments();

    test_dependency_2_args(42, 43);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_actual_calls());
}

/* Chaining modifiers */

/* Tests_SRS_UMOCK_C_01_075: [The last modifier in a chain overrides previous modifiers if any collision occurs.]*/
TEST_FUNCTION(STRICT_EXPECTED_CALL_with_ignore_all_arguments_and_then_validate_all_args_still_validates_args)
{
    // arrange

    // act
    STRICT_EXPECTED_CALL(test_dependency_2_args(42, 43))
        .IgnoreAllArguments()
        .ValidateAllArguments();

    test_dependency_2_args(43, 44);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "[test_dependency_2_args(42,43)]", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "[test_dependency_2_args(43,44)]", umock_c_get_actual_calls());
}

/* Tests_SRS_UMOCK_C_01_075: [The last modifier in a chain overrides previous modifiers if any collision occurs.]*/
TEST_FUNCTION(EXPECTED_CALL_with_validate_all_arguments_and_then_ignore_all_args_still_ignores_args)
{
    // arrange

    // act
    EXPECTED_CALL(test_dependency_2_args(42, 43))
        .ValidateAllArguments()
        .IgnoreAllArguments();

    test_dependency_2_args(43, 44);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_actual_calls());
}

/* Tests_SRS_UMOCK_C_01_075: [The last modifier in a chain overrides previous modifiers if any collision occurs.]*/
TEST_FUNCTION(STRICT_EXPECTED_CALL_with_ignore_validate_ignore_all_arguments_ignores_args)
{
    // arrange

    // act
    STRICT_EXPECTED_CALL(test_dependency_2_args(42, 43))
        .IgnoreAllArguments()
        .ValidateAllArguments()
        .IgnoreAllArguments();

    test_dependency_2_args(43, 44);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_actual_calls());
}

/* Tests_SRS_UMOCK_C_01_075: [The last modifier in a chain overrides previous modifiers if any collision occurs.]*/
TEST_FUNCTION(STRICT_EXPECTED_CALL_with_validate_ignore_validate_all_arguments_validates_args)
{
    // arrange

    // act
    EXPECTED_CALL(test_dependency_2_args(42, 43))
        .ValidateAllArguments()
        .IgnoreAllArguments()
        .ValidateAllArguments();

    test_dependency_2_args(43, 44);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "[test_dependency_2_args(42,43)]", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "[test_dependency_2_args(43,44)]", umock_c_get_actual_calls());
}

/* IgnoreAllArguments */

/* Tests_SRS_UMOCK_C_01_076: [The IgnoreAllArguments call modifier shall record that for that specific call all arguments will be ignored for that specific call.] */
TEST_FUNCTION(IgnoreAllArguments_ignores_args_on_a_STRICT_EXPECTED_CALL)
{
    // arrange

    // act
    STRICT_EXPECTED_CALL(test_dependency_2_args(42, 43))
        .IgnoreAllArguments();

    test_dependency_2_args(43, 44);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_actual_calls());
}

/* ValidateAllArguments */

/* Tests_SRS_UMOCK_C_01_077: [The ValidateAllArguments call modifier shall record that for that specific call all arguments will be validated.] */
TEST_FUNCTION(ValidateAllArguments_validates_all_args_on_an_EXPECTED_CALL)
{
    // arrange

    // act
    EXPECTED_CALL(test_dependency_2_args(42, 43))
        .ValidateAllArguments();

    test_dependency_2_args(43, 44);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "[test_dependency_2_args(42,43)]", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "[test_dependency_2_args(43,44)]", umock_c_get_actual_calls());
}

/* Tests_SRS_UMOCK_C_01_077: [The ValidateAllArguments call modifier shall record that for that specific call all arguments will be validated.] */
TEST_FUNCTION(ValidateAllArguments_for_a_function_without_arguments_means_nothing)
{
    // arrange

    // act
    EXPECTED_CALL(test_dependency_no_args())
        .ValidateAllArguments();

    test_dependency_no_args();

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_actual_calls());
}

/* IgnoreArgument_{arg_name} */

/* Tests_SRS_UMOCK_C_01_078: [The IgnoreArgument_{arg_name} call modifier shall record that the argument identified by arg_name will be ignored for that specific call.] */
TEST_FUNCTION(IgnoreArgument_by_name_ignores_only_that_argument_on_a_STRICT_EXPECTED_CALL)
{
    // arrange

    // act
    STRICT_EXPECTED_CALL(test_dependency_2_args(42, 43))
        .IgnoreArgument_a();

    test_dependency_2_args(41, 43);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_actual_calls());
}

/* Tests_SRS_UMOCK_C_01_078: [The IgnoreArgument_{arg_name} call modifier shall record that the argument identified by arg_name will be ignored for that specific call.] */
TEST_FUNCTION(IgnoreArgument_by_name_with_second_argument_ignores_only_that_argument_on_a_STRICT_EXPECTED_CALL)
{
    // arrange
    STRICT_EXPECTED_CALL(test_dependency_2_args(42, 43))
        .IgnoreArgument_b();

    // act
    test_dependency_2_args(42, 42);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_actual_calls());
}

/* ValidateArgument_{arg_name} */

/* Tests_SRS_UMOCK_C_01_079: [The ValidateArgument_{arg_name} call modifier shall record that the argument identified by arg_name will be validated for that specific call.]*/
TEST_FUNCTION(ValidateArgument_by_name_validates_only_that_argument_on_an_EXPECTED_CALL)
{
    // arrange
    EXPECTED_CALL(test_dependency_2_args(42, 43))
        .ValidateArgument_a();

    // act
    test_dependency_2_args(42, 44);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_actual_calls());
}

/* Tests_SRS_UMOCK_C_01_079: [The ValidateArgument_{arg_name} call modifier shall record that the argument identified by arg_name will be validated for that specific call.]*/
TEST_FUNCTION(ValidateArgument_by_name_validates_only_that_argument_on_an_EXPECTED_CALL_and_args_are_different)
{
    // arrange
    EXPECTED_CALL(test_dependency_2_args(42, 43))
        .ValidateArgument_a();

    // act
    test_dependency_2_args(41, 44);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "[test_dependency_2_args(42,43)]", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "[test_dependency_2_args(41,44)]", umock_c_get_actual_calls());
}

/* Tests_SRS_UMOCK_C_01_079: [The ValidateArgument_{arg_name} call modifier shall record that the argument identified by arg_name will be validated for that specific call.]*/
TEST_FUNCTION(ValidateArgument_by_name_2nd_arg_validates_only_that_argument_on_an_EXPECTED_CALL)
{
    // arrange
    EXPECTED_CALL(test_dependency_2_args(42, 43))
        .ValidateArgument_b();

    // act
    test_dependency_2_args(41, 43);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_actual_calls());
}

/* Tests_SRS_UMOCK_C_01_079: [The ValidateArgument_{arg_name} call modifier shall record that the argument identified by arg_name will be validated for that specific call.]*/
TEST_FUNCTION(ValidateArgument_by_name_2nd_arg_validates_only_that_argument_on_an_EXPECTED_CALL_and_args_are_different)
{
    // arrange
    EXPECTED_CALL(test_dependency_2_args(42, 43))
        .ValidateArgument_b();

    // act
    test_dependency_2_args(42, 44);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "[test_dependency_2_args(42,43)]", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "[test_dependency_2_args(42,44)]", umock_c_get_actual_calls());
}

/* IgnoreArgument */

/* Tests_SRS_UMOCK_C_01_080: [The IgnoreArgument call modifier shall record that the indexth argument will be ignored for that specific call.]*/
TEST_FUNCTION(IgnoreArgument_by_index_for_first_arg_ignores_the_first_argument)
{
    // arrange
    STRICT_EXPECTED_CALL(test_dependency_2_args(42, 43))
        .IgnoreArgument(1);

    // act
    test_dependency_2_args(41, 43);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_actual_calls());
}

/* Tests_SRS_UMOCK_C_01_080: [The IgnoreArgument call modifier shall record that the indexth argument will be ignored for that specific call.]*/
TEST_FUNCTION(IgnoreArgument_by_index_for_second_arg_ignores_the_second_argument)
{
    // arrange
    STRICT_EXPECTED_CALL(test_dependency_2_args(41, 42))
        .IgnoreArgument(2);

    // act
    test_dependency_2_args(41, 43);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_actual_calls());
}

/* Tests_SRS_UMOCK_C_01_080: [The IgnoreArgument call modifier shall record that the indexth argument will be ignored for that specific call.]*/
TEST_FUNCTION(IgnoreArgument_by_index_for_first_arg_ignores_only_the_first_argument)
{
    // arrange
    STRICT_EXPECTED_CALL(test_dependency_2_args(42, 43))
        .IgnoreArgument(1);

    // act
    test_dependency_2_args(42, 42);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "[test_dependency_2_args(42,43)]", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "[test_dependency_2_args(42,42)]", umock_c_get_actual_calls());
}

/* Tests_SRS_UMOCK_C_01_080: [The IgnoreArgument call modifier shall record that the indexth argument will be ignored for that specific call.]*/
TEST_FUNCTION(IgnoreArgument_by_index_for_second_arg_ignores_only_the_second_argument)
{
    // arrange
    STRICT_EXPECTED_CALL(test_dependency_2_args(41, 42))
        .IgnoreArgument(2);

    // act
    test_dependency_2_args(42, 42);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "[test_dependency_2_args(41,42)]", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "[test_dependency_2_args(42,42)]", umock_c_get_actual_calls());
}

/* ValidateArgument */

/* Tests_SRS_UMOCK_C_01_082: [The ValidateArgument call modifier shall record that the indexth argument will be validated for that specific call.]*/
TEST_FUNCTION(ValidateArgument_by_index_for_first_arg_ignores_the_first_argument)
{
    // arrange
    EXPECTED_CALL(test_dependency_2_args(42, 43))
        .ValidateArgument(1);

    // act
    test_dependency_2_args(41, 43);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "[test_dependency_2_args(42,43)]", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "[test_dependency_2_args(41,43)]", umock_c_get_actual_calls());
}

/* Tests_SRS_UMOCK_C_01_082: [The ValidateArgument call modifier shall record that the indexth argument will be validated for that specific call.]*/
TEST_FUNCTION(ValidateArgument_by_index_for_second_arg_validates_the_second_argument)
{
    // arrange
    EXPECTED_CALL(test_dependency_2_args(42, 42))
        .ValidateArgument(2);

    // act
    test_dependency_2_args(42, 43);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "[test_dependency_2_args(42,42)]", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "[test_dependency_2_args(42,43)]", umock_c_get_actual_calls());
}

/* Tests_SRS_UMOCK_C_01_082: [The ValidateArgument call modifier shall record that the indexth argument will be validated for that specific call.]*/
TEST_FUNCTION(ValidateArgument_by_index_for_first_arg_validates_only_the_first_argument)
{
    // arrange
    EXPECTED_CALL(test_dependency_2_args(42, 43))
        .ValidateArgument(1);

    // act
    test_dependency_2_args(42, 42);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_actual_calls());
}

/* Tests_SRS_UMOCK_C_01_082: [The ValidateArgument call modifier shall record that the indexth argument will be validated for that specific call.]*/
TEST_FUNCTION(ValidateArgument_by_index_for_second_arg_validates_only_the_second_argument)
{
    // arrange
    EXPECTED_CALL(test_dependency_2_args(42, 42))
        .ValidateArgument(2);

    // act
    test_dependency_2_args(43, 42);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_actual_calls());
}

/* SetReturn */

/* Tests_SRS_UMOCK_C_01_084: [The SetReturn call modifier shall record that when an actual call is matched with the specific expected call, it shall return the result value to the code under test.] */
TEST_FUNCTION(SetReturn_sets_the_return_value_for_a_strict_expected_call)
{
    // arrange
    STRICT_EXPECTED_CALL(test_dependency_no_args())
        .SetReturn(42);

    // act
    int result = test_dependency_no_args();

    // assert
    ASSERT_ARE_EQUAL(int, 42, result);
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_actual_calls());
}

/* Tests_SRS_UMOCK_C_01_084: [The SetReturn call modifier shall record that when an actual call is matched with the specific expected call, it shall return the result value to the code under test.] */
TEST_FUNCTION(SetReturn_sets_the_return_value_for_an_expected_call)
{
    // arrange
    EXPECTED_CALL(test_dependency_no_args())
        .SetReturn(42);

    // act
    int result = test_dependency_no_args();

    // assert
    ASSERT_ARE_EQUAL(int, 42, result);
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_actual_calls());
}

/* Tests_SRS_UMOCK_C_01_084: [The SetReturn call modifier shall record that when an actual call is matched with the specific expected call, it shall return the result value to the code under test.] */
TEST_FUNCTION(SetReturn_sets_the_return_value_only_for_a_matched_call)
{
    // arrange
    STRICT_EXPECTED_CALL(test_dependency_1_arg(42))
        .SetReturn(42);

    // act
    int result = test_dependency_1_arg(41);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
}

/* Tests_SRS_UMOCK_C_01_084: [The SetReturn call modifier shall record that when an actual call is matched with the specific expected call, it shall return the result value to the code under test.] */
TEST_FUNCTION(SetReturn_sets_independent_return_values_for_each_call)
{
    // arrange
    STRICT_EXPECTED_CALL(test_dependency_1_arg(42))
        .SetReturn(142);
    STRICT_EXPECTED_CALL(test_dependency_1_arg(43))
        .SetReturn(143);

    // act
    int result1 = test_dependency_1_arg(42);
    int result2 = test_dependency_1_arg(43);

    // assert
    ASSERT_ARE_EQUAL(int, 142, result1);
    ASSERT_ARE_EQUAL(int, 143, result2);
}

/* CopyOutArgumentBuffer */

/* Tests_SRS_UMOCK_C_01_087: [The CopyOutArgumentBuffer call modifier shall copy the memory pointed to by bytes and being length bytes so that it is later injected as an out argument when the code under test calls the mock function.] */
/* Tests_SRS_UMOCK_C_01_116: [ The argument targetted by CopyOutArgument shall also be marked as ignored. ] */
TEST_FUNCTION(CopyOutArgumentBuffer_copies_bytes_to_the_out_argument_for_a_strict_expected_call)
{
    // arrange
    int injected_int = 0x42;
    STRICT_EXPECTED_CALL(test_dependency_1_out_arg(IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(1, &injected_int, sizeof(injected_int));
    int actual_int = 0;

    // act
    (void)test_dependency_1_out_arg(&actual_int);

    // assert
    ASSERT_ARE_EQUAL(int, injected_int, actual_int);
}

/* Tests_SRS_UMOCK_C_01_087: [The CopyOutArgumentBuffer call modifier shall copy the memory pointed to by bytes and being length bytes so that it is later injected as an out argument when the code under test calls the mock function.] */
/* Tests_SRS_UMOCK_C_01_116: [ The argument targetted by CopyOutArgument shall also be marked as ignored. ] */
TEST_FUNCTION(CopyOutArgumentBuffer_copies_bytes_to_the_out_argument_for_an_expected_call)
{
    // arrange
    int injected_int = 0x42;
    EXPECTED_CALL(test_dependency_1_out_arg(IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(1, &injected_int, sizeof(injected_int));
    int actual_int = 0;

    // act
    (void)test_dependency_1_out_arg(&actual_int);

    // assert
    ASSERT_ARE_EQUAL(int, injected_int, actual_int);
}

/* Tests_SRS_UMOCK_C_01_087: [The CopyOutArgumentBuffer call modifier shall copy the memory pointed to by bytes and being length bytes so that it is later injected as an out argument when the code under test calls the mock function.] */
/* Tests_SRS_UMOCK_C_01_116: [ The argument targetted by CopyOutArgument shall also be marked as ignored. ] */
TEST_FUNCTION(CopyOutArgumentBuffer_only_copies_bytes_to_the_out_argument_that_was_specified)
{
    // arrange
    int injected_int = 0x42;
    EXPECTED_CALL(test_dependency_2_out_args(IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(1, &injected_int, sizeof(injected_int));
    int actual_int_1 = 0;
    int actual_int_2 = 0;

    // act
    (void)test_dependency_2_out_args(&actual_int_1, &actual_int_2);

    // assert
    ASSERT_ARE_EQUAL(int, injected_int, actual_int_1);
    ASSERT_ARE_EQUAL(int, 0, actual_int_2);
}

/* Tests_SRS_UMOCK_C_01_087: [The CopyOutArgumentBuffer call modifier shall copy the memory pointed to by bytes and being length bytes so that it is later injected as an out argument when the code under test calls the mock function.] */
/* Tests_SRS_UMOCK_C_01_116: [ The argument targetted by CopyOutArgument shall also be marked as ignored. ] */
TEST_FUNCTION(CopyOutArgumentBuffer_only_copies_bytes_to_the_second_out_argument)
{
    // arrange
    int injected_int = 0x42;
    EXPECTED_CALL(test_dependency_2_out_args(IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(2, &injected_int, sizeof(injected_int));
    int actual_int_1 = 0;
    int actual_int_2 = 0;

    // act
    (void)test_dependency_2_out_args(&actual_int_1, &actual_int_2);

    // assert
    ASSERT_ARE_EQUAL(int, 0, actual_int_1);
    ASSERT_ARE_EQUAL(int, injected_int, actual_int_2);
}

END_TEST_SUITE(umock_c_unittests)
