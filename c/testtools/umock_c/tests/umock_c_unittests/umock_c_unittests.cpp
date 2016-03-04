// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdint.h>
#include "testrunnerswitcher.h"

#define ENABLE_MOCKS

#include "umock_c.h"
#include "test_dependency.h"
#include "umockvalue_charptr.h"

char* umockvalue_stringify_TEST_STRUCT(const TEST_STRUCT* value)
{
    char* result;
    char temp_buffer[32];
    int sprintf_result = sprintf(temp_buffer, "{%d}", value->x);
    if (sprintf_result < 0)
    {
        result = NULL;
    }
    else
    {
        result = (char*)malloc(sprintf_result + 1);
        if (result != NULL)
        {
            (void)memcpy(result, temp_buffer, sprintf_result + 1);
        }
    }

    return result;
}

int umockvalue_are_equal_TEST_STRUCT(const TEST_STRUCT* left, const TEST_STRUCT* right)
{
    return (left->x == right->x);
}

int umockvalue_copy_TEST_STRUCT(TEST_STRUCT* destination, const TEST_STRUCT* source)
{
    destination->x = source->x;
    return 0;
}

void umockvalue_free_TEST_STRUCT(TEST_STRUCT* value)
{
}

int mock_test_dependency_no_args(void)
{
    return 42;
}

BEGIN_TEST_SUITE(umock_c_unittests)

TEST_SUITE_INITIALIZE(suite_init)
{
    umock_c_init(NULL);
    REGISTER_UMOCK_VALUE_TYPE(TEST_STRUCT, umockvalue_stringify_TEST_STRUCT, umockvalue_are_equal_TEST_STRUCT, umockvalue_copy_TEST_STRUCT, umockvalue_free_TEST_STRUCT);
    REGISTER_UMOCK_VALUE_TYPE(char*, umockvalue_stringify_charptr, umockvalue_are_equal_charptr, umockvalue_copy_charptr, umockvalue_free_charptr);
    REGISTER_GLOBAL_MOCK_RETURN_HOOK(test_dependency_no_args, mock_test_dependency_no_args);
    REGISTER_GLOBAL_MOCK_RETURN(test_dependency_1_arg, 42);
}

TEST_SUITE_CLEANUP(suite_cleanup)
{
    umock_c_deinit();
}

TEST_FUNCTION_INITIALIZE(test_function_init)
{
    (void)umock_c_reset_all_calls();
}

TEST_FUNCTION_CLEANUP(test_function_cleanup)
{
}

TEST_FUNCTION(STRICT_EXPECTED_CALL_without_an_actual_call_yields_a_missing_call)
{
    // arrange

    // act
    STRICT_EXPECTED_CALL(test_dependency_no_args());

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "[test_dependency_no_args()]", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_actual_calls());
}

TEST_FUNCTION(STRICT_EXPECTED_CALL_2_times_without_an_actual_call_yields_2_missing_calls)
{
    // arrange

    // act
    STRICT_EXPECTED_CALL(test_dependency_no_args());
    STRICT_EXPECTED_CALL(test_dependency_no_args());

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "[test_dependency_no_args()][test_dependency_no_args()]", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_actual_calls());
}

TEST_FUNCTION(STRICT_EXPECTED_CALL_2_times_and_only_one_actual_call_yields_a_missing_call)
{
    // arrange
    STRICT_EXPECTED_CALL(test_dependency_no_args());
    STRICT_EXPECTED_CALL(test_dependency_no_args());

    // act
    (void)test_dependency_no_args();

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "[test_dependency_no_args()]", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_actual_calls());
}

TEST_FUNCTION(actual_call_without_expected_call_yields_an_unexpected_call)
{
    // arrange

    // act
    (void)test_dependency_no_args();

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "[test_dependency_no_args()]", umock_c_get_actual_calls());
}

TEST_FUNCTION(STRICT_EXPECTED_CALL_with_int_arg_without_an_actual_call_yields_a_missing_call)
{
    // arrange

    // act
    STRICT_EXPECTED_CALL(test_dependency_1_arg(42));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "[test_dependency_1_arg(42)]", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_actual_calls());
}

TEST_FUNCTION(STRICT_EXPECTED_CALL_with_ignored_args_ignores_the_args)
{
    // arrange
    STRICT_EXPECTED_CALL(test_dependency_1_arg(42))
        .IgnoreAllArguments();

    // act
    (void)test_dependency_1_arg(43);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_actual_calls());
}

TEST_FUNCTION(when_args_do_not_match_a_call_mismatch_occurs)
{
    // arrange
    STRICT_EXPECTED_CALL(test_dependency_1_arg(42));

    // act
    (void)test_dependency_1_arg(43);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "[test_dependency_1_arg(42)]", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "[test_dependency_1_arg(43)]", umock_c_get_actual_calls());
}

TEST_FUNCTION(SetReturn_injects_a_return_value)
{
    // arrange
    STRICT_EXPECTED_CALL(test_dependency_no_args())
        .SetReturn(44);

    // act
    int result = test_dependency_no_args();

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(int, 44, result);
}

TEST_FUNCTION(SetReturn_can_have_IgnoreAllArguments_chained)
{
    // arrange
    STRICT_EXPECTED_CALL(test_dependency_1_arg(42))
        .SetReturn(44)
        .IgnoreAllArguments();

    // act
    int result = test_dependency_1_arg(43);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(int, 44, result);
}

TEST_FUNCTION(ValidateAllArguments_validates_the_arguments)
{
    // arrange
    EXPECTED_CALL(test_dependency_1_arg(42))
        .ValidateAllArguments();

    // act
    (void)test_dependency_1_arg(43);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "[test_dependency_1_arg(42)]", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "[test_dependency_1_arg(43)]", umock_c_get_actual_calls());
}

TEST_FUNCTION(STRICT_EXPECTED_CALL_with_2_arguments_verifies_both_arguments)
{
    // arrange
    STRICT_EXPECTED_CALL(test_dependency_2_args(42, 1));

    // act
    (void)test_dependency_2_args(42, 2);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "[test_dependency_2_args(42,1)]", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "[test_dependency_2_args(42,2)]", umock_c_get_actual_calls());
}

TEST_FUNCTION(STRICT_EXPECTED_CALL_with_1_struct_argument_verifies_structure_content)
{
    // arrange
    TEST_STRUCT expected_struct = { 42 };
    TEST_STRUCT actual_struct = { 43 };
    STRICT_EXPECTED_CALL(test_dependency_struct_arg(expected_struct));

    // act
    (void)test_dependency_struct_arg(actual_struct);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "[test_dependency_struct_arg({42})]", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "[test_dependency_struct_arg({43})]", umock_c_get_actual_calls());
}

TEST_FUNCTION(STRICT_EXPECTED_CALL_with_char_star_as_argument)
{
    // arrange
    STRICT_EXPECTED_CALL(test_dependency_char_star_arg("a"));

    // act
    (void)test_dependency_char_star_arg("b");

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "[test_dependency_char_star_arg(\"a\")]", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "[test_dependency_char_star_arg(\"b\")]", umock_c_get_actual_calls());
}

END_TEST_SUITE(umock_c_unittests)
