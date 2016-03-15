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

/* umocktypes_stringify_int */

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

END_TEST_SUITE(umocktypes_c_unittests)
