// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdint.h>
#include <stdlib.h>
#include "testrunnerswitcher.h"
#include "umocktypes.h"

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
    umocktypes_deinit();
}

/* umocktypename_normalize */

#if 0
TEST_FUNCTION(umocktypes_init_initializes_the_module)
{
    // arrange

    // act
    int result = umocktypes_init();

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
}
#endif

END_TEST_SUITE(umocktypename_unittests)
