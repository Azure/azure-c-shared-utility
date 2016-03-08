// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdint.h>
#include <stdlib.h>
#include "testrunnerswitcher.h"
#include "umockvalue_charptr.h"

/* TODO: test malloc failures */
/* TODO: serialize tests */
/* TODO: test for registering types */

BEGIN_TEST_SUITE(umockvalue_charptr_unittests)

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

/* umockvalue_stringify_charptr */

/* Tests_SRS_UMOCKVALUE_CHARPTR_01_002: [ umockvalue_stringify_charptr shall return a string containing the string representation of value, enclosed by quotes (\"value\"). ] */
TEST_FUNCTION(umockvalue_stringify_charptr_with_an_empty_string_returns_2_quotes)
{
    // arrange
    char* input = "";

    // act
    char* result = umockvalue_stringify_charptr((const char**)&input);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "\"\"", result);

    // cleanup
    free(result);
}

/* Tests_SRS_UMOCKVALUE_CHARPTR_01_002: [ umockvalue_stringify_charptr shall return a string containing the string representation of value, enclosed by quotes (\"value\"). ] */
TEST_FUNCTION(umockvalue_stringify_charptr_with_a_non_empty_string_returns_the_string_surrounded_by_quotes)
{
    // arrange
    const char* input = "test42";

    // act
    char* result = umockvalue_stringify_charptr(&input);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "\"test42\"", result);

    // cleanup
    free(result);
}

/* Tests_SRS_UMOCKVALUE_CHARPTR_01_004: [ If value is NULL, umockvalue_stringify_charptr shall return NULL. ]*/
TEST_FUNCTION(umockvalue_stringify_charptr_with_NULL_argument_returns_NULL)
{
    // arrange

    // act
    char* result = umockvalue_stringify_charptr(NULL);

    // assert
    ASSERT_IS_NULL(result);
}

/* umockvalue_are_equal_charptr */

/* Tests_SRS_UMOCKVALUE_CHARPTR_01_005: [ umockvalue_are_equal_charptr shall compare the 2 strings pointed to by left and right. ] */
/* Tests_SRS_UMOCKVALUE_CHARPTR_01_007: [ If left or right are equal, umockvalue_are_equal_charptr shall return 1. ]*/
TEST_FUNCTION(umockvalue_are_equal_charptr_with_same_pointer_returns_1)
{
    // arrange
    const char* input = "test42";

    // act
    int result = umockvalue_are_equal_charptr(&input, &input);

    // assert
    ASSERT_ARE_EQUAL(int, 1, result);
}

/* Tests_SRS_UMOCKVALUE_CHARPTR_01_005: [ umockvalue_are_equal_charptr shall compare the 2 strings pointed to by left and right. ] */
/* Tests_SRS_UMOCKVALUE_CHARPTR_01_007: [ If left or right are equal, umockvalue_are_equal_charptr shall return 1. ]*/
TEST_FUNCTION(umockvalue_are_equal_charptr_with_same_NULL_pointer_returns_1)
{
    // arrange

    // act
    int result = umockvalue_are_equal_charptr(NULL, NULL);

    // assert
    ASSERT_ARE_EQUAL(int, 1, result);
}

/* Tests_SRS_UMOCKVALUE_CHARPTR_01_008: [ If only one of the left and right argument is NULL, umockvalue_are_equal_charptr shall return 0. ] */
TEST_FUNCTION(umockvalue_are_equal_charptr_with_left_NULL_returns_0)
{
    // arrange
    const char* input = "test42";

    // act
    int result = umockvalue_are_equal_charptr(NULL, &input);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
}

/* Tests_SRS_UMOCKVALUE_CHARPTR_01_008: [ If only one of the left and right argument is NULL, umockvalue_are_equal_charptr shall return 0. ] */
TEST_FUNCTION(umockvalue_are_equal_charptr_with_right_NULL_returns_0)
{
    // arrange
    const char* input = "test42";

    // act
    int result = umockvalue_are_equal_charptr(&input, NULL);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
}

/* Tests_SRS_UMOCKVALUE_CHARPTR_01_009: [ If the string pointed to by left is equal to the string pointed to by right, umockvalue_are_equal_charptr shall return 1. ]*/
TEST_FUNCTION(umockvalue_are_equal_with_string_being_the_same_returns_1)
{
    // arrange
    char* input1 = (char*)malloc(7);
    char* input2 = (char*)malloc(7);
    (void)strcpy(input1, "test42");
    (void)strcpy(input2, "test42");

    // act
    int result = umockvalue_are_equal_charptr((const char**)&input1, (const char**)&input2);

    // assert
    ASSERT_ARE_EQUAL(int, 1, result);

    // cleanup
    free(input1);
    free(input2);
}

/* Tests_SRS_UMOCKVALUE_CHARPTR_01_010: [ If the string pointed to by left is different than the string pointed to by right, umockvalue_are_equal_charptr shall return 0. ]*/
TEST_FUNCTION(umockvalue_are_equal_with_string_being_different_returns_0)
{
    // arrange
    char* input1 = (char*)malloc(7);
    char* input2 = (char*)malloc(7);
    (void)strcpy(input1, "test42");
    (void)strcpy(input2, "test43");

    // act
    int result = umockvalue_are_equal_charptr((const char**)&input1, (const char**)&input2);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);

    // cleanup
    free(input1);
    free(input2);
}

/* Tests_SRS_UMOCKVALUE_CHARPTR_01_006: [ The comparison shall be case sensitive. ]*/
TEST_FUNCTION(umockvalue_are_equal_with_string_being_different_in_case_returns_0)
{
    // arrange
    char* input1 = (char*)malloc(5);
    char* input2 = (char*)malloc(5);
    (void)strcpy(input1, "Test");
    (void)strcpy(input2, "test");

    // act
    int result = umockvalue_are_equal_charptr((const char**)&input1, (const char**)&input2);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);

    // cleanup
    free(input1);
    free(input2);
}

/* umockvalue_copy_charptr */

/* Tests_SRS_UMOCKVALUE_CHARPTR_01_011: [ umockvalue_copy_charptr shall allocate a new sequence of chars by using malloc. ]*/
/* Tests_SRS_UMOCKVALUE_CHARPTR_01_012: [ The number of bytes allocated shall accomodate the string pointed to by source. ]*/
/* Tests_SRS_UMOCKVALUE_CHARPTR_01_016: [ On success umockvalue_copy_charptr shall return 0. ]*/
/* Tests_SRS_UMOCKVALUE_CHARPTR_01_014: [ umockvalue_copy_charptr shall copy the string pointed to by source to the newly allocated memory. ]*/
/* Tests_SRS_UMOCKVALUE_CHARPTR_01_015: [ The newly allocated string shall be returned in the destination argument. ]*/
TEST_FUNCTION(umockvalue_copy_charptr_copies_an_empty_string)
{
    // arrange
    const char* source = "";
    char* destination = "a";

    // act
    int result = umockvalue_copy_charptr(&destination, &source);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, "", destination);

    // cleanup
    free(destination);
}

/* Tests_SRS_UMOCKVALUE_CHARPTR_01_013: [ If source or destination are NULL, umockvalue_copy_charptr shall return a non-zero value. ]*/
TEST_FUNCTION(umockvalue_copy_charptr_with_NULL_destination_fails)
{
    // arrange
    const char* source = "42";

    // act
    int result = umockvalue_copy_charptr(NULL, &source);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
}

/* Tests_SRS_UMOCKVALUE_CHARPTR_01_013: [ If source or destination are NULL, umockvalue_copy_charptr shall return a non-zero value. ]*/
TEST_FUNCTION(umockvalue_copy_charptr_with_NULL_source_fails)
{
    // arrange
    char* destination = "a";

    // act
    int result = umockvalue_copy_charptr(&destination, NULL);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
}

END_TEST_SUITE(umockvalue_charptr_unittests)
