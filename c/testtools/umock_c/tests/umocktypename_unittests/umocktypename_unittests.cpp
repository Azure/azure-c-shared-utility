// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdint.h>
#include <stdlib.h>
#include "testrunnerswitcher.h"
#include "umocktypename.h"

/* TODO:
- serialize tests
- test failures of malloc
*/

BEGIN_TEST_SUITE(umocktypename_unittests)

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

/* umocktypename_normalize */

/* Tests_SRS_UMOCKTYPENAME_01_001: [ umocktypename_normalize shall return a char\* with a newly allocated string that contains the normalized typename. ]*/
TEST_FUNCTION(umocktypename_normalize_returns_the_same_string_when_already_normalized)
{
    // arrange

    // act
    char* result = umocktypename_normalize("char");

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "char", result);

    // cleanup
    free(result);
}

END_TEST_SUITE(umocktypename_unittests)
