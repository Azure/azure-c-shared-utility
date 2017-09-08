// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifdef __cplusplus
#include <cstdlib>
#include <cstddef>
#else
#include <stdlib.h>
#include <stddef.h>
#endif

#include "testrunnerswitcher.h"
#include "azure_c_shared_utility/uniqueid.h"
#include <rpc.h>

static TEST_MUTEX_HANDLE g_testByTest;
static TEST_MUTEX_HANDLE g_dllByDll;

TEST_DEFINE_ENUM_TYPE(UNIQUEID_RESULT, UNIQUEID_RESULT_VALUES);

#define BUFFER_SIZE                 37
#define DEFAULT_UUID_N_OF_OCTECTS   (BUFFER_SIZE - 4)/2

static unsigned char uid_octects[DEFAULT_UUID_N_OF_OCTECTS] = { 222,193,74,152,197,252,67,14,180,227,51,193,196,52,220,175 };


BEGIN_TEST_SUITE(uniqueid_unittests)

TEST_SUITE_INITIALIZE(suite_init)
{
    TEST_INITIALIZE_MEMORY_DEBUG(g_dllByDll);
    g_testByTest = TEST_MUTEX_CREATE();
    ASSERT_IS_NOT_NULL(g_testByTest);
}

TEST_SUITE_CLEANUP(suite_cleanup)
{
    TEST_MUTEX_DESTROY(g_testByTest);
    TEST_DEINITIALIZE_MEMORY_DEBUG(g_dllByDll);
}

TEST_FUNCTION_INITIALIZE(TestMethodInitialize)
{
    if (TEST_MUTEX_ACQUIRE(g_testByTest))
    {
        ASSERT_FAIL("our mutex is ABANDONED. Failure in test framework");
    }
}

TEST_FUNCTION_CLEANUP(TestMethodCleanup)
{
    TEST_MUTEX_RELEASE(g_testByTest);
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

// Tests_SRS_UNIQUEID_09_001: [ If `uid` or `output_string` are NULL, UniqueId_GetStringFromBytes shall return UNIQUEID_INVALID_ARG ]
TEST_FUNCTION(UniqueId_GetStringFromBytes_NULL_uid)
{
    //Arrange
    char uid_as_string[BUFFER_SIZE];

    //Act
    UNIQUEID_RESULT result = UniqueId_GetStringFromBytes(NULL, DEFAULT_UUID_N_OF_OCTECTS, uid_as_string);

    //Assert
    ASSERT_ARE_EQUAL(UNIQUEID_RESULT, UNIQUEID_INVALID_ARG, result);
}

// Tests_SRS_UNIQUEID_09_001: [ If `uid` or `output_string` are NULL, UniqueId_GetStringFromBytes shall return UNIQUEID_INVALID_ARG ]
TEST_FUNCTION(UniqueId_GetStringFromBytes_NULL_output_string)
{
    //Arrange

    //Act
    UNIQUEID_RESULT result = UniqueId_GetStringFromBytes(uid_octects, DEFAULT_UUID_N_OF_OCTECTS, NULL);

    //Assert
    ASSERT_ARE_EQUAL(UNIQUEID_RESULT, UNIQUEID_INVALID_ARG, result);
}


// Tests_SRS_UNIQUEID_09_002: [ If `uid_size` is zero or not a multiple of two, UniqueId_GetStringFromBytes shall return UNIQUEID_INVALID_ARG ]
TEST_FUNCTION(UniqueId_GetStringFromBytes_Invalid_uid_size)
{
    //Arrange
    char uid_as_string[BUFFER_SIZE];

    memset(uid_as_string, 0, BUFFER_SIZE);

    //Act
    UNIQUEID_RESULT result = UniqueId_GetStringFromBytes(uid_octects, DEFAULT_UUID_N_OF_OCTECTS - 1, uid_as_string);

    //Assert
    ASSERT_ARE_EQUAL(UNIQUEID_RESULT, UNIQUEID_INVALID_ARG, result);
    ASSERT_ARE_EQUAL(char_ptr, "\0", uid_as_string);
}

// Tests_SRS_UNIQUEID_09_003: [ `output_string` shall be filled according to RFC4122 using the byte sequence in `uid` ]
// Tests_SRS_UNIQUEID_09_004: [ If no failures occur, UniqueId_Generate shall return UNIQUEID_OK ]  
TEST_FUNCTION(UniqueId_GetStringFromBytes_Succeed)
{
    //Arrange
    char uid_as_string[BUFFER_SIZE];

    //Act
    UNIQUEID_RESULT result = UniqueId_GetStringFromBytes(uid_octects, DEFAULT_UUID_N_OF_OCTECTS, uid_as_string);

    //Assert
    ASSERT_ARE_EQUAL(UNIQUEID_RESULT, UNIQUEID_OK, result);
    ASSERT_ARE_EQUAL(char_ptr, "dec14a98-c5fc-430e-b4e3-33c1c434dcaf", uid_as_string);
}

END_TEST_SUITE(uniqueid_unittests)
