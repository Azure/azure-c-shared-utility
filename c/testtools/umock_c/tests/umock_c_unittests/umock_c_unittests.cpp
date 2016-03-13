// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdint.h>
#include <stdlib.h>
#include "testrunnerswitcher.h"

/* TODO:
- Switch to .c
- Make it clear that ENABLE_MOCKS has to be defined after including the unit under test header
*/

#define ENABLE_MOCKS

#include "umock_c.h"
#include "test_dependency.h"
#include "umocktypes_charptr.h"

typedef struct test_on_umock_c_error_CALL_TAG
{
    UMOCK_C_ERROR_CODE error_code;
} test_on_umock_c_error_CALL;

static test_on_umock_c_error_CALL* test_on_umock_c_error_calls;
static size_t test_on_umock_c_error_call_count;

DECLARE_UMOCK_POINTER_TYPE_FOR_TYPE(int, int);
DECLARE_UMOCK_POINTER_TYPE_FOR_TYPE(unsigned char, unsignedchar);

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

static int my_hook_result;
static int my_hook_test_dependency_no_args(void)
{
    return my_hook_result++;
}

static int my_hook_test_dependency_no_args_2(void)
{
    return 0x21;
}

BEGIN_TEST_SUITE(umock_c_unittests)

TEST_SUITE_INITIALIZE(suite_init)
{
    ASSERT_ARE_EQUAL(int, 0, umock_c_init(test_on_umock_c_error));
    REGISTER_UMOCK_VALUE_TYPE(int*, stringify_func_intptr, are_equal_func_intptr, copy_func_intptr, free_func_intptr);
    REGISTER_UMOCK_VALUE_TYPE(unsigned char*, stringify_func_unsignedcharptr, are_equal_func_unsignedcharptr, copy_func_unsignedcharptr, free_func_unsignedcharptr);
}

TEST_SUITE_CLEANUP(suite_cleanup)
{
    umock_c_deinit();
}

TEST_FUNCTION_INITIALIZE(test_function_init)
{
    test_on_umock_c_error_calls = NULL;
    test_on_umock_c_error_call_count = 0;

    ASSERT_ARE_EQUAL(int, 0, umock_c_reset_all_calls());
}

TEST_FUNCTION_CLEANUP(test_function_cleanup)
{
    REGISTER_GLOBAL_MOCK_RETURN_HOOK(test_dependency_no_args, NULL);

    free(test_on_umock_c_error_calls);
    test_on_umock_c_error_calls = NULL;
    test_on_umock_c_error_call_count = 0;
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

/* Tests_SRS_UMOCK_C_01_081: [If the index is out of range umock_c shall raise an error with the code UMOCK_C_ARG_INDEX_OUT_OF_RANGE.] */
TEST_FUNCTION(IgnoreArgument_by_index_with_index_0_triggers_the_on_error_callback)
{
    // arrange

    // act
    STRICT_EXPECTED_CALL(test_dependency_2_args(41, 42))
        .IgnoreArgument(0);

    // assert
    ASSERT_ARE_EQUAL(int, 1, test_on_umock_c_error_call_count);
    ASSERT_ARE_EQUAL(int, (int)UMOCK_C_ARG_INDEX_OUT_OF_RANGE, test_on_umock_c_error_calls[0].error_code);
}

/* Tests_SRS_UMOCK_C_01_081: [If the index is out of range umock_c shall raise an error with the code UMOCK_C_ARG_INDEX_OUT_OF_RANGE.] */
TEST_FUNCTION(IgnoreArgument_by_index_with_index_greater_than_arg_count_triggers_the_on_error_callback)
{
    // arrange

    // act
    STRICT_EXPECTED_CALL(test_dependency_2_args(41, 42))
        .IgnoreArgument(3);

    // assert
    ASSERT_ARE_EQUAL(int, 1, test_on_umock_c_error_call_count);
    ASSERT_ARE_EQUAL(int, (int)UMOCK_C_ARG_INDEX_OUT_OF_RANGE, test_on_umock_c_error_calls[0].error_code);
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

/* Tests_SRS_UMOCK_C_01_083: [If the index is out of range umock_c shall raise an error with the code UMOCK_C_ARG_INDEX_OUT_OF_RANGE.]*/
TEST_FUNCTION(ValidateArgument_by_index_with_0_index_triggers_the_on_error_callback)
{
    // arrange

    // act
    EXPECTED_CALL(test_dependency_2_args(42, 42))
        .ValidateArgument(0);

    // assert
    ASSERT_ARE_EQUAL(int, 1, test_on_umock_c_error_call_count);
    ASSERT_ARE_EQUAL(int, (int)UMOCK_C_ARG_INDEX_OUT_OF_RANGE, test_on_umock_c_error_calls[0].error_code);
}

/* Tests_SRS_UMOCK_C_01_083: [If the index is out of range umock_c shall raise an error with the code UMOCK_C_ARG_INDEX_OUT_OF_RANGE.]*/
TEST_FUNCTION(ValidateArgument_by_index_with_index_greater_than_arg_count_triggers_the_on_error_callback)
{
    // arrange

    // act
    EXPECTED_CALL(test_dependency_2_args(42, 42))
        .ValidateArgument(3);

    // assert
    ASSERT_ARE_EQUAL(int, 1, test_on_umock_c_error_call_count);
    ASSERT_ARE_EQUAL(int, (int)UMOCK_C_ARG_INDEX_OUT_OF_RANGE, test_on_umock_c_error_calls[0].error_code);
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

/* Tests_SRS_UMOCK_C_01_088: [The memory shall be copied.]*/
TEST_FUNCTION(CopyOutArgumentBuffer_copies_the_memory_for_later_use)
{
    // arrange
    int injected_int = 0x42;
    EXPECTED_CALL(test_dependency_1_out_arg(IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(1, &injected_int, sizeof(injected_int));
    int actual_int = 0;
    injected_int = 0;

    // act
    (void)test_dependency_1_out_arg(&actual_int);

    // assert
    ASSERT_ARE_EQUAL(int, 0x42, actual_int);
}

/* Tests_SRS_UMOCK_C_01_089: [The buffers for previous CopyOutArgumentBuffer calls shall be freed.]*/
/* Tests_SRS_UMOCK_C_01_133: [ If several calls to CopyOutArgumentBuffer are made, only the last buffer shall be kept. ]*/
TEST_FUNCTION(CopyOutArgumentBuffer_frees_allocated_buffers_for_previous_CopyOutArgumentBuffer)
{
    // arrange
    int injected_int = 0x42;
    EXPECTED_CALL(test_dependency_1_out_arg(IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(1, &injected_int, sizeof(injected_int))
        .CopyOutArgumentBuffer(1, &injected_int, sizeof(injected_int));
    int actual_int = 0;

    // act
    (void)test_dependency_1_out_arg(&actual_int);

    // assert
    ASSERT_ARE_EQUAL(int, 0x42, actual_int);
}

/* Tests_SRS_UMOCK_C_01_091: [If the index is out of range umock_c shall raise an error with the code UMOCK_C_ARG_INDEX_OUT_OF_RANGE.]*/
TEST_FUNCTION(CopyOutArgumentBuffer_with_0_index_triggers_the_error_callback)
{
    // arrange
    int injected_int = 0x42;

    // act
    EXPECTED_CALL(test_dependency_1_out_arg(IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(0, &injected_int, sizeof(injected_int));

    // assert
    ASSERT_ARE_EQUAL(int, 1, test_on_umock_c_error_call_count);
    ASSERT_ARE_EQUAL(int, (int)UMOCK_C_ARG_INDEX_OUT_OF_RANGE, test_on_umock_c_error_calls[0].error_code);
}

/* Tests_SRS_UMOCK_C_01_091: [If the index is out of range umock_c shall raise an error with the code UMOCK_C_ARG_INDEX_OUT_OF_RANGE.]*/
TEST_FUNCTION(CopyOutArgumentBuffer_with_index_higher_than_count_of_args_triggers_the_error_callback)
{
    // arrange
    int injected_int = 0x42;

    // act
    EXPECTED_CALL(test_dependency_1_out_arg(IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(2, &injected_int, sizeof(injected_int));

    // assert
    ASSERT_ARE_EQUAL(int, 1, test_on_umock_c_error_call_count);
    ASSERT_ARE_EQUAL(int, (int)UMOCK_C_ARG_INDEX_OUT_OF_RANGE, test_on_umock_c_error_calls[0].error_code);
}

/* Tests_SRS_UMOCK_C_01_092: [If bytes is NULL or length is 0, umock_c shall raise an error with the code UMOCK_C_INVALID_ARGUMENT_BUFFER.] */
TEST_FUNCTION(CopyOutArgumentBuffer_with_NULL_bytes_triggers_the_error_callback)
{
    // arrange

    // act
    EXPECTED_CALL(test_dependency_1_out_arg(IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(1, NULL, sizeof(int));

    // assert
    ASSERT_ARE_EQUAL(int, 1, test_on_umock_c_error_call_count);
    ASSERT_ARE_EQUAL(int, (int)UMOCK_C_INVALID_ARGUMENT_BUFFER, test_on_umock_c_error_calls[0].error_code);
}

/* Tests_SRS_UMOCK_C_01_092: [If bytes is NULL or length is 0, umock_c shall raise an error with the code UMOCK_C_INVALID_ARGUMENT_BUFFER.] */
TEST_FUNCTION(CopyOutArgumentBuffer_with_0_length_triggers_the_error_callback)
{
    // arrange
    int injected_int = 0x42;

    // act
    EXPECTED_CALL(test_dependency_1_out_arg(IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(1, &injected_int, 0);

    // assert
    ASSERT_ARE_EQUAL(int, 1, test_on_umock_c_error_call_count);
    ASSERT_ARE_EQUAL(int, (int)UMOCK_C_INVALID_ARGUMENT_BUFFER, test_on_umock_c_error_calls[0].error_code);
}

/* Tests_SRS_UMOCK_C_01_087: [The CopyOutArgumentBuffer call modifier shall copy the memory pointed to by bytes and being length bytes so that it is later injected as an out argument when the code under test calls the mock function.] */
/* Tests_SRS_UMOCK_C_01_116: [ The argument targetted by CopyOutArgument shall also be marked as ignored. ] */
TEST_FUNCTION(CopyOutArgumentBuffer_when_an_error_occurs_preserves_the_previous_state)
{
    // arrange
    int injected_int = 0x42;
    int injected_int_2 = 0x43;
    EXPECTED_CALL(test_dependency_2_out_args(IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(2, &injected_int, sizeof(injected_int))
        .CopyOutArgumentBuffer(0, &injected_int_2, sizeof(injected_int_2));
    int actual_int_1 = 0;
    int actual_int_2 = 0;

    // act
    (void)test_dependency_2_out_args(&actual_int_1, &actual_int_2);

    // assert
    ASSERT_ARE_EQUAL(int, 0, actual_int_1);
    ASSERT_ARE_EQUAL(int, injected_int, actual_int_2);
}

/* ValidateArgumentBuffer */

/* Tests_SRS_UMOCK_C_01_095: [The ValidateArgumentBuffer call modifier shall copy the memory pointed to by bytes and being length bytes so that it is later compared against a pointer type argument when the code under test calls the mock function.] */
/* Tests_SRS_UMOCK_C_01_096: [If the content of the code under test buffer and the buffer supplied to ValidateArgumentBuffer does not match then this should be treated as a mismatch in argument comparison for that argument.]*/
/* Tests_SRS_UMOCK_C_01_097: [ValidateArgumentBuffer shall implicitly perform an IgnoreArgument on the indexth argument.]*/
TEST_FUNCTION(ValidateArgumentBuffer_checks_the_argument_buffer)
{
    // arrange
    int expected_int = 0x42;
    int actual_int = 0x42;
    STRICT_EXPECTED_CALL(test_dependency_1_out_arg(IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(1, &expected_int, sizeof(expected_int));

    // act
    (void)test_dependency_1_out_arg(&actual_int);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_actual_calls());
}

/* Tests_SRS_UMOCK_C_01_095: [The ValidateArgumentBuffer call modifier shall copy the memory pointed to by bytes and being length bytes so that it is later compared against a pointer type argument when the code under test calls the mock function.] */
/* Tests_SRS_UMOCK_C_01_096: [If the content of the code under test buffer and the buffer supplied to ValidateArgumentBuffer does not match then this should be treated as a mismatch in argument comparison for that argument.]*/
/* Tests_SRS_UMOCK_C_01_097: [ValidateArgumentBuffer shall implicitly perform an IgnoreArgument on the indexth argument.]*/
TEST_FUNCTION(ValidateArgumentBuffer_checks_the_argument_buffer_and_mismatch_is_detected_when_content_does_not_match)
{
    // arrange
    unsigned char expected_buffer[] = { 0x42 };
    unsigned char actual_buffer[] = { 0x43 };
    char actual_string[64];
    STRICT_EXPECTED_CALL(test_dependency_buffer_arg(IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(1, expected_buffer, sizeof(expected_buffer));

    // act
    test_dependency_buffer_arg(actual_buffer);

    // assert
    (void)sprintf(actual_string, "[test_dependency_buffer_arg(%p)]", actual_buffer);
    ASSERT_ARE_EQUAL(char_ptr, "[test_dependency_buffer_arg([0x42])]", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, actual_string, umock_c_get_actual_calls());
}

/* Tests_SRS_UMOCK_C_01_095: [The ValidateArgumentBuffer call modifier shall copy the memory pointed to by bytes and being length bytes so that it is later compared against a pointer type argument when the code under test calls the mock function.] */
/* Tests_SRS_UMOCK_C_01_096: [If the content of the code under test buffer and the buffer supplied to ValidateArgumentBuffer does not match then this should be treated as a mismatch in argument comparison for that argument.]*/
/* Tests_SRS_UMOCK_C_01_097: [ValidateArgumentBuffer shall implicitly perform an IgnoreArgument on the indexth argument.]*/
TEST_FUNCTION(ValidateArgumentBuffer_checks_the_argument_buffer_and_mismatch_is_detected_when_content_does_not_match_for_expected_call)
{
    // arrange
    unsigned char expected_buffer[] = { 0x42 };
    unsigned char actual_buffer[] = { 0x43 };
    char actual_string[64];
    EXPECTED_CALL(test_dependency_buffer_arg(IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(1, expected_buffer, sizeof(expected_buffer));

    // act
    test_dependency_buffer_arg(actual_buffer);

    // assert
    (void)sprintf(actual_string, "[test_dependency_buffer_arg(%p)]", actual_buffer);
    ASSERT_ARE_EQUAL(char_ptr, "[test_dependency_buffer_arg([0x42])]", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, actual_string, umock_c_get_actual_calls());
}

/* Tests_SRS_UMOCK_C_01_099: [If the index is out of range umock_c shall raise an error with the code UMOCK_C_ARG_INDEX_OUT_OF_RANGE.]*/
TEST_FUNCTION(ValidateArgumentBuffer_with_0_index_triggers_an_error)
{
    // arrange
    unsigned char expected_buffer[] = { 0x42 };
    unsigned char actual_buffer[] = { 0x43 };
    EXPECTED_CALL(test_dependency_buffer_arg(IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(0, expected_buffer, sizeof(expected_buffer));

    // act
    test_dependency_buffer_arg(actual_buffer);

    // assert
    ASSERT_ARE_EQUAL(int, 1, test_on_umock_c_error_call_count);
    ASSERT_ARE_EQUAL(int, (int)UMOCK_C_ARG_INDEX_OUT_OF_RANGE, test_on_umock_c_error_calls[0].error_code);
}

/* Tests_SRS_UMOCK_C_01_099: [If the index is out of range umock_c shall raise an error with the code UMOCK_C_ARG_INDEX_OUT_OF_RANGE.]*/
TEST_FUNCTION(ValidateArgumentBuffer_with_index_higher_than_the_Arg_count_triggers_an_error)
{
    // arrange
    unsigned char expected_buffer[] = { 0x42 };
    unsigned char actual_buffer[] = { 0x43 };
    EXPECTED_CALL(test_dependency_buffer_arg(IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(2, expected_buffer, sizeof(expected_buffer));

    // act
    test_dependency_buffer_arg(actual_buffer);

    // assert
    ASSERT_ARE_EQUAL(int, 1, test_on_umock_c_error_call_count);
    ASSERT_ARE_EQUAL(int, (int)UMOCK_C_ARG_INDEX_OUT_OF_RANGE, test_on_umock_c_error_calls[0].error_code);
}

/* Tests_SRS_UMOCK_C_01_100: [If bytes is NULL or length is 0, umock_c shall raise an error with the code UMOCK_C_INVALID_ARGUMENT_BUFFER.] */
TEST_FUNCTION(ValidateArgumentBuffer_with_NULL_buffer_triggers_the_error_callback)
{
    // arrange

    // act
    EXPECTED_CALL(test_dependency_buffer_arg(IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(1, NULL, 1);

    // assert
    ASSERT_ARE_EQUAL(int, 1, test_on_umock_c_error_call_count);
    ASSERT_ARE_EQUAL(int, (int)UMOCK_C_INVALID_ARGUMENT_BUFFER, test_on_umock_c_error_calls[0].error_code);
}

/* Tests_SRS_UMOCK_C_01_100: [If bytes is NULL or length is 0, umock_c shall raise an error with the code UMOCK_C_INVALID_ARGUMENT_BUFFER.] */
TEST_FUNCTION(ValidateArgumentBuffer_with_0_length_triggers_the_error_callback)
{
    // arrange
    unsigned char expected_buffer[] = { 0x42 };

    // act
    EXPECTED_CALL(test_dependency_buffer_arg(IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(1, expected_buffer, 0);

    // assert
    ASSERT_ARE_EQUAL(int, 1, test_on_umock_c_error_call_count);
    ASSERT_ARE_EQUAL(int, (int)UMOCK_C_INVALID_ARGUMENT_BUFFER, test_on_umock_c_error_calls[0].error_code);
}

/* Tests_SRS_UMOCK_C_01_095: [The ValidateArgumentBuffer call modifier shall copy the memory pointed to by bytes and being length bytes so that it is later compared against a pointer type argument when the code under test calls the mock function.] */
/* Tests_SRS_UMOCK_C_01_096: [If the content of the code under test buffer and the buffer supplied to ValidateArgumentBuffer does not match then this should be treated as a mismatch in argument comparison for that argument.]*/
/* Tests_SRS_UMOCK_C_01_097: [ValidateArgumentBuffer shall implicitly perform an IgnoreArgument on the indexth argument.]*/
TEST_FUNCTION(ValidateArgumentBuffer_with_2_bytes_and_first_byte_different_checks_the_content)
{
    // arrange
    unsigned char expected_buffer[] = { 0x42, 0x41 };
    unsigned char actual_buffer[] = { 0x43, 0x41 };
    char actual_string[64];
    EXPECTED_CALL(test_dependency_buffer_arg(IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(1, expected_buffer, sizeof(expected_buffer));

    // act
    test_dependency_buffer_arg(actual_buffer);

    // assert
    (void)sprintf(actual_string, "[test_dependency_buffer_arg(%p)]", actual_buffer);
    ASSERT_ARE_EQUAL(char_ptr, "[test_dependency_buffer_arg([0x42 0x41])]", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, actual_string, umock_c_get_actual_calls());
}

/* Tests_SRS_UMOCK_C_01_095: [The ValidateArgumentBuffer call modifier shall copy the memory pointed to by bytes and being length bytes so that it is later compared against a pointer type argument when the code under test calls the mock function.] */
/* Tests_SRS_UMOCK_C_01_096: [If the content of the code under test buffer and the buffer supplied to ValidateArgumentBuffer does not match then this should be treated as a mismatch in argument comparison for that argument.]*/
/* Tests_SRS_UMOCK_C_01_097: [ValidateArgumentBuffer shall implicitly perform an IgnoreArgument on the indexth argument.]*/
TEST_FUNCTION(ValidateArgumentBuffer_with_2_bytes_and_second_byte_different_checks_the_content)
{
    // arrange
    unsigned char expected_buffer[] = { 0x42, 0x41 };
    unsigned char actual_buffer[] = { 0x42, 0x42 };
    char actual_string[64];
    EXPECTED_CALL(test_dependency_buffer_arg(IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(1, expected_buffer, sizeof(expected_buffer));

    // act
    test_dependency_buffer_arg(actual_buffer);

    // assert
    (void)sprintf(actual_string, "[test_dependency_buffer_arg(%p)]", actual_buffer);
    ASSERT_ARE_EQUAL(char_ptr, "[test_dependency_buffer_arg([0x42 0x41])]", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, actual_string, umock_c_get_actual_calls());
}

/* Tests_SRS_UMOCK_C_01_131: [ The memory pointed by bytes shall be copied. ]*/
TEST_FUNCTION(ValidateArgumentBuffer_copies_the_bytes_to_compare)
{
    // arrange
    unsigned char expected_buffer[] = { 0x42 };
    unsigned char actual_buffer[] = { 0x42 };
    EXPECTED_CALL(test_dependency_buffer_arg(IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(1, expected_buffer, sizeof(expected_buffer));

    expected_buffer[0] = 0x43;

    // act
    test_dependency_buffer_arg(actual_buffer);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_actual_calls());
}

/* Tests_SRS_UMOCK_C_01_132: [ If several calls to ValidateArgumentBuffer are made, only the last buffer shall be kept. ]*/
/* Tests_SRS_UMOCK_C_01_130: [ The buffers for previous ValidateArgumentBuffer calls shall be freed. ]*/
TEST_FUNCTION(When_ValidateArgumentBuffer_is_called_twice_the_last_buffer_is_used)
{
    // arrange
    unsigned char expected_buffer1[] = { 0x43 };
    unsigned char expected_buffer2[] = { 0x42 };
    unsigned char actual_buffer[] = { 0x42 };
    EXPECTED_CALL(test_dependency_buffer_arg(IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(1, expected_buffer1, sizeof(expected_buffer1))
        .ValidateArgumentBuffer(1, expected_buffer2, sizeof(expected_buffer2));

    // act
    test_dependency_buffer_arg(actual_buffer);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_actual_calls());
}

/* REGISTER_GLOBAL_MOCK_RETURN_HOOK */

/* Tests_SRS_UMOCK_C_01_104: [The REGISTER_GLOBAL_MOCK_RETURN_HOOK shall register a mock hook to be called every time the mocked function is called by production code.]*/
/* Tests_SRS_UMOCK_C_01_105: [The hook’s result shall be returned by the mock to the production code.]*/
/* Tests_SRS_UMOCK_C_01_106: [The signature for the hook shall be assumed to have exactly the same arguments and return as the mocked function.]*/
TEST_FUNCTION(REGISTER_GLOBAL_MOCK_RETURN_HOOK_registers_a_hook_for_the_mock)
{
    // arrange
    REGISTER_GLOBAL_MOCK_RETURN_HOOK(test_dependency_no_args, my_hook_test_dependency_no_args);
    my_hook_result = 0x42;

    // act
    int result = test_dependency_no_args();

    // assert
    ASSERT_ARE_EQUAL(int, 0x42, result);
}

/* Tests_SRS_UMOCK_C_01_104: [The REGISTER_GLOBAL_MOCK_RETURN_HOOK shall register a mock hook to be called every time the mocked function is called by production code.]*/
/* Tests_SRS_UMOCK_C_01_105: [The hook’s result shall be returned by the mock to the production code.]*/
TEST_FUNCTION(REGISTER_GLOBAL_MOCK_RETURN_HOOK_registers_a_hook_for_the_mock_that_returns_2_different_values)
{
    // arrange
    REGISTER_GLOBAL_MOCK_RETURN_HOOK(test_dependency_no_args, my_hook_test_dependency_no_args);
    my_hook_result = 0x42;

    // act
    int call1_result = test_dependency_no_args();
    int call2_result = test_dependency_no_args();

    // assert
    ASSERT_ARE_EQUAL(int, 0x42, call1_result);
    ASSERT_ARE_EQUAL(int, 0x43, call2_result);
}

/* Tests_SRS_UMOCK_C_01_107: [If there are multiple invocations of REGISTER_GLOBAL_MOCK_RETURN_HOOK, the last one shall take effect over the previous ones.] */
TEST_FUNCTION(REGISTER_GLOBAL_MOCK_RETURN_HOOK_twice_makes_the_last_hook_stick)
{
    // arrange
    REGISTER_GLOBAL_MOCK_RETURN_HOOK(test_dependency_no_args, my_hook_test_dependency_no_args);
    REGISTER_GLOBAL_MOCK_RETURN_HOOK(test_dependency_no_args, my_hook_test_dependency_no_args_2);
    my_hook_result = 0x42;

    // act
    int result = test_dependency_no_args();

    // assert
    ASSERT_ARE_EQUAL(int, 0x21, result);
}

/* Tests_SRS_UMOCK_C_01_134: [ REGISTER_GLOBAL_MOCK_RETURN_HOOK called with a NULL hook unregisters a previously registered hook. ]*/
TEST_FUNCTION(REGISTER_GLOBAL_MOCK_RETURN_HOOK_with_NULL_unregisters_a_previously_registered_hook)
{
    // arrange
    REGISTER_GLOBAL_MOCK_RETURN_HOOK(test_dependency_no_args, my_hook_test_dependency_no_args);
    REGISTER_GLOBAL_MOCK_RETURN_HOOK(test_dependency_no_args, NULL);

    // act
    int result = test_dependency_no_args();

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
}

END_TEST_SUITE(umock_c_unittests)
