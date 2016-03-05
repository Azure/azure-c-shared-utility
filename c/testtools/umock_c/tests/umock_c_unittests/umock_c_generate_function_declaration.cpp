// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdint.h>
#include "testrunnerswitcher.h"

#include "umock_c.h"

MOCKABLE_FUNCTION(int, test_generate_signature, int, a, double, b, char*, s);
MOCKABLE_FUNCTION(void, test_generate_signature_void_return, int, a);

int test_generate_signature(int a, double b, char* s)
{
    return 42;
}

void test_generate_signature_void_return(int a)
{
}

BEGIN_TEST_SUITE(umock_c_generate_function_declaration_unittests)

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

/* Tests_SRS_UMOCK_C_01_002: [The macro shall generate a function signature in case ENABLE_MOCKS is not defined.] */
TEST_FUNCTION(when_ENABLE_MOCKS_is_not_on_MOCKABLE_FUNCTION_generates_a_standard_function_declaration)
{
    // arrange

    // act
    int result = test_generate_signature(1, 0.42, "42");

    // assert
    ASSERT_ARE_EQUAL(int, 42, result);
}

/* Tests_SRS_UMOCK_C_01_002: [The macro shall generate a function signature in case ENABLE_MOCKS is not defined.] */
TEST_FUNCTION(when_ENABLE_MOCKS_is_not_on_MOCKABLE_FUNCTION_generates_a_standard_function_declaration_with_void_return)
{
    // arrange

    // act
    test_generate_signature_void_return(1);

    // assert
    // no explicit assert
}

END_TEST_SUITE(umock_c_generate_function_declaration_unittests)
