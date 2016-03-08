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
    char* input = "test42";

    // act
    char* result = umockvalue_stringify_charptr((const char**)&input);

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

END_TEST_SUITE(umockvalue_charptr_unittests)
