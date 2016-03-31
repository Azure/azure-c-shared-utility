// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

//#define DECLSPEC_IMPORT

#include <cstdlib>
#ifdef _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#include <stddef.h>

#include "testrunnerswitcher.h"
#include "micromock.h"
#include "micromockcharstararenullterminatedstrings.h"
#include "uniqueid.h"
#include <rpc.h>

static MICROMOCK_MUTEX_HANDLE g_testByTest;
static MICROMOCK_GLOBAL_SEMAPHORE_HANDLE g_dllByDll;

DEFINE_MICROMOCK_ENUM_TO_STRING(UNIQUEID_RESULT, UNIQUEID_RESULT_VALUES);

#define BUFFER_SIZE         37

BEGIN_TEST_SUITE(uniqueid_unittests)

TEST_SUITE_INITIALIZE(suite_init)
{
    INITIALIZE_MEMORY_DEBUG(g_dllByDll);
    g_testByTest = MicroMockCreateMutex();
    ASSERT_IS_NOT_NULL(g_testByTest);
}

TEST_SUITE_CLEANUP(suite_cleanup)
{
    MicroMockDestroyMutex(g_testByTest);
    DEINITIALIZE_MEMORY_DEBUG(g_dllByDll);
}

TEST_FUNCTION_INITIALIZE(TestMethodInitialize)
{
    MicroMockAcquireMutex(g_testByTest);
}

TEST_FUNCTION_CLEANUP(TestMethodCleanup)
{
    if (!MicroMockReleaseMutex(g_testByTest))
    {
        ASSERT_FAIL("failure in test framework at ReleaseMutex");
    }
}

/* UniqueId_Generate */
/* Tests_SRS_UNIQUEID_07_002: [If uid is NULL then UniqueId_Generate shall return UNIQUEID_INVALID_ARG] */
TEST_FUNCTION(UniqueId_Generate_UID_NULL_Fail)
{
    //Arrange

    //Act
    UNIQUEID_RESULT result = UniqueId_Generate(NULL, BUFFER_SIZE);

    //Assert
    ASSERT_ARE_EQUAL(UNIQUEID_RESULT, UNIQUEID_INVALID_ARG, result);
}

/* Tests_SRS_UNIQUEID_07_003: [If len is less then 37 then UniqueId_Generate shall return UNIQUEID_INVALID_ARG] */
TEST_FUNCTION(UniqueId_Generate_Len_too_small_Fail)
{
    //Arrange
    char uid[BUFFER_SIZE];

    //Act
    UNIQUEID_RESULT result = UniqueId_Generate(uid, BUFFER_SIZE/2);

    //Assert
    ASSERT_ARE_EQUAL(UNIQUEID_RESULT, UNIQUEID_INVALID_ARG, result);
}

/* Tests_SRS_UNIQUEID_07_001: [UniqueId_Generate shall create a unique Id 36 character long string.] */
TEST_FUNCTION(UniqueId_Generate_Succeed)
{
    //Arrange
    char uid[BUFFER_SIZE];

    //Act
    UNIQUEID_RESULT result = UniqueId_Generate(uid, BUFFER_SIZE);

    //Assert
    ASSERT_ARE_EQUAL(UNIQUEID_RESULT, UNIQUEID_OK, result);
    ASSERT_ARE_EQUAL(size_t, 36, strlen(uid) );
}

END_TEST_SUITE(uniqueid_unittests)
