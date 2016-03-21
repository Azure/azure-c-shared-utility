// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdint.h>
#include <stdlib.h>
#include "testrunnerswitcher.h"

/* TODO:
- Switch to .c
- Make it clear that ENABLE_MOCKS has to be defined after including the unit under test header
- Test freeing of return values allocated by the user in the copy functions
- Test set return value order
*/

#define ENABLE_MOCKS

#include "umock_c.h"
#include "test_dependency.h"

/* Tests_SRS_UMOCK_C_01_067: [char\* and const char\* shall be supported out of the box through a separate header, umockvalue_charptr.h.]*/
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

static int arg_a;
static int arg_b;
static int my_hook_test_dependency_2_args(int a, int b)
{
    arg_a = a;
    arg_b = b;
    return 0;
}

static unsigned int test_dependency_void_return_called = 0;
static void my_hook_test_dependency_void_return(void)
{
    test_dependency_void_return_called = 1;
}

static int test_dependency_global_mock_return_copy_fails_call_count = 0;
static int test_dependency_global_mock_return_copy_fails_fail_call = 0;

char* stringify_func_TEST_STRUCT_COPY_FAILS(const TEST_STRUCT_COPY_FAILS* value)
{
    char* result = (char*)malloc(1);
    result[0] = '\0';
    return result;
}

int are_equal_func_TEST_STRUCT_COPY_FAILS(const TEST_STRUCT_COPY_FAILS* left, const TEST_STRUCT_COPY_FAILS* right)
{
    return 1;
}

int copy_func_TEST_STRUCT_COPY_FAILS(TEST_STRUCT_COPY_FAILS* destination, const TEST_STRUCT_COPY_FAILS* source)
{
    int result;

    test_dependency_global_mock_return_copy_fails_call_count++;
    if (test_dependency_global_mock_return_copy_fails_fail_call == test_dependency_global_mock_return_copy_fails_call_count)
    {
        result = 1;
    }
    else
    {
        result = 0;
    }

    return result;
}

void free_func_TEST_STRUCT_COPY_FAILS(TEST_STRUCT_COPY_FAILS* value)
{
}

TEST_MUTEX_HANDLE test_mutex;

BEGIN_TEST_SUITE(umock_c_integrationtests)

TEST_SUITE_INITIALIZE(suite_init)
{
    test_mutex = TEST_MUTEX_CREATE();
    ASSERT_IS_NOT_NULL(test_mutex);

    ASSERT_ARE_EQUAL(int, 0, umock_c_init(test_on_umock_c_error));
    /* Tests_SRS_UMOCK_C_01_069: [The signature shall be: ...*/
    /* Tests_SRS_UMOCK_C_01_070: [umockvalue_charptr_register_types shall return 0 on success and non-zero on failure.]*/
    ASSERT_ARE_EQUAL(int, 0, umocktypes_charptr_register_types());
    REGISTER_UMOCK_VALUE_TYPE(int*, stringify_func_intptr, are_equal_func_intptr, copy_func_intptr, free_func_intptr);
    REGISTER_UMOCK_VALUE_TYPE(unsigned char*, stringify_func_unsignedcharptr, are_equal_func_unsignedcharptr, copy_func_unsignedcharptr, free_func_unsignedcharptr);
    REGISTER_UMOCK_VALUE_TYPE(TEST_STRUCT_COPY_FAILS, stringify_func_TEST_STRUCT_COPY_FAILS, are_equal_func_TEST_STRUCT_COPY_FAILS, copy_func_TEST_STRUCT_COPY_FAILS, free_func_TEST_STRUCT_COPY_FAILS);
}

TEST_SUITE_CLEANUP(suite_cleanup)
{
    umock_c_deinit();

    TEST_MUTEX_DESTROY(test_mutex);
}

TEST_FUNCTION_INITIALIZE(test_function_init)
{
    ASSERT_ARE_EQUAL(int, 0, TEST_MUTEX_ACQUIRE(test_mutex));

    test_on_umock_c_error_calls = NULL;
    test_on_umock_c_error_call_count = 0;
}

TEST_FUNCTION_CLEANUP(test_function_cleanup)
{
    umock_c_reset_all_calls();

    REGISTER_GLOBAL_MOCK_HOOK(test_dependency_no_args, NULL);

    free(test_on_umock_c_error_calls);
    test_on_umock_c_error_calls = NULL;
    test_on_umock_c_error_call_count = 0;

    TEST_MUTEX_RELEASE(test_mutex);
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

/* REGISTER_GLOBAL_MOCK_HOOK */

/* Tests_SRS_UMOCK_C_01_104: [The REGISTER_GLOBAL_MOCK_HOOK shall register a mock hook to be called every time the mocked function is called by production code.]*/
/* Tests_SRS_UMOCK_C_01_105: [The hook’s result shall be returned by the mock to the production code.]*/
/* Tests_SRS_UMOCK_C_01_106: [The signature for the hook shall be assumed to have exactly the same arguments and return as the mocked function.]*/
TEST_FUNCTION(REGISTER_GLOBAL_MOCK_HOOK_registers_a_hook_for_the_mock)
{
    // arrange
    REGISTER_GLOBAL_MOCK_HOOK(test_dependency_no_args, my_hook_test_dependency_no_args);
    my_hook_result = 0x42;

    // act
    int result = test_dependency_no_args();

    // assert
    ASSERT_ARE_EQUAL(int, 0x42, result);
}

/* Tests_SRS_UMOCK_C_01_104: [The REGISTER_GLOBAL_MOCK_HOOK shall register a mock hook to be called every time the mocked function is called by production code.]*/
/* Tests_SRS_UMOCK_C_01_105: [The hook’s result shall be returned by the mock to the production code.]*/
TEST_FUNCTION(REGISTER_GLOBAL_MOCK_HOOK_registers_a_hook_for_the_mock_that_returns_2_different_values)
{
    // arrange
    REGISTER_GLOBAL_MOCK_HOOK(test_dependency_no_args, my_hook_test_dependency_no_args);
    my_hook_result = 0x42;

    // act
    int call1_result = test_dependency_no_args();
    int call2_result = test_dependency_no_args();

    // assert
    ASSERT_ARE_EQUAL(int, 0x42, call1_result);
    ASSERT_ARE_EQUAL(int, 0x43, call2_result);
}

/* Tests_SRS_UMOCK_C_01_107: [If there are multiple invocations of REGISTER_GLOBAL_MOCK_HOOK, the last one shall take effect over the previous ones.] */
TEST_FUNCTION(REGISTER_GLOBAL_MOCK_HOOK_twice_makes_the_last_hook_stick)
{
    // arrange
    REGISTER_GLOBAL_MOCK_HOOK(test_dependency_no_args, my_hook_test_dependency_no_args);
    REGISTER_GLOBAL_MOCK_HOOK(test_dependency_no_args, my_hook_test_dependency_no_args_2);
    my_hook_result = 0x42;

    // act
    int result = test_dependency_no_args();

    // assert
    ASSERT_ARE_EQUAL(int, 0x21, result);
}

/* Tests_SRS_UMOCK_C_01_134: [ REGISTER_GLOBAL_MOCK_HOOK called with a NULL hook unregisters a previously registered hook. ]*/
TEST_FUNCTION(REGISTER_GLOBAL_MOCK_HOOK_with_NULL_unregisters_a_previously_registered_hook)
{
    // arrange
    REGISTER_GLOBAL_MOCK_HOOK(test_dependency_no_args, my_hook_test_dependency_no_args);
    REGISTER_GLOBAL_MOCK_HOOK(test_dependency_no_args, NULL);

    // act
    int result = test_dependency_no_args();

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
}

/* Tests_SRS_UMOCK_C_01_135: [ All parameters passed to the mock shall be passed down to the mock hook. ]*/
TEST_FUNCTION(REGISTER_GLOBAL_MOCK_HOOK_the_args_of_the_mock_get_passed_to_the_hook)
{
    // arrange
    REGISTER_GLOBAL_MOCK_HOOK(test_dependency_2_args, my_hook_test_dependency_2_args);

    // act
    (void)test_dependency_2_args(0x42, 0x43);

    // assert
    ASSERT_ARE_EQUAL(int, 0x42, arg_a);
    ASSERT_ARE_EQUAL(int, 0x43, arg_b);
}

/* Tests_SRS_UMOCK_C_01_104: [The REGISTER_GLOBAL_MOCK_HOOK shall register a mock hook to be called every time the mocked function is called by production code.]*/
TEST_FUNCTION(REGISTER_GLOBAL_MOCK_HOOK_with_a_function_that_returns_void_works)
{
    // arrange
    REGISTER_GLOBAL_MOCK_HOOK(test_dependency_void_return, my_hook_test_dependency_void_return);

    // act
    test_dependency_void_return();

    // assert
    ASSERT_ARE_EQUAL(int, 1, test_dependency_void_return_called);
}

/* REGISTER_GLOBAL_MOCK_RETURN */

/* Tests_SRS_UMOCK_C_01_108: [The REGISTER_GLOBAL_MOCK_RETURN shall register a return value to always be returned by a mock function.]*/
TEST_FUNCTION(REGISTER_GLOBAL_MOCK_RETURN_makes_a_subsequent_call_to_the_mock_return_the_value)
{
    // arrange
    REGISTER_GLOBAL_MOCK_RETURN(test_dependency_global_mock_return_test, 0x45);

    // act
    int result = test_dependency_global_mock_return_test();

    // assert
    ASSERT_ARE_EQUAL(int, 0x45, result);
}

/* Tests_SRS_UMOCK_C_01_109: [If there are multiple invocations of REGISTER_GLOBAL_MOCK_RETURN, the last one shall take effect over the previous ones.]*/
TEST_FUNCTION(REGISTER_GLOBAL_MOCK_RETURN_twice_only_makes_the_second_call_stick)
{
    // arrange
    REGISTER_GLOBAL_MOCK_RETURN(test_dependency_global_mock_return_test, 0x45);
    REGISTER_GLOBAL_MOCK_RETURN(test_dependency_global_mock_return_test, 0x46);

    // act
    int result = test_dependency_global_mock_return_test();

    // assert
    ASSERT_ARE_EQUAL(int, 0x46, result);
}

/* Tests_SRS_UMOCK_C_01_141: [ If any error occurs during REGISTER_GLOBAL_MOCK_RETURN, umock_c shall raise an error with the code UMOCK_C_ERROR. ]*/
TEST_FUNCTION(when_copy_fails_in_REGISTER_GLOBAL_MOCK_RETURN_then_on_error_is_triggered)
{
    // arrange
    TEST_STRUCT_COPY_FAILS test_struct = { 0x42 };
    test_dependency_global_mock_return_copy_fails_fail_call = 1;
    test_dependency_global_mock_return_copy_fails_call_count = 0;
    REGISTER_GLOBAL_MOCK_RETURN(test_dependency_global_mock_return_copy_fails, test_struct);

    // act
    (void)test_dependency_global_mock_return_copy_fails();

    // assert
    ASSERT_ARE_EQUAL(int, 1, test_on_umock_c_error_call_count);
    ASSERT_ARE_EQUAL(int, (int)UMOCK_C_ERROR, test_on_umock_c_error_calls[0].error_code);
}

/* REGISTER_GLOBAL_MOCK_FAIL_RETURN */

/* Tests_SRS_UMOCK_C_01_111: [The REGISTER_GLOBAL_MOCK_FAIL_RETURN shall register a fail return value to be returned by a mock function when marked as failed in the expected calls.]*/
TEST_FUNCTION(REGISTER_GLOBAL_MOCK_FAIL_RETURN_is_possible_and_does_not_affect_the_return_value)
{
    // arrange
    REGISTER_GLOBAL_MOCK_RETURN(test_dependency_global_mock_return_test, 0x42);
    REGISTER_GLOBAL_MOCK_FAIL_RETURN(test_dependency_global_mock_return_test, 0x45);

    // act
    int result = test_dependency_global_mock_return_test();

    // assert
    ASSERT_ARE_EQUAL(int, 0x42, result);
}

/* Tests_SRS_UMOCK_C_01_112: [If there are multiple invocations of REGISTER_GLOBAL_FAIL_MOCK_RETURN, the last one shall take effect over the previous ones.]*/
TEST_FUNCTION(Multiple_REGISTER_GLOBAL_MOCK_FAIL_RETURN_calls_are_possible)
{
    // arrange
    REGISTER_GLOBAL_MOCK_RETURN(test_dependency_global_mock_return_test, 0x42);
    REGISTER_GLOBAL_MOCK_FAIL_RETURN(test_dependency_global_mock_return_test, 0x45);
    REGISTER_GLOBAL_MOCK_FAIL_RETURN(test_dependency_global_mock_return_test, 0x46);

    // act
    int result = test_dependency_global_mock_return_test();

    // assert
    ASSERT_ARE_EQUAL(int, 0x42, result);
}

/* Tests_SRS_UMOCK_C_01_142: [ If any error occurs during REGISTER_GLOBAL_MOCK_FAIL_RETURN, umock_c shall raise an error with the code UMOCK_C_ERROR. ]*/
TEST_FUNCTION(When_copy_fails_in_REGISTER_GLOBAL_MOCK_FAIL_RETURN_then_on_error_is_triggered)
{
    // arrange
    REGISTER_GLOBAL_MOCK_RETURN(test_dependency_global_mock_return_test, 0x42);
    REGISTER_GLOBAL_MOCK_FAIL_RETURN(test_dependency_global_mock_return_test, 0x45);

    // act
    int result = test_dependency_global_mock_return_test();

    // assert
    ASSERT_ARE_EQUAL(int, 0x42, result);
}

/* REGISTER_GLOBAL_MOCK_RETURNS */

/* Tests_SRS_UMOCK_C_01_113: [The REGISTER_GLOBAL_MOCK_RETURNS shall register both a success and a fail return value associated with a mock function.]*/
TEST_FUNCTION(REGISTER_GLOBAL_MOCK_RETURNS_registers_the_return_value)
{
    // arrange
    REGISTER_GLOBAL_MOCK_RETURNS(test_dependency_global_mock_return_test, 0xAA, 0x43);

    // act
    int result = test_dependency_global_mock_return_test();

    // assert
    ASSERT_ARE_EQUAL(int, 0xAA, result);
}

/* Tests_SRS_UMOCK_C_01_114: [If there are multiple invocations of REGISTER_GLOBAL_MOCK_RETURNS, the last one shall take effect over the previous ones.]*/
TEST_FUNCTION(REGISTER_GLOBAL_MOCK_RETURNS_twice_makes_only_the_last_call_stick)
{
    // arrange
    REGISTER_GLOBAL_MOCK_RETURNS(test_dependency_global_mock_return_test, 0xAA, 0x43);
    REGISTER_GLOBAL_MOCK_RETURNS(test_dependency_global_mock_return_test, 0xAB, 0x44);

    // act
    int result = test_dependency_global_mock_return_test();

    // assert
    ASSERT_ARE_EQUAL(int, 0xAB, result);
}

/* Tests_SRS_UMOCK_C_01_143: [ If any error occurs during REGISTER_GLOBAL_MOCK_RETURNS, umock_c shall raise an error with the code UMOCK_C_ERROR. ]*/
TEST_FUNCTION(when_copy_fails_in_REGISTER_GLOBAL_MOCK_RETURNS_then_on_error_is_triggered)
{
    TEST_STRUCT_COPY_FAILS test_struct = { 0x42 };
    TEST_STRUCT_COPY_FAILS test_struct_fail = { 0x43 };
    test_dependency_global_mock_return_copy_fails_fail_call = 1;
    test_dependency_global_mock_return_copy_fails_call_count = 0;
    REGISTER_GLOBAL_MOCK_RETURNS(test_dependency_global_mock_return_copy_fails, test_struct, test_struct_fail);

    // act
    (void)test_dependency_global_mock_return_copy_fails();

    // assert
    ASSERT_ARE_EQUAL(int, 1, test_on_umock_c_error_call_count);
    ASSERT_ARE_EQUAL(int, (int)UMOCK_C_ERROR, test_on_umock_c_error_calls[0].error_code);
}

/* Tests_SRS_UMOCK_C_01_143: [ If any error occurs during REGISTER_GLOBAL_MOCK_RETURNS, umock_c shall raise an error with the code UMOCK_C_ERROR. ]*/
TEST_FUNCTION(when_copy_fails_for_the_second_call_in_REGISTER_GLOBAL_MOCK_RETURNS_then_on_error_is_triggered)
{
    TEST_STRUCT_COPY_FAILS test_struct = { 0x42 };
    TEST_STRUCT_COPY_FAILS test_struct_fail = { 0x43 };
    test_dependency_global_mock_return_copy_fails_fail_call = 2;
    test_dependency_global_mock_return_copy_fails_call_count = 0;

    REGISTER_GLOBAL_MOCK_RETURNS(test_dependency_global_mock_return_copy_fails, test_struct, test_struct_fail);

    // act
    (void)test_dependency_global_mock_return_copy_fails();

    // assert
    ASSERT_ARE_EQUAL(int, 1, test_on_umock_c_error_call_count);
    ASSERT_ARE_EQUAL(int, (int)UMOCK_C_ERROR, test_on_umock_c_error_calls[0].error_code);
}

/* Type names */

/* Tests_SRS_UMOCK_C_01_145: [ Since umock_c needs to maintain a list of registered types, the following rules shall be applied: ]*/
/* Tests_SRS_UMOCK_C_01_146: [ Each type shall be normalized to a form where all extra spaces are removed. ]*/
TEST_FUNCTION(spaces_are_stripped_from_typenames)
{
    // arrange
    STRICT_EXPECTED_CALL(test_dependency_type_with_space("b"));

    // act
    test_dependency_type_with_space("b");

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_actual_calls());
}

/* Supported types */

/* Tests_SRS_UMOCK_C_01_144: [ Out of the box umock_c shall support the following types through the header umocktypes_c.h: ]*/
/* Tests_SRS_UMOCK_C_01_028: [**char**] */
/* Tests_SRS_UMOCK_C_01_029 : [**unsigned char**] */
/* Tests_SRS_UMOCK_C_01_030 : [**short**] */
/* Tests_SRS_UMOCK_C_01_031 : [**unsigned short**] */
/* Tests_SRS_UMOCK_C_01_032 : [**int**] */
/* Tests_SRS_UMOCK_C_01_033 : [**unsigned int**] */
/* Tests_SRS_UMOCK_C_01_034 : [**long**] */
/* Tests_SRS_UMOCK_C_01_035 : [**unsigned long**] */
/* Tests_SRS_UMOCK_C_01_036 : [**long long**] */
/* Tests_SRS_UMOCK_C_01_037 : [**unsigned long long**] */
/* Tests_SRS_UMOCK_C_01_038 : [**float**] */
/* Tests_SRS_UMOCK_C_01_039 : [**double**] */
/* Tests_SRS_UMOCK_C_01_040 : [**long double**] */
/* Tests_SRS_UMOCK_C_01_041 : [**size_t**] */
TEST_FUNCTION(native_c_types_are_supported)
{
    // arrange
    STRICT_EXPECTED_CALL(test_dependency_all_types(-42, 42, /* char */
        -43, 43, /* short */
        -44, 44, /* int */
        -45, 45, /* long */
        -46, 46, /* long long */
        -42.42f,  /* float */
        4242.42, /* double */
        4242.42, /* long double */
        0x42 /* size_t*/
        ));

    // act
    test_dependency_all_types(-42, 42, /* char */
        -43, 43, /* short */
        -44, 44, /* int */
        -45, 45, /* long */
        -46, 46, /* long long */
        -42.42f,  /* float */
        4242.42, /* double */
        4242.42, /* long double */
        0x42 /* size_t*/
        );

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 0, test_on_umock_c_error_call_count);
}

/* Tests_SRS_UMOCK_C_01_148: [ If call comparison fails an error shall be indicated by calling the error callback with UMOCK_C_COMPARE_CALL_ERROR. ]*/
TEST_FUNCTION(when_a_type_is_not_supported_an_error_is_triggered)
{
    TEST_STRUCT_NOT_REGISTERED a = { 0 };

    // arrange
    STRICT_EXPECTED_CALL(test_dependency_type_not_registered(a));

    // act
    test_dependency_type_not_registered(a);

    // assert
    ASSERT_IS_NULL(umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(int, 1, test_on_umock_c_error_call_count);
}

END_TEST_SUITE(umock_c_integrationtests)
