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
    int result = umocktypes_are_equal_char(&left, &right);

    // assert
    ASSERT_ARE_EQUAL(int, 1, result);
}

/* Tests_SRS_UMOCKTYPES_C_01_009: [ If the values pochared to by left and right are different, umocktypes_are_equal_char shall return 0. ]*/
TEST_FUNCTION(umocktypes_are_equal_char_with_2_different_values_returns_0)
{
    // arrange
    char left = 0x42;
    char right = 0x43;

    // act
    int result = umocktypes_are_equal_char(&left, &right);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
}

/* Tests_SRS_UMOCKTYPES_C_01_007: [ If any of the arguments is NULL, umocktypes_are_equal_char shall return -1. ]*/
TEST_FUNCTION(umocktypes_are_equal_char_with_NULL_left_fails)
{
    // arrange
    char right = 0x43;

    // act
    int result = umocktypes_are_equal_char(NULL, &right);

    // assert
    ASSERT_ARE_EQUAL(int, -1, result);
}

/* Tests_SRS_UMOCKTYPES_C_01_007: [ If any of the arguments is NULL, umocktypes_are_equal_char shall return -1. ]*/
TEST_FUNCTION(umocktypes_are_equal_char_with_NULL_right_fails)
{
    // arrange
    char left = 0x42;

    // act
    int result = umocktypes_are_equal_char(&left, NULL);

    // assert
    ASSERT_ARE_EQUAL(int, -1, result);
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
    int result = umocktypes_copy_char(&destination, &source);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char, 0x42, destination);
}

/* Tests_SRS_UMOCKTYPES_C_01_012: [ If source or destination are NULL, umocktypes_copy_char shall return a non-zero value. ]*/
TEST_FUNCTION(umocktypes_copy_char_with_NULL_destination_fails)
{
    // arrange
    char source = 0x42;

    // act
    int result = umocktypes_copy_char(NULL, &source);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
}

/* Tests_SRS_UMOCKTYPES_C_01_012: [ If source or destination are NULL, umocktypes_copy_char shall return a non-zero value. ]*/
TEST_FUNCTION(umocktypes_copy_char_with_NULL_source_fails)
{
    // arrange
    char destination = 0;

    // act
    int result = umocktypes_copy_char(&destination, NULL);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
}

/* umocktypes_free_char */

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

/* umocktypes_stringify_unsignedchar */

/* Tests_SRS_UMOCKTYPES_C_01_014: [ umocktypes_stringify_unsignedchar shall return the string representation of value. ]*/
TEST_FUNCTION(umocktypes_stringify_unsignedchar_with_0_value)
{
    // arrange
    unsigned char input = 0;

    // act
    char* result = umocktypes_stringify_unsignedchar(&input);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "0", result);

    // cleanup
    free(result);
}

/* Tests_SRS_UMOCKTYPES_C_01_014: [ umocktypes_stringify_unsignedchar shall return the string representation of value. ]*/
TEST_FUNCTION(umocktypes_stringify_unsignedchar_with_positive_value)
{
    // arrange
    unsigned char input = 255;

    // act
    char* result = umocktypes_stringify_unsignedchar(&input);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "255", result);

    // cleanup
    free(result);
}

/* Tests_SRS_UMOCKTYPES_C_01_015: [ If value is NULL, umocktypes_stringify_unsignedchar shall return NULL. ]*/
TEST_FUNCTION(umocktypes_stringify_unsignedchar_with_NULL_fails)
{
    // arrange

    // act
    char* result = umocktypes_stringify_unsignedchar(NULL);

    // assert
    ASSERT_IS_NULL(result);
}

/* umocktypes_are_equal_unsignedchar */

/* Tests_SRS_UMOCKTYPES_C_01_018: [ umocktypes_are_equal_unsignedchar shall compare the 2 unsigned chars pounsigned chared to by left and right. ]*/
/* Tests_SRS_UMOCKTYPES_C_01_020: [ If the values pounsigned chared to by left and right are equal, umocktypes_are_equal_unsignedchar shall return 1. ]*/
TEST_FUNCTION(umocktypes_are_equal_unsignedchar_with_2_equal_values_returns_1)
{
    // arrange
    unsigned char left = 0x42;
    unsigned char right = 0x42;

    // act
    int result = umocktypes_are_equal_unsignedchar(&left, &right);

    // assert
    ASSERT_ARE_EQUAL(int, 1, result);
}

/* Tests_SRS_UMOCKTYPES_C_01_021: [ If the values pounsigned chared to by left and right are different, umocktypes_are_equal_unsignedchar shall return 0. ]*/
TEST_FUNCTION(umocktypes_are_equal_unsignedchar_with_2_different_values_returns_0)
{
    // arrange
    unsigned char left = 0x42;
    unsigned char right = 0x43;

    // act
    int result = umocktypes_are_equal_unsignedchar(&left, &right);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
}

/* Tests_SRS_UMOCKTYPES_C_01_019: [ If any of the arguments is NULL, umocktypes_are_equal_unsignedchar shall return -1. ]*/
TEST_FUNCTION(umocktypes_are_equal_unsignedchar_with_NULL_left_fails)
{
    // arrange
    unsigned char right = 0x43;

    // act
    int result = umocktypes_are_equal_unsignedchar(NULL, &right);

    // assert
    ASSERT_ARE_EQUAL(int, -1, result);
}

/* Tests_SRS_UMOCKTYPES_C_01_019: [ If any of the arguments is NULL, umocktypes_are_equal_unsignedchar shall return -1. ]*/
TEST_FUNCTION(umocktypes_are_equal_unsignedchar_with_NULL_right_fails)
{
    // arrange
    unsigned char left = 0x42;

    // act
    int result = umocktypes_are_equal_unsignedchar(&left, NULL);

    // assert
    ASSERT_ARE_EQUAL(int, -1, result);
}

/* umocktypes_copy_unsignedchar */

/* Tests_SRS_UMOCKTYPES_C_01_022: [ umocktypes_copy_unsignedchar shall copy the unsigned char value from source to destination. ]*/
/* Tests_SRS_UMOCKTYPES_C_01_023: [ On success umocktypes_copy_unsignedchar shall return 0. ]*/
TEST_FUNCTION(umocktypes_copy_unsignedchar_succeeds)
{
    // arrange
    unsigned char destination = 0;
    unsigned char source = 0x42;

    // act
    int result = umocktypes_copy_unsignedchar(&destination, &source);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(uint8_t, 0x42, destination);
}

/* Tests_SRS_UMOCKTYPES_C_01_024: [ If source or destination are NULL, umocktypes_copy_unsignedchar shall return a non-zero value. ]*/
TEST_FUNCTION(umocktypes_copy_unsignedchar_with_NULL_destination_fails)
{
    // arrange
    unsigned char source = 0x42;

    // act
    int result = umocktypes_copy_unsignedchar(NULL, &source);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
}

/* Tests_SRS_UMOCKTYPES_C_01_024: [ If source or destination are NULL, umocktypes_copy_unsignedchar shall return a non-zero value. ]*/
TEST_FUNCTION(umocktypes_copy_unsignedchar_with_NULL_source_fails)
{
    // arrange
    unsigned char destination = 0;

    // act
    int result = umocktypes_copy_unsignedchar(&destination, NULL);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
}

/* umocktypes_free_unsignedchar */

/* Tests_SRS_UMOCKTYPES_C_01_025: [ umocktypes_free_unsignedchar shall do nothing. ]*/
TEST_FUNCTION(umocktypes_free_unsignedchar_does_nothing)
{
    // arrange
    unsigned char value = 0;

    // act
    umocktypes_free_unsignedchar(&value);

    // assert
    // no explicit assert
}

/* umocktypes_stringify_short */

/* Tests_SRS_UMOCKTYPES_C_01_026: [ umocktypes_stringify_short shall return the string representation of value. ]*/
TEST_FUNCTION(umocktypes_stringify_short_with_0_value)
{
    // arrange
    short input = 0;

    // act
    char* result = umocktypes_stringify_short(&input);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "0", result);

    // cleanup
    free(result);
}

/* Tests_SRS_UMOCKTYPES_C_01_026: [ umocktypes_stringify_short shall return the string representation of value. ]*/
TEST_FUNCTION(umocktypes_stringify_short_with_negative_value)
{
    // arrange
    short input = -127-1;

    // act
    char* result = umocktypes_stringify_short(&input);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "-128", result);

    // cleanup
    free(result);
}

/* Tests_SRS_UMOCKTYPES_C_01_026: [ umocktypes_stringify_short shall return the string representation of value. ]*/
TEST_FUNCTION(umocktypes_stringify_short_with_positive_value)
{
    // arrange
    short input = 127;

    // act
    char* result = umocktypes_stringify_short(&input);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "127", result);

    // cleanup
    free(result);
}

/* Tests_SRS_UMOCKTYPES_C_01_027: [ If value is NULL, umocktypes_stringify_short shall return NULL. ]*/
TEST_FUNCTION(umocktypes_stringify_short_with_NULL_fails)
{
    // arrange

    // act
    char* result = umocktypes_stringify_short(NULL);

    // assert
    ASSERT_IS_NULL(result);
}

/* umocktypes_are_equal_short */

/* Tests_SRS_UMOCKTYPES_C_01_030: [ umocktypes_are_equal_short shall compare the 2 shorts poshorted to by left and right. ]*/
/* Tests_SRS_UMOCKTYPES_C_01_032: [ If the values poshorted to by left and right are equal, umocktypes_are_equal_short shall return 1. ]*/
TEST_FUNCTION(umocktypes_are_equal_short_with_2_equal_values_returns_1)
{
    // arrange
    short left = 0x42;
    short right = 0x42;

    // act
    int result = umocktypes_are_equal_short(&left, &right);

    // assert
    ASSERT_ARE_EQUAL(int, 1, result);
}

/* Tests_SRS_UMOCKTYPES_C_01_033: [ If the values poshorted to by left and right are different, umocktypes_are_equal_short shall return 0. ]*/
TEST_FUNCTION(umocktypes_are_equal_short_with_2_different_values_returns_0)
{
    // arrange
    short left = 0x42;
    short right = 0x43;

    // act
    int result = umocktypes_are_equal_short(&left, &right);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
}

/* Tests_SRS_UMOCKTYPES_C_01_031: [ If any of the arguments is NULL, umocktypes_are_equal_short shall return -1. ]*/
TEST_FUNCTION(umocktypes_are_equal_short_with_NULL_left_fails)
{
    // arrange
    short right = 0x43;

    // act
    int result = umocktypes_are_equal_short(NULL, &right);

    // assert
    ASSERT_ARE_EQUAL(int, -1, result);
}

/* Tests_SRS_UMOCKTYPES_C_01_031: [ If any of the arguments is NULL, umocktypes_are_equal_short shall return -1. ]*/
TEST_FUNCTION(umocktypes_are_equal_short_with_NULL_right_fails)
{
    // arrange
    short left = 0x42;

    // act
    int result = umocktypes_are_equal_short(&left, NULL);

    // assert
    ASSERT_ARE_EQUAL(int, -1, result);
}

/* umocktypes_copy_short */

/* Tests_SRS_UMOCKTYPES_C_01_034: [ umocktypes_copy_short shall copy the short value from source to destination. ]*/
/* Tests_SRS_UMOCKTYPES_C_01_035: [ On success umocktypes_copy_short shall return 0. ]*/
TEST_FUNCTION(umocktypes_copy_short_succeeds)
{
    // arrange
    short destination = 0;
    short source = 0x42;

    // act
    int result = umocktypes_copy_short(&destination, &source);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(short, 0x42, destination);
}

/* Tests_SRS_UMOCKTYPES_C_01_036: [ If source or destination are NULL, umocktypes_copy_short shall return a non-zero value. ]*/
TEST_FUNCTION(umocktypes_copy_short_with_NULL_destination_fails)
{
    // arrange
    short source = 0x42;

    // act
    int result = umocktypes_copy_short(NULL, &source);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
}

/* Tests_SRS_UMOCKTYPES_C_01_036: [ If source or destination are NULL, umocktypes_copy_short shall return a non-zero value. ]*/
TEST_FUNCTION(umocktypes_copy_short_with_NULL_source_fails)
{
    // arrange
    short destination = 0;

    // act
    int result = umocktypes_copy_short(&destination, NULL);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
}

/* umocktypes_free_short */

/* Tests_SRS_UMOCKTYPES_C_01_037: [ umocktypes_free_short shall do nothing. ]*/
TEST_FUNCTION(umocktypes_free_short_does_nothing)
{
    // arrange
    short value = 0;

    // act
    umocktypes_free_short(&value);

    // assert
    // no explicit assert
}

/* umocktypes_stringify_unsignedshort */

/* Tests_SRS_UMOCKTYPES_C_01_038: [ umocktypes_stringify_unsignedshort shall return the string representation of value. ]*/
TEST_FUNCTION(umocktypes_stringify_unsignedshort_with_0_value)
{
    // arrange
    unsigned short input = 0;

    // act
    char* result = umocktypes_stringify_unsignedshort(&input);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "0", result);

    // cleanup
    free(result);
}

/* Tests_SRS_UMOCKTYPES_C_01_038: [ umocktypes_stringify_unsignedshort shall return the string representation of value. ]*/
TEST_FUNCTION(umocktypes_stringify_unsignedshort_with_positive_value)
{
    // arrange
    unsigned short input = 255;

    // act
    char* result = umocktypes_stringify_unsignedshort(&input);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "255", result);

    // cleanup
    free(result);
}

/* Tests_SRS_UMOCKTYPES_C_01_039: [ If value is NULL, umocktypes_stringify_unsignedshort shall return NULL. ]*/
TEST_FUNCTION(umocktypes_stringify_unsignedshort_with_NULL_fails)
{
    // arrange

    // act
    char* result = umocktypes_stringify_unsignedshort(NULL);

    // assert
    ASSERT_IS_NULL(result);
}

/* umocktypes_are_equal_unsignedshort */

/* Tests_SRS_UMOCKTYPES_C_01_042: [ umocktypes_are_equal_unsignedshort shall compare the 2 unsigned shorts pounsigned shorted to by left and right. ]*/
/* Tests_SRS_UMOCKTYPES_C_01_044: [ If the values pounsigned shorted to by left and right are equal, umocktypes_are_equal_unsignedshort shall return 1. ]*/
TEST_FUNCTION(umocktypes_are_equal_unsignedshort_with_2_equal_values_returns_1)
{
    // arrange
    unsigned short left = 0x42;
    unsigned short right = 0x42;

    // act
    int result = umocktypes_are_equal_unsignedshort(&left, &right);

    // assert
    ASSERT_ARE_EQUAL(int, 1, result);
}

/* Tests_SRS_UMOCKTYPES_C_01_045: [ If the values pounsigned shorted to by left and right are different, umocktypes_are_equal_unsignedshort shall return 0. ]*/
TEST_FUNCTION(umocktypes_are_equal_unsignedshort_with_2_different_values_returns_0)
{
    // arrange
    unsigned short left = 0x42;
    unsigned short right = 0x43;

    // act
    int result = umocktypes_are_equal_unsignedshort(&left, &right);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
}

/* Tests_SRS_UMOCKTYPES_C_01_043: [ If any of the arguments is NULL, umocktypes_are_equal_unsignedshort shall return -1. ]*/
TEST_FUNCTION(umocktypes_are_equal_unsignedshort_with_NULL_left_fails)
{
    // arrange
    unsigned short right = 0x43;

    // act
    int result = umocktypes_are_equal_unsignedshort(NULL, &right);

    // assert
    ASSERT_ARE_EQUAL(int, -1, result);
}

/* Tests_SRS_UMOCKTYPES_C_01_043: [ If any of the arguments is NULL, umocktypes_are_equal_unsignedshort shall return -1. ]*/
TEST_FUNCTION(umocktypes_are_equal_unsignedshort_with_NULL_right_fails)
{
    // arrange
    unsigned short left = 0x42;

    // act
    int result = umocktypes_are_equal_unsignedshort(&left, NULL);

    // assert
    ASSERT_ARE_EQUAL(int, -1, result);
}

/* umocktypes_copy_unsignedshort */

/* Tests_SRS_UMOCKTYPES_C_01_046: [ umocktypes_copy_unsignedshort shall copy the unsigned short value from source to destination. ]*/
/* Tests_SRS_UMOCKTYPES_C_01_047: [ On success umocktypes_copy_unsignedshort shall return 0. ]*/
TEST_FUNCTION(umocktypes_copy_unsignedshort_succeeds)
{
    // arrange
    unsigned short destination = 0;
    unsigned short source = 0x42;

    // act
    int result = umocktypes_copy_unsignedshort(&destination, &source);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(int, 0x42, (int)destination);
}

/* Tests_SRS_UMOCKTYPES_C_01_048: [ If source or destination are NULL, umocktypes_copy_unsignedshort shall return a non-zero value. ]*/
TEST_FUNCTION(umocktypes_copy_unsignedshort_with_NULL_destination_fails)
{
    // arrange
    unsigned short source = 0x42;

    // act
    int result = umocktypes_copy_unsignedshort(NULL, &source);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
}

/* Tests_SRS_UMOCKTYPES_C_01_048: [ If source or destination are NULL, umocktypes_copy_unsignedshort shall return a non-zero value. ]*/
TEST_FUNCTION(umocktypes_copy_unsignedshort_with_NULL_source_fails)
{
    // arrange
    unsigned short destination = 0;

    // act
    int result = umocktypes_copy_unsignedshort(&destination, NULL);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
}

/* umocktypes_free_unsignedshort */

/* Tests_SRS_UMOCKTYPES_C_01_049: [ umocktypes_free_unsignedshort shall do nothing. ]*/
TEST_FUNCTION(umocktypes_free_unsignedshort_does_nothing)
{
    // arrange
    unsigned short value = 0;

    // act
    umocktypes_free_unsignedshort(&value);

    // assert
    // no explicit assert
}

/* umocktypes_stringify_int */

/* Tests_SRS_UMOCKTYPES_C_01_050: [ umocktypes_stringify_int shall return the string representation of value. ]*/
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

/* Tests_SRS_UMOCKTYPES_C_01_050: [ umocktypes_stringify_int shall return the string representation of value. ]*/
TEST_FUNCTION(umocktypes_stringify_int_with_negative_value)
{
    // arrange
    int input = -127-1;

    // act
    char* result = umocktypes_stringify_int(&input);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "-128", result);

    // cleanup
    free(result);
}

/* Tests_SRS_UMOCKTYPES_C_01_050: [ umocktypes_stringify_int shall return the string representation of value. ]*/
TEST_FUNCTION(umocktypes_stringify_int_with_positive_value)
{
    // arrange
    int input = 127;

    // act
    char* result = umocktypes_stringify_int(&input);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "127", result);

    // cleanup
    free(result);
}

/* Tests_SRS_UMOCKTYPES_C_01_051: [ If value is NULL, umocktypes_stringify_int shall return NULL. ]*/
TEST_FUNCTION(umocktypes_stringify_int_with_NULL_fails)
{
    // arrange

    // act
    char* result = umocktypes_stringify_int(NULL);

    // assert
    ASSERT_IS_NULL(result);
}

/* umocktypes_are_equal_int */

/* Tests_SRS_UMOCKTYPES_C_01_054: [ umocktypes_are_equal_int shall compare the 2 ints pointed to by left and right. ]*/
/* Tests_SRS_UMOCKTYPES_C_01_056: [ If the values pointed to by left and right are equal, umocktypes_are_equal_int shall return 1. ]*/
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

/* Tests_SRS_UMOCKTYPES_C_01_057: [ If the values pointed to by left and right are different, umocktypes_are_equal_int shall return 0. ]*/
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

/* Tests_SRS_UMOCKTYPES_C_01_055: [ If any of the arguments is NULL, umocktypes_are_equal_int shall return -1. ]*/
TEST_FUNCTION(umocktypes_are_equal_int_with_NULL_left_fails)
{
    // arrange
    int right = 0x43;

    // act
    int result = umocktypes_are_equal_int(NULL, &right);

    // assert
    ASSERT_ARE_EQUAL(int, -1, result);
}

/* Tests_SRS_UMOCKTYPES_C_01_055: [ If any of the arguments is NULL, umocktypes_are_equal_int shall return -1. ]*/
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

/* Tests_SRS_UMOCKTYPES_C_01_058: [ umocktypes_copy_int shall copy the int value from source to destination. ]*/
/* Tests_SRS_UMOCKTYPES_C_01_059: [ On success umocktypes_copy_int shall return 0. ]*/
TEST_FUNCTION(umocktypes_copy_int_succeeds)
{
    // arrange
    int destination = 0;
    int source = 0x42;

    // act
    int result = umocktypes_copy_int(&destination, &source);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(int, 0x42, (int)destination);
}

/* Tests_SRS_UMOCKTYPES_C_01_060: [ If source or destination are NULL, umocktypes_copy_int shall return a non-zero value. ]*/
TEST_FUNCTION(umocktypes_copy_int_with_NULL_destination_fails)
{
    // arrange
    int source = 0x42;

    // act
    int result = umocktypes_copy_int(NULL, &source);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
}

/* Tests_SRS_UMOCKTYPES_C_01_060: [ If source or destination are NULL, umocktypes_copy_int shall return a non-zero value. ]*/
TEST_FUNCTION(umocktypes_copy_int_with_NULL_source_fails)
{
    // arrange
    int destination = 0;

    // act
    int result = umocktypes_copy_int(&destination, NULL);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
}

/* umocktypes_free_int */

/* Tests_SRS_UMOCKTYPES_C_01_061: [ umocktypes_free_int shall do nothing. ]*/
TEST_FUNCTION(umocktypes_free_int_does_nothing)
{
    // arrange
    int value = 0;

    // act
    umocktypes_free_int(&value);

    // assert
    // no explicit assert
}

/* umocktypes_stringify_unsignedint */

/* Tests_SRS_UMOCKTYPES_C_01_062: [ umocktypes_stringify_unsignedint shall return the string representation of value. ]*/
TEST_FUNCTION(umocktypes_stringify_unsignedint_with_0_value)
{
    // arrange
    unsigned int input = 0;

    // act
    char* result = umocktypes_stringify_unsignedint(&input);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "0", result);

    // cleanup
    free(result);
}

/* Tests_SRS_UMOCKTYPES_C_01_062: [ umocktypes_stringify_unsignedint shall return the string representation of value. ]*/
TEST_FUNCTION(umocktypes_stringify_unsignedint_with_positive_value)
{
    // arrange
    unsigned int input = 127;

    // act
    char* result = umocktypes_stringify_unsignedint(&input);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "127", result);

    // cleanup
    free(result);
}

/* Tests_SRS_UMOCKTYPES_C_01_063: [ If value is NULL, umocktypes_stringify_unsignedint shall return NULL. ]*/
TEST_FUNCTION(umocktypes_stringify_unsignedint_with_NULL_fails)
{
    // arrange

    // act
    char* result = umocktypes_stringify_unsignedint(NULL);

    // assert
    ASSERT_IS_NULL(result);
}

/* umocktypes_are_equal_unsignedint */

/* Tests_SRS_UMOCKTYPES_C_01_066: [ umocktypes_are_equal_unsignedint shall compare the 2 unsigned ints pointed to by left and right. ]*/
/* Tests_SRS_UMOCKTYPES_C_01_068: [ If the values pointed to by left and right are equal, umocktypes_are_equal_unsignedint shall return 1. ]*/
TEST_FUNCTION(umocktypes_are_equal_unsignedint_with_2_equal_values_returns_1)
{
    // arrange
    unsigned int left = 0x42;
    unsigned int right = 0x42;

    // act
    int result = umocktypes_are_equal_unsignedint(&left, &right);

    // assert
    ASSERT_ARE_EQUAL(int, 1, result);
}

/* Tests_SRS_UMOCKTYPES_C_01_069: [ If the values pointed to by left and right are different, umocktypes_are_equal_unsignedint shall return 0. ]*/
TEST_FUNCTION(umocktypes_are_equal_unsignedint_with_2_different_values_returns_0)
{
    // arrange
    unsigned int left = 0x42;
    unsigned int right = 0x43;

    // act
    int result = umocktypes_are_equal_unsignedint(&left, &right);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
}

/* Tests_SRS_UMOCKTYPES_C_01_067: [ If any of the arguments is NULL, umocktypes_are_equal_unsignedint shall return -1. ]*/
TEST_FUNCTION(umocktypes_are_equal_unsignedint_with_NULL_left_fails)
{
    // arrange
    unsigned int right = 0x43;

    // act
    int result = umocktypes_are_equal_unsignedint(NULL, &right);

    // assert
    ASSERT_ARE_EQUAL(int, -1, result);
}

/* Tests_SRS_UMOCKTYPES_C_01_067: [ If any of the arguments is NULL, umocktypes_are_equal_unsignedint shall return -1. ]*/
TEST_FUNCTION(umocktypes_are_equal_unsignedint_with_NULL_right_fails)
{
    // arrange
    unsigned int left = 0x42;

    // act
    int result = umocktypes_are_equal_unsignedint(&left, NULL);

    // assert
    ASSERT_ARE_EQUAL(int, -1, result);
}

/* umocktypes_copy_unsignedint */

/* Tests_SRS_UMOCKTYPES_C_01_070: [ umocktypes_copy_unsignedint shall copy the unsigned int value from source to destination. ]*/
/* Tests_SRS_UMOCKTYPES_C_01_071: [ On success umocktypes_copy_unsignedint shall return 0. ]*/
TEST_FUNCTION(umocktypes_copy_unsignedint_succeeds)
{
    // arrange
    unsigned int destination = 0;
    unsigned int source = 0x42;

    // act
    int result = umocktypes_copy_unsignedint(&destination, &source);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(long, 0x42, (unsigned int)destination);
}

/* Tests_SRS_UMOCKTYPES_C_01_072: [ If source or destination are NULL, umocktypes_copy_unsignedint shall return a non-zero value. ]*/
TEST_FUNCTION(umocktypes_copy_unsignedint_with_NULL_destination_fails)
{
    // arrange
    unsigned int source = 0x42;

    // act
    int result = umocktypes_copy_unsignedint(NULL, &source);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
}

/* Tests_SRS_UMOCKTYPES_C_01_072: [ If source or destination are NULL, umocktypes_copy_unsignedint shall return a non-zero value. ]*/
TEST_FUNCTION(umocktypes_copy_unsignedint_with_NULL_source_fails)
{
    // arrange
    unsigned int destination = 0;

    // act
    int result = umocktypes_copy_unsignedint(&destination, NULL);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
}

/* umocktypes_free_unsignedint */

/* Tests_SRS_UMOCKTYPES_C_01_073: [ umocktypes_free_unsignedint shall do nothing. ]*/
TEST_FUNCTION(umocktypes_free_unsignedint_does_nothing)
{
    // arrange
    unsigned int value = 0;

    // act
    umocktypes_free_unsignedint(&value);

    // assert
    // no explicit assert
}

/* umocktypes_stringify_long */

/* Tests_SRS_UMOCKTYPES_C_01_074: [ umocktypes_stringify_long shall return the string representation of value. ]*/
TEST_FUNCTION(umocktypes_stringify_long_with_0_value)
{
    // arrange
    long input = 0;

    // act
    char* result = umocktypes_stringify_long(&input);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "0", result);

    // cleanup
    free(result);
}

/* Tests_SRS_UMOCKTYPES_C_01_074: [ umocktypes_stringify_long shall return the string representation of value. ]*/
TEST_FUNCTION(umocktypes_stringify_long_with_negative_value)
{
    // arrange
    long input = -127 - 1;

    // act
    char* result = umocktypes_stringify_long(&input);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "-128", result);

    // cleanup
    free(result);
}

/* Tests_SRS_UMOCKTYPES_C_01_074: [ umocktypes_stringify_long shall return the string representation of value. ]*/
TEST_FUNCTION(umocktypes_stringify_long_with_positive_value)
{
    // arrange
    long input = 127;

    // act
    char* result = umocktypes_stringify_long(&input);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "127", result);

    // cleanup
    free(result);
}

/* Tests_SRS_UMOCKTYPES_C_01_075: [ If value is NULL, umocktypes_stringify_long shall return NULL. ]*/
TEST_FUNCTION(umocktypes_stringify_long_with_NULL_fails)
{
    // arrange

    // act
    char* result = umocktypes_stringify_long(NULL);

    // assert
    ASSERT_IS_NULL(result);
}

/* umocktypes_are_equal_long */

/* Tests_SRS_UMOCKTYPES_C_01_078: [ umocktypes_are_equal_long shall compare the 2 longs pointed to by left and right. ]*/
/* Tests_SRS_UMOCKTYPES_C_01_080: [ If the values pointed to by left and right are equal, umocktypes_are_equal_long shall return 1. ]*/
TEST_FUNCTION(umocktypes_are_equal_long_with_2_equal_values_returns_1)
{
    // arrange
    long left = 0x42;
    long right = 0x42;

    // act
    int result = umocktypes_are_equal_long(&left, &right);

    // assert
    ASSERT_ARE_EQUAL(int, 1, result);
}

/* Tests_SRS_UMOCKTYPES_C_01_081: [ If the values pointed to by left and right are different, umocktypes_are_equal_long shall return 0. ]*/
TEST_FUNCTION(umocktypes_are_equal_long_with_2_different_values_returns_0)
{
    // arrange
    long left = 0x42;
    long right = 0x43;

    // act
    int result = umocktypes_are_equal_long(&left, &right);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
}

/* Tests_SRS_UMOCKTYPES_C_01_079: [ If any of the arguments is NULL, umocktypes_are_equal_long shall return -1. ]*/
TEST_FUNCTION(umocktypes_are_equal_long_with_NULL_left_fails)
{
    // arrange
    long right = 0x43;

    // act
    int result = umocktypes_are_equal_long(NULL, &right);

    // assert
    ASSERT_ARE_EQUAL(int, -1, result);
}

/* Tests_SRS_UMOCKTYPES_C_01_079: [ If any of the arguments is NULL, umocktypes_are_equal_long shall return -1. ]*/
TEST_FUNCTION(umocktypes_are_equal_long_with_NULL_right_fails)
{
    // arrange
    long left = 0x42;

    // act
    int result = umocktypes_are_equal_long(&left, NULL);

    // assert
    ASSERT_ARE_EQUAL(int, -1, result);
}

/* umocktypes_copy_long */

/* Tests_SRS_UMOCKTYPES_C_01_082: [ umocktypes_copy_long shall copy the long value from source to destination. ]*/
/* Tests_SRS_UMOCKTYPES_C_01_083: [ On success umocktypes_copy_long shall return 0. ]*/
TEST_FUNCTION(umocktypes_copy_long_succeeds)
{
    // arrange
    long destination = 0;
    long source = 0x42;

    // act
    int result = umocktypes_copy_long(&destination, &source);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(long, 0x42, (long)destination);
}

/* Tests_SRS_UMOCKTYPES_C_01_084: [ If source or destination are NULL, umocktypes_copy_long shall return a non-zero value. ]*/
TEST_FUNCTION(umocktypes_copy_long_with_NULL_destination_fails)
{
    // arrange
    long source = 0x42;

    // act
    int result = umocktypes_copy_long(NULL, &source);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
}

/* Tests_SRS_UMOCKTYPES_C_01_084: [ If source or destination are NULL, umocktypes_copy_long shall return a non-zero value. ]*/
TEST_FUNCTION(umocktypes_copy_long_with_NULL_source_fails)
{
    // arrange
    long destination = 0;

    // act
    int result = umocktypes_copy_long(&destination, NULL);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
}

/* umocktypes_free_long */

/* Tests_SRS_UMOCKTYPES_C_01_085: [ umocktypes_free_long shall do nothing. ]*/
TEST_FUNCTION(umocktypes_free_long_does_nothing)
{
    // arrange
    long value = 0;

    // act
    umocktypes_free_long(&value);

    // assert
    // no explicit assert
}

/* umocktypes_stringify_unsignedlong */

/* Tests_SRS_UMOCKTYPES_C_01_086: [ umocktypes_stringify_unsignedlong shall return the string representation of value. ]*/
TEST_FUNCTION(umocktypes_stringify_unsignedlong_with_0_value)
{
    // arrange
    unsigned long input = 0;

    // act
    char* result = umocktypes_stringify_unsignedlong(&input);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "0", result);

    // cleanup
    free(result);
}

/* Tests_SRS_UMOCKTYPES_C_01_086: [ umocktypes_stringify_unsignedlong shall return the string representation of value. ]*/
TEST_FUNCTION(umocktypes_stringify_unsignedlong_with_positive_value)
{
    // arrange
    unsigned long input = 127;

    // act
    char* result = umocktypes_stringify_unsignedlong(&input);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "127", result);

    // cleanup
    free(result);
}

/* Tests_SRS_UMOCKTYPES_C_01_087: [ If value is NULL, umocktypes_stringify_unsignedlong shall return NULL. ]*/
TEST_FUNCTION(umocktypes_stringify_unsignedlong_with_NULL_fails)
{
    // arrange

    // act
    char* result = umocktypes_stringify_unsignedlong(NULL);

    // assert
    ASSERT_IS_NULL(result);
}

/* umocktypes_are_equal_unsignedlong */

/* Tests_SRS_UMOCKTYPES_C_01_090: [ umocktypes_are_equal_unsignedlong shall compare the 2 unsigned longs pointed to by left and right. ]*/
/* Tests_SRS_UMOCKTYPES_C_01_092: [ If the values pointed to by left and right are equal, umocktypes_are_equal_unsignedlong shall return 1. ]*/
TEST_FUNCTION(umocktypes_are_equal_unsignedlong_with_2_equal_values_returns_1)
{
    // arrange
    unsigned long left = 0x42;
    unsigned long right = 0x42;

    // act
    int result = umocktypes_are_equal_unsignedlong(&left, &right);

    // assert
    ASSERT_ARE_EQUAL(int, 1, result);
}

/* Tests_SRS_UMOCKTYPES_C_01_093: [ If the values pointed to by left and right are different, umocktypes_are_equal_unsignedlong shall return 0. ]*/
TEST_FUNCTION(umocktypes_are_equal_unsignedlong_with_2_different_values_returns_0)
{
    // arrange
    unsigned long left = 0x42;
    unsigned long right = 0x43;

    // act
    int result = umocktypes_are_equal_unsignedlong(&left, &right);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
}

/* Tests_SRS_UMOCKTYPES_C_01_091: [ If any of the arguments is NULL, umocktypes_are_equal_unsignedlong shall return -1. ]*/
TEST_FUNCTION(umocktypes_are_equal_unsignedlong_with_NULL_left_fails)
{
    // arrange
    unsigned long right = 0x43;

    // act
    int result = umocktypes_are_equal_unsignedlong(NULL, &right);

    // assert
    ASSERT_ARE_EQUAL(int, -1, result);
}

/* Tests_SRS_UMOCKTYPES_C_01_091: [ If any of the arguments is NULL, umocktypes_are_equal_unsignedlong shall return -1. ]*/
TEST_FUNCTION(umocktypes_are_equal_unsignedlong_with_NULL_right_fails)
{
    // arrange
    unsigned long left = 0x42;

    // act
    int result = umocktypes_are_equal_unsignedlong(&left, NULL);

    // assert
    ASSERT_ARE_EQUAL(int, -1, result);
}

/* umocktypes_copy_unsignedlong */

/* Tests_SRS_UMOCKTYPES_C_01_094: [ umocktypes_copy_unsignedlong shall copy the unsigned long value from source to destination. ]*/
/* Tests_SRS_UMOCKTYPES_C_01_095: [ On success umocktypes_copy_unsignedlong shall return 0. ]*/
TEST_FUNCTION(umocktypes_copy_unsignedlong_succeeds)
{
    // arrange
    unsigned long destination = 0;
    unsigned long source = 0x42;

    // act
    int result = umocktypes_copy_unsignedlong(&destination, &source);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(long, 0x42, (long)destination);
}

/* Tests_SRS_UMOCKTYPES_C_01_096: [ If source or destination are NULL, umocktypes_copy_unsignedlong shall return a non-zero value. ]*/
TEST_FUNCTION(umocktypes_copy_unsignedlong_with_NULL_destination_fails)
{
    // arrange
    unsigned long source = 0x42;

    // act
    int result = umocktypes_copy_unsignedlong(NULL, &source);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
}

/* Tests_SRS_UMOCKTYPES_C_01_096: [ If source or destination are NULL, umocktypes_copy_unsignedlong shall return a non-zero value. ]*/
TEST_FUNCTION(umocktypes_copy_unsignedlong_with_NULL_source_fails)
{
    // arrange
    unsigned long destination = 0;

    // act
    int result = umocktypes_copy_unsignedlong(&destination, NULL);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
}

/* umocktypes_free_unsignedlong */

/* Tests_SRS_UMOCKTYPES_C_01_097: [ umocktypes_free_unsignedlong shall do nothing. ]*/
TEST_FUNCTION(umocktypes_free_unsignedlong_does_nothing)
{
    // arrange
    unsigned long value = 0;

    // act
    umocktypes_free_unsignedlong(&value);

    // assert
    // no explicit assert
}

/* umocktypes_stringify_longlong */

/* Tests_SRS_UMOCKTYPES_C_01_098: [ umocktypes_stringify_longlong shall return the string representation of value. ]*/
TEST_FUNCTION(umocktypes_stringify_longlong_with_0_value)
{
    // arrange
    long long input = 0;

    // act
    char* result = umocktypes_stringify_longlong(&input);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "0", result);

    // cleanup
    free(result);
}

/* Tests_SRS_UMOCKTYPES_C_01_098: [ umocktypes_stringify_longlong shall return the string representation of value. ]*/
TEST_FUNCTION(umocktypes_stringify_longlong_with_negative_value)
{
    // arrange
    long long input = -127 - 1;

    // act
    char* result = umocktypes_stringify_longlong(&input);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "-128", result);

    // cleanup
    free(result);
}

/* Tests_SRS_UMOCKTYPES_C_01_098: [ umocktypes_stringify_longlong shall return the string representation of value. ]*/
TEST_FUNCTION(umocktypes_stringify_longlong_with_positive_value)
{
    // arrange
    long long input = 127;

    // act
    char* result = umocktypes_stringify_longlong(&input);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "127", result);

    // cleanup
    free(result);
}

/* Tests_SRS_UMOCKTYPES_C_01_099: [ If value is NULL, umocktypes_stringify_longlong shall return NULL. ]*/
TEST_FUNCTION(umocktypes_stringify_longlong_with_NULL_fails)
{
    // arrange

    // act
    char* result = umocktypes_stringify_longlong(NULL);

    // assert
    ASSERT_IS_NULL(result);
}

/* umocktypes_are_equal_longlong */

/* Tests_SRS_UMOCKTYPES_C_01_102: [ umocktypes_are_equal_longlong shall compare the 2 long longs pointed to by left and right. ]*/
/* Tests_SRS_UMOCKTYPES_C_01_104: [ If the values pointed to by left and right are equal, umocktypes_are_equal_longlong shall return 1. ]*/
TEST_FUNCTION(umocktypes_are_equal_longlong_with_2_equal_values_returns_1)
{
    // arrange
    long long left = 0x42;
    long long right = 0x42;

    // act
    int result = umocktypes_are_equal_longlong(&left, &right);

    // assert
    ASSERT_ARE_EQUAL(int, 1, result);
}

/* Tests_SRS_UMOCKTYPES_C_01_105: [ If the values pointed to by left and right are different, umocktypes_are_equal_longlong shall return 0. ]*/
TEST_FUNCTION(umocktypes_are_equal_longlong_with_2_different_values_returns_0)
{
    // arrange
    long long left = 0x42;
    long long right = 0x43;

    // act
    int result = umocktypes_are_equal_longlong(&left, &right);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
}

/* Tests_SRS_UMOCKTYPES_C_01_103: [ If any of the arguments is NULL, umocktypes_are_equal_longlong shall return -1. ]*/
TEST_FUNCTION(umocktypes_are_equal_longlong_with_NULL_left_fails)
{
    // arrange
    long long right = 0x43;

    // act
    int result = umocktypes_are_equal_longlong(NULL, &right);

    // assert
    ASSERT_ARE_EQUAL(int, -1, result);
}

/* Tests_SRS_UMOCKTYPES_C_01_103: [ If any of the arguments is NULL, umocktypes_are_equal_longlong shall return -1. ]*/
TEST_FUNCTION(umocktypes_are_equal_longlong_with_NULL_right_fails)
{
    // arrange
    long long left = 0x42;

    // act
    int result = umocktypes_are_equal_longlong(&left, NULL);

    // assert
    ASSERT_ARE_EQUAL(int, -1, result);
}

/* umocktypes_copy_longlong */

/* Tests_SRS_UMOCKTYPES_C_01_106: [ umocktypes_copy_longlong shall copy the long long value from source to destination. ]*/
/* Tests_SRS_UMOCKTYPES_C_01_107: [ On success umocktypes_copy_longlong shall return 0. ]*/
TEST_FUNCTION(umocktypes_copy_longlong_succeeds)
{
    // arrange
    long long destination = 0;
    long long source = 0x42;

    // act
    int result = umocktypes_copy_longlong(&destination, &source);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(long, 0x42, (long)destination);
}

/* Tests_SRS_UMOCKTYPES_C_01_108: [ If source or destination are NULL, umocktypes_copy_longlong shall return a non-zero value. ]*/
TEST_FUNCTION(umocktypes_copy_longlong_with_NULL_destination_fails)
{
    // arrange
    long long source = 0x42;

    // act
    int result = umocktypes_copy_longlong(NULL, &source);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
}

/* Tests_SRS_UMOCKTYPES_C_01_108: [ If source or destination are NULL, umocktypes_copy_longlong shall return a non-zero value. ]*/
TEST_FUNCTION(umocktypes_copy_longlong_with_NULL_source_fails)
{
    // arrange
    long long destination = 0;

    // act
    int result = umocktypes_copy_longlong(&destination, NULL);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
}

/* umocktypes_free_longlong */

/* Tests_SRS_UMOCKTYPES_C_01_109: [ umocktypes_free_longlong shall do nothing. ]*/
TEST_FUNCTION(umocktypes_free_longlong_does_nothing)
{
    // arrange
    long long value = 0;

    // act
    umocktypes_free_longlong(&value);

    // assert
    // no explicit assert
}

/* umocktypes_stringify_unsignedlonglong */

/* Tests_SRS_UMOCKTYPES_C_01_110: [ umocktypes_stringify_unsignedlonglong shall return the string representation of value. ]*/
TEST_FUNCTION(umocktypes_stringify_unsignedlonglong_with_0_value)
{
    // arrange
    unsigned long long input = 0;

    // act
    char* result = umocktypes_stringify_unsignedlonglong(&input);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "0", result);

    // cleanup
    free(result);
}

/* Tests_SRS_UMOCKTYPES_C_01_110: [ umocktypes_stringify_unsignedlonglong shall return the string representation of value. ]*/
TEST_FUNCTION(umocktypes_stringify_unsignedlonglong_with_positive_value)
{
    // arrange
    unsigned long long input = 127;

    // act
    char* result = umocktypes_stringify_unsignedlonglong(&input);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "127", result);

    // cleanup
    free(result);
}

/* Tests_SRS_UMOCKTYPES_C_01_111: [ If value is NULL, umocktypes_stringify_unsignedlonglong shall return NULL. ]*/
TEST_FUNCTION(umocktypes_stringify_unsignedlonglong_with_NULL_fails)
{
    // arrange

    // act
    char* result = umocktypes_stringify_unsignedlonglong(NULL);

    // assert
    ASSERT_IS_NULL(result);
}

/* umocktypes_are_equal_unsignedlonglong */

/* Tests_SRS_UMOCKTYPES_C_01_114: [ umocktypes_are_equal_unsignedlonglong shall compare the 2 unsigned long longs pointed to by left and right. ]*/
/* Tests_SRS_UMOCKTYPES_C_01_116: [ If the values pointed to by left and right are equal, umocktypes_are_equal_unsignedlonglong shall return 1. ]*/
TEST_FUNCTION(umocktypes_are_equal_unsignedlonglong_with_2_equal_values_returns_1)
{
    // arrange
    unsigned long long left = 0x42;
    unsigned long long right = 0x42;

    // act
    int result = umocktypes_are_equal_unsignedlonglong(&left, &right);

    // assert
    ASSERT_ARE_EQUAL(int, 1, result);
}

/* Tests_SRS_UMOCKTYPES_C_01_117: [ If the values pointed to by left and right are different, umocktypes_are_equal_unsignedlonglong shall return 0. ]*/
TEST_FUNCTION(umocktypes_are_equal_unsignedlonglong_with_2_different_values_returns_0)
{
    // arrange
    unsigned long long left = 0x42;
    unsigned long long right = 0x43;

    // act
    int result = umocktypes_are_equal_unsignedlonglong(&left, &right);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
}

/* Tests_SRS_UMOCKTYPES_C_01_115: [ If any of the arguments is NULL, umocktypes_are_equal_unsignedlonglong shall return -1. ]*/
TEST_FUNCTION(umocktypes_are_equal_unsignedlonglong_with_NULL_left_fails)
{
    // arrange
    unsigned long long right = 0x43;

    // act
    int result = umocktypes_are_equal_unsignedlonglong(NULL, &right);

    // assert
    ASSERT_ARE_EQUAL(int, -1, result);
}

/* Tests_SRS_UMOCKTYPES_C_01_115: [ If any of the arguments is NULL, umocktypes_are_equal_unsignedlonglong shall return -1. ]*/
TEST_FUNCTION(umocktypes_are_equal_unsignedlonglong_with_NULL_right_fails)
{
    // arrange
    unsigned long long left = 0x42;

    // act
    int result = umocktypes_are_equal_unsignedlonglong(&left, NULL);

    // assert
    ASSERT_ARE_EQUAL(int, -1, result);
}

/* umocktypes_copy_unsignedlonglong */

/* Tests_SRS_UMOCKTYPES_C_01_118: [ umocktypes_copy_unsignedlonglong shall copy the unsigned long long value from source to destination. ]*/
/* Tests_SRS_UMOCKTYPES_C_01_119: [ On success umocktypes_copy_unsignedlonglong shall return 0. ]*/
TEST_FUNCTION(umocktypes_copy_unsignedlonglong_succeeds)
{
    // arrange
    unsigned long long destination = 0;
    unsigned long long source = 0x42;

    // act
    int result = umocktypes_copy_unsignedlonglong(&destination, &source);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(long, 0x42, (long)destination);
}

/* Tests_SRS_UMOCKTYPES_C_01_120: [ If source or destination are NULL, umocktypes_copy_unsignedlonglong shall return a non-zero value. ]*/
TEST_FUNCTION(umocktypes_copy_unsignedlonglong_with_NULL_destination_fails)
{
    // arrange
    unsigned long long source = 0x42;

    // act
    int result = umocktypes_copy_unsignedlonglong(NULL, &source);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
}

/* Tests_SRS_UMOCKTYPES_C_01_120: [ If source or destination are NULL, umocktypes_copy_unsignedlonglong shall return a non-zero value. ]*/
TEST_FUNCTION(umocktypes_copy_unsignedlonglong_with_NULL_source_fails)
{
    // arrange
    unsigned long long destination = 0;

    // act
    int result = umocktypes_copy_unsignedlonglong(&destination, NULL);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
}

/* umocktypes_free_unsignedlonglong */

/* Tests_SRS_UMOCKTYPES_C_01_121: [ umocktypes_free_unsignedlonglong shall do nothing. ]*/
TEST_FUNCTION(umocktypes_free_unsignedlonglong_does_nothing)
{
    // arrange
    unsigned long long value = 0;

    // act
    umocktypes_free_unsignedlonglong(&value);

    // assert
    // no explicit assert
}

/* umocktypes_stringify_float */

/* Tests_SRS_UMOCKTYPES_C_01_110: [ umocktypes_stringify_float shall return the string representation of value. ]*/
TEST_FUNCTION(umocktypes_stringify_float_with_0_0_value)
{
    // arrange
    float input = 0.0f;
    char expected_string[32];

    // act
    char* result = umocktypes_stringify_float(&input);

    // assert
    (void)sprintf(expected_string, "%f", input);
    ASSERT_ARE_EQUAL(char_ptr, expected_string, result);

    // cleanup
    free(result);
}

/* Tests_SRS_UMOCKTYPES_C_01_110: [ umocktypes_stringify_float shall return the string representation of value. ]*/
TEST_FUNCTION(umocktypes_stringify_float_with_negative_value)
{
    // arrange
    float input = -1.42f;
    char expected_string[32];

    // act
    char* result = umocktypes_stringify_float(&input);

    // assert
    (void)sprintf(expected_string, "%f", input);
    ASSERT_ARE_EQUAL(char_ptr, expected_string, result);

    // cleanup
    free(result);
}

/* Tests_SRS_UMOCKTYPES_C_01_110: [ umocktypes_stringify_float shall return the string representation of value. ]*/
TEST_FUNCTION(umocktypes_stringify_float_with_positive_value)
{
    // arrange
    float input = 2.42f;
    char expected_string[32];

    // act
    char* result = umocktypes_stringify_float(&input);

    // assert
    (void)sprintf(expected_string, "%f", input);
    ASSERT_ARE_EQUAL(char_ptr, expected_string, result);

    // cleanup
    free(result);
}

/* Tests_SRS_UMOCKTYPES_C_01_111: [ If value is NULL, umocktypes_stringify_float shall return NULL. ]*/
TEST_FUNCTION(umocktypes_stringify_float_with_NULL_fails)
{
    // arrange

    // act
    char* result = umocktypes_stringify_float(NULL);

    // assert
    ASSERT_IS_NULL(result);
}

/* umocktypes_are_equal_float */

/* Tests_SRS_UMOCKTYPES_C_01_114: [ umocktypes_are_equal_float shall compare the 2 floats pointed to by left and right. ]*/
/* Tests_SRS_UMOCKTYPES_C_01_116: [ If the values pointed to by left and right are equal, umocktypes_are_equal_float shall return 1. ]*/
TEST_FUNCTION(umocktypes_are_equal_float_with_2_equal_values_returns_1)
{
    // arrange
    float left = 0x42;
    float right = 0x42;

    // act
    int result = umocktypes_are_equal_float(&left, &right);

    // assert
    ASSERT_ARE_EQUAL(int, 1, result);
}

/* Tests_SRS_UMOCKTYPES_C_01_117: [ If the values pointed to by left and right are different, umocktypes_are_equal_float shall return 0. ]*/
TEST_FUNCTION(umocktypes_are_equal_float_with_2_different_values_returns_0)
{
    // arrange
    float left = 0x42;
    float right = 0x43;

    // act
    int result = umocktypes_are_equal_float(&left, &right);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
}

/* Tests_SRS_UMOCKTYPES_C_01_115: [ If any of the arguments is NULL, umocktypes_are_equal_float shall return -1. ]*/
TEST_FUNCTION(umocktypes_are_equal_float_with_NULL_left_fails)
{
    // arrange
    float right = 0x43;

    // act
    int result = umocktypes_are_equal_float(NULL, &right);

    // assert
    ASSERT_ARE_EQUAL(int, -1, result);
}

/* Tests_SRS_UMOCKTYPES_C_01_115: [ If any of the arguments is NULL, umocktypes_are_equal_float shall return -1. ]*/
TEST_FUNCTION(umocktypes_are_equal_float_with_NULL_right_fails)
{
    // arrange
    float left = 0x42;

    // act
    int result = umocktypes_are_equal_float(&left, NULL);

    // assert
    ASSERT_ARE_EQUAL(int, -1, result);
}

/* umocktypes_copy_float */

/* Tests_SRS_UMOCKTYPES_C_01_118: [ umocktypes_copy_float shall copy the float value from source to destination. ]*/
/* Tests_SRS_UMOCKTYPES_C_01_119: [ On success umocktypes_copy_float shall return 0. ]*/
TEST_FUNCTION(umocktypes_copy_float_succeeds)
{
    // arrange
    float destination = 0;
    float source = 0x42;

    // act
    int result = umocktypes_copy_float(&destination, &source);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(long, 0x42, (long)destination);
}

/* Tests_SRS_UMOCKTYPES_C_01_120: [ If source or destination are NULL, umocktypes_copy_float shall return a non-zero value. ]*/
TEST_FUNCTION(umocktypes_copy_float_with_NULL_destination_fails)
{
    // arrange
    float source = 0x42;

    // act
    int result = umocktypes_copy_float(NULL, &source);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
}

/* Tests_SRS_UMOCKTYPES_C_01_120: [ If source or destination are NULL, umocktypes_copy_float shall return a non-zero value. ]*/
TEST_FUNCTION(umocktypes_copy_float_with_NULL_source_fails)
{
    // arrange
    float destination = 0;

    // act
    int result = umocktypes_copy_float(&destination, NULL);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
}

/* umocktypes_free_float */

/* Tests_SRS_UMOCKTYPES_C_01_121: [ umocktypes_free_float shall do nothing. ]*/
TEST_FUNCTION(umocktypes_free_float_does_nothing)
{
    // arrange
    float value = 0;

    // act
    umocktypes_free_float(&value);

    // assert
    // no explicit assert
}

END_TEST_SUITE(umocktypes_c_unittests)
