// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "TestRunnerSwitcher.h"
#include "MicroMock.h"
#include "MicroMockCharStarArenullTerminatedStrings.h"
#include "uniqueid.h"

static MICROMOCK_MUTEX_HANDLE g_testByTest;

DEFINE_MICROMOCK_ENUM_TO_STRING(UNIQUEID_RESULT, UNIQUEID_RESULT_VALUES);

#define BUFFER_SIZE         37

static char* uidBuffer = NULL;
static char* uidBuffer2 = NULL;

BEGIN_TEST_SUITE(uniqueid_unittests)


TEST_SUITE_INITIALIZE(TestClassInitialize)
{
    g_testByTest = MicroMockCreateMutex();
    ASSERT_IS_NOT_NULL(g_testByTest);
}

TEST_SUITE_CLEANUP(TestClassCleanup)
{
    MicroMockDestroyMutex(g_testByTest);

#ifdef _CRTDBG_MAP_ALLOC
    if (_CrtDumpMemoryLeaks())
    {
#ifdef CPP_UNITTEST
        Logger::WriteMessage("Memory Leak found!!! Run test in debug mode & review Debug output for details.");
#endif
    }
#endif
}

TEST_FUNCTION_INITIALIZE(TestMethodInitialize)
{
    if (!MicroMockAcquireMutex(g_testByTest))
    {
        ASSERT_FAIL("our mutex is ABANDONED. Failure in test framework");
    }
}

TEST_FUNCTION_CLEANUP(TestMethodCleanup)
{
    if (uidBuffer != NULL)
    {
        free(uidBuffer);
        uidBuffer = NULL;
    }

    if (uidBuffer2 != NULL)
    {
        free(uidBuffer2);
        uidBuffer2 = NULL;
    }

    if (!MicroMockReleaseMutex(g_testByTest))
    {
        ASSERT_FAIL("failure in test framework at ReleaseMutex");
    }
}

/* UniqueId_Generate */
TEST_FUNCTION(UniqueId_Generate_UID_NULL_Fail)
{
    //Arrange

    //Act
    UNIQUEID_RESULT result = UniqueId_Generate(NULL, BUFFER_SIZE);

    //Assert
    ASSERT_ARE_EQUAL(UNIQUEID_RESULT, UNIQUEID_INVALID_ARG, result);
}

TEST_FUNCTION(UniqueId_Generate_Len_too_small_Fail)
{
    //Arrange
    char uid[BUFFER_SIZE];

    //Act
    UNIQUEID_RESULT result = UniqueId_Generate(uid, BUFFER_SIZE/2);

    //Assert
    ASSERT_ARE_EQUAL(UNIQUEID_RESULT, UNIQUEID_INVALID_ARG, result);
}

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
