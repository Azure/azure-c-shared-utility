// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdint.h>
#include <stdlib.h>
#include "testrunnerswitcher.h"
#include "umocktypes.h"
#include "umocktypes_c.h"

/* TODO: 
- test malloc failures
- serialize tests
*/

int umocktypes_register_type(const char* type, UMOCKTYPE_STRINGIFY_FUNC stringify_func, UMOCKTYPE_ARE_EQUAL_FUNC are_equal_func, UMOCKTYPE_COPY_FUNC copy_func, UMOCKTYPE_FREE_FUNC free_func)
{
    return 0;
}

BEGIN_TEST_SUITE(umocktypes_c_unittests)

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
}

/* umocktypes_stringify_char */

/* Tests_SRS_UMOCKTYPES_C_01_002: [ umocktypes_stringify_char shall return the string representation of value. ]*/
TEST_FUNCTION(umocktypes_stringify_char_with_0_value)
{
    // arrange
    char input = 0;

    // act
    char* result = umocktypes_stringify_char(&input);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "0", result);

    // cleanup
    free(result);
}

/* Tests_SRS_UMOCKTYPES_C_01_002: [ umocktypes_stringify_char shall return the string representation of value. ]*/
TEST_FUNCTION(umocktypes_stringify_char_with_min_value)
{
    // arrange
    char input = -127 - 1;

    // act
    char* result = umocktypes_stringify_char(&input);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "-128", result);

    // cleanup
    free(result);
}

/* Tests_SRS_UMOCKTYPES_C_01_002: [ umocktypes_stringify_char shall return the string representation of value. ]*/
TEST_FUNCTION(umocktypes_stringify_char_with_max_value)
{
    // arrange
    char input = 127;

    // act
    char* result = umocktypes_stringify_char(&input);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "127", result);

    // cleanup
    free(result);
}

/* Tests_SRS_UMOCKTYPES_C_01_003: [ If value is NULL, umocktypes_stringify_char shall return NULL. ]*/
TEST_FUNCTION(umocktypes_stringify_char_with_NULL_fails)
{
    // arrange

    // act
    char* result = umocktypes_stringify_char(NULL);

    // assert
    ASSERT_IS_NULL(result);
}

/* umocktypes_are_equal_char */

/* Tests_SRS_UMOCKTYPES_C_01_006: [ umocktypes_are_equal_char shall compare the 2 chars pochared to by left and right. ]*/
/* Tests_SRS_UMOCKTYPES_C_01_008: [ If the values pochared to by left and right are equal, umocktypes_are_equal_char shall return 1. ]*/
TEST_FUNCTION(umocktypes_are_equal_char_with_2_equal_values_returns_1)
{
    // arrange
    char left = 0x42;
    char right = 0x42;

    // act
    char result = umocktypes_are_equal_char(&left, &right);

    // assert
    ASSERT_ARE_EQUAL(char, 1, result);
}

/* Tests_SRS_UMOCKTYPES_C_01_009: [ If the values pochared to by left and right are different, umocktypes_are_equal_char shall return 0. ]*/
TEST_FUNCTION(umocktypes_are_equal_char_with_2_different_values_returns_0)
{
    // arrange
    char left = 0x42;
    char right = 0x43;

    // act
    char result = umocktypes_are_equal_char(&left, &right);

    // assert
    ASSERT_ARE_EQUAL(char, 0, result);
}

/* Tests_SRS_UMOCKTYPES_C_01_007: [ If any of the arguments is NULL, umocktypes_are_equal_char shall return -1. ]*/
TEST_FUNCTION(umocktypes_are_equal_char_with_NULL_left_fails)
{
    // arrange
    char right = 0x43;

    // act
    char result = umocktypes_are_equal_char(NULL, &right);

    // assert
    ASSERT_ARE_EQUAL(char, -1, result);
}

/* Tests_SRS_UMOCKTYPES_C_01_007: [ If any of the arguments is NULL, umocktypes_are_equal_char shall return -1. ]*/
TEST_FUNCTION(umocktypes_are_equal_char_with_NULL_right_fails)
{
    // arrange
    char left = 0x42;

    // act
    char result = umocktypes_are_equal_char(&left, NULL);

    // assert
    ASSERT_ARE_EQUAL(char, -1, result);
}

/* umocktypes_copy_char */

/* Tests_SRS_UMOCKTYPES_C_01_010: [ umocktypes_copy_char shall copy the char value from source to destination. ]*/
/* Tests_SRS_UMOCKTYPES_C_01_011: [ On success umocktypes_copy_char shall return 0. ]*/
TEST_FUNCTION(umocktypes_copy_char_succeeds)
{
    // arrange
    char destination = 0;
    char source = 0x42;

    // act
    char result = umocktypes_copy_char(&destination, &source);

    // assert
    ASSERT_ARE_EQUAL(char, 0, result);
    ASSERT_ARE_EQUAL(char, 0x42, destination);
}

/* Tests_SRS_UMOCKTYPES_C_01_012: [ If source or destination are NULL, umocktypes_copy_char shall return a non-zero value. ]*/
TEST_FUNCTION(umocktypes_copy_char_with_NULL_destination_fails)
{
    // arrange
    char source = 0x42;

    // act
    char result = umocktypes_copy_char(NULL, &source);

    // assert
    ASSERT_ARE_NOT_EQUAL(char, 0, result);
}

/* Tests_SRS_UMOCKTYPES_C_01_012: [ If source or destination are NULL, umocktypes_copy_char shall return a non-zero value. ]*/
TEST_FUNCTION(umocktypes_copy_with_NULL_source_fails)
{
    // arrange
    char destination = 0;

    // act
    char result = umocktypes_copy_char(&destination, NULL);

    // assert
    ASSERT_ARE_NOT_EQUAL(char, 0, result);
}

/* Tests_SRS_UMOCKTYPES_C_01_013: [ umocktypes_free_char shall do nothing. ]*/
TEST_FUNCTION(umocktypes_free_char_does_nothing)
{
    // arrange
    char value = 0;

    // act
    umocktypes_free_char(&value);

    // assert
    // no explicit assert
}

/* umocktypes_stringify_int */

/* Tests_SRS_UMOCKTYPES_C_01_002: [ umocktypes\_stringify\_{type} shall return the string representation of value. ]*/
TEST_FUNCTION(umocktypes_stringify_int_with_0_value)
{
    // arrange
    int input = 0;

    // act
    char* result = umocktypes_stringify_int(&input);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "0", result);

    // cleanup
    free(result);
}

/* Tests_SRS_UMOCKTYPES_C_01_002: [ umocktypes_stringify_int shall return the string representation of value. ]*/
TEST_FUNCTION(umocktypes_stringify_int_with_min_value)
{
    // arrange
    int input = -2147483647 - 1;

    // act
    char* result = umocktypes_stringify_int(&input);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "-2147483648", result);

    // cleanup
    free(result);
}

/* Tests_SRS_UMOCKTYPES_C_01_002: [ umocktypes_stringify_int shall return the string representation of value. ]*/
TEST_FUNCTION(umocktypes_stringify_int_with_max_value)
{
    // arrange
    int input = 2147483647;

    // act
    char* result = umocktypes_stringify_int(&input);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "2147483647", result);

    // cleanup
    free(result);
}

/* Tests_SRS_UMOCKTYPES_C_01_003: [ If value is NULL, umocktypes_stringify_int shall return NULL. ]*/
TEST_FUNCTION(umocktypes_stringify_int_with_NULL_fails)
{
    // arrange

    // act
    char* result = umocktypes_stringify_int(NULL);

    // assert
    ASSERT_IS_NULL(result);
}

/* umocktypes_are_equal_int */

/* Tests_SRS_UMOCKTYPES_C_01_006: [ umocktypes_are_equal_int shall compare the 2 ints pointed to by left and right. ]*/
/* Tests_SRS_UMOCKTYPES_C_01_008: [ If the values pointed to by left and right are equal, umocktypes_are_equal_int shall return 1. ]*/
TEST_FUNCTION(umocktypes_are_equal_int_with_2_equal_values_returns_1)
{
    // arrange
    int left = 0x42;
    int right = 0x42;

    // act
    int result = umocktypes_are_equal_int(&left, &right);

    // assert
    ASSERT_ARE_EQUAL(int, 1, result);
}

/* Tests_SRS_UMOCKTYPES_C_01_009: [ If the values pointed to by left and right are different, umocktypes_are_equal_int shall return 0. ]*/
TEST_FUNCTION(umocktypes_are_equal_int_with_2_different_values_returns_0)
{
    // arrange
    int left = 0x42;
    int right = 0x43;

    // act
    int result = umocktypes_are_equal_int(&left, &right);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
}

/* Tests_SRS_UMOCKTYPES_C_01_007: [ If any of the arguments is NULL, umocktypes_are_equal_int shall return -1. ]*/
TEST_FUNCTION(umocktypes_are_equal_int_with_NULL_left_fails)
{
    // arrange
    int right = 0x43;

    // act
    int result = umocktypes_are_equal_int(NULL, &right);

    // assert
    ASSERT_ARE_EQUAL(int, -1, result);
}

/* Tests_SRS_UMOCKTYPES_C_01_007: [ If any of the arguments is NULL, umocktypes_are_equal_int shall return -1. ]*/
TEST_FUNCTION(umocktypes_are_equal_int_with_NULL_right_fails)
{
    // arrange
    int left = 0x42;

    // act
    int result = umocktypes_are_equal_int(&left, NULL);

    // assert
    ASSERT_ARE_EQUAL(int, -1, result);
}

/* umocktypes_copy_int */

/* Tests_SRS_UMOCKTYPES_C_01_010: [ umocktypes_copy_int shall copy the int value from source to destination. ]*/
/* Tests_SRS_UMOCKTYPES_C_01_011: [ On success umocktypes_copy_int shall return 0. ]*/
TEST_FUNCTION(umocktypes_copy_int_succeeds)
{
    // arrange
    int destination = 0;
    int source = 0x42;

    // act
    int result = umocktypes_copy_int(&destination, &source);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(int, 0x42, destination);
}

/* Tests_SRS_UMOCKTYPES_C_01_012: [ If source or destination are NULL, umocktypes_copy_int shall return a non-zero value. ]*/
TEST_FUNCTION(umocktypes_copy_with_NULL_destination_fails)
{
    // arrange
    int source = 0x42;

    // act
    int result = umocktypes_copy_int(NULL, &source);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
}

/* Tests_SRS_UMOCKTYPES_C_01_012: [ If source or destination are NULL, umocktypes_copy_int shall return a non-zero value. ]*/
TEST_FUNCTION(umocktypes_copy_int_with_NULL_source_fails)
{
    // arrange
    int destination = 0;

    // act
    int result = umocktypes_copy_int(&destination, NULL);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
}

/* Tests_SRS_UMOCKTYPES_C_01_013: [ umocktypes_free_int shall do nothing. ]*/
TEST_FUNCTION(umocktypes_free_int_does_nothing)
{
    // arrange
    int value = 0;

    // act
    umocktypes_free_int(&value);

    // assert
    // no explicit assert
}

END_TEST_SUITE(umocktypes_c_unittests)
