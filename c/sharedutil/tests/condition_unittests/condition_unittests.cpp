// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <cstdlib>
#ifdef _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#include "micromock.h"
#include "testrunnerswitcher.h"
#include "crt_abstractions.h"
#include "condition.h"

DEFINE_MICROMOCK_ENUM_TO_STRING(COND_RESULT, COND_RESULT_VALUES);

COND_RESULT Condition_Handle_ToString(COND_HANDLE handle)
{
    COND_RESULT result = COND_OK;

    if (handle == NULL)
    {
        result = COND_ERROR;
    }

    return result;
}

static MICROMOCK_GLOBAL_SEMAPHORE_HANDLE g_dllByDll;

BEGIN_TEST_SUITE(Condition_UnitTests)

TEST_SUITE_INITIALIZE(a)
{
    INITIALIZE_MEMORY_DEBUG(g_dllByDll);
}
TEST_SUITE_CLEANUP(b)
{
    DEINITIALIZE_MEMORY_DEBUG(g_dllByDll);
}

TEST_FUNCTION(Test_condition_Init_Success)
{
    //arrange
    COND_HANDLE handle = NULL;
    COND_RESULT result;

    //act
    handle = Condition_Init();

    //assert
    ASSERT_IS_NOT_NULL(handle);

    //free
    result = Condition_Deinit(handle);
    ASSERT_ARE_EQUAL(COND_RESULT, COND_OK, result);
}

END_TEST_SUITE(Condition_UnitTests);
