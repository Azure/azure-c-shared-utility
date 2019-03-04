// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifdef __cplusplus
#include <cstdlib>
#else
#include <stdlib.h>
#endif


#include "testrunnerswitcher.h"
#include "azure_c_shared_utility/crt_abstractions.h"
#include "azure_c_shared_utility/lock.h"

TEST_DEFINE_ENUM_TYPE(LOCK_RESULT, LOCK_RESULT_VALUES);

BEGIN_TEST_SUITE(LOCK_UnitTests)

TEST_SUITE_INITIALIZE(a)
{
}

TEST_SUITE_CLEANUP(b)
{
}

/* Tests_SRS_LOCK_10_002: [Lock_Init on success shall return a valid lock handle which should be a non NULL value] */
TEST_FUNCTION(LOCK_Lock_Init_succeeds)
{
    //arrange

    //act
    LOCK_HANDLE handle = Lock_Init();

    //assert
    ASSERT_IS_NOT_NULL(handle);

    //cleanup
    (void)Lock_Deinit(handle);
}

/* Tests_SRS_LOCK_10_005: [Lock on success shall return LOCK_OK] */
TEST_FUNCTION(LOCK_Init_Lock_succeeds)
{
    //arrange
    LOCK_HANDLE handle = Lock_Init();

    //act
    LOCK_RESULT result = Lock(handle);

    //assert
    ASSERT_ARE_EQUAL(LOCK_RESULT, LOCK_OK, result);

    //cleanup
    (void)Unlock(handle);
    (void)Lock_Deinit(handle);
}

/* Tests_SRS_LOCK_10_009: [Unlock on success shall return LOCK_OK] */
TEST_FUNCTION(LOCK_Init_Lock_Unlock_succeeds)
{
    //arrange
    LOCK_RESULT result;
    LOCK_HANDLE handle = Lock_Init();
    (void)Lock(handle);

    //act
    result = Unlock(handle);

    //assert
    ASSERT_ARE_EQUAL(LOCK_RESULT, LOCK_OK, result);

    //cleanup
    (void)Lock_Deinit(handle);
}

/* Tests_SRS_LOCK_10_002: [Lock_Init on success shall return a valid lock handle which should be a non NULL value] */
TEST_FUNCTION(LOCK_Init_DeInit_succeeds)
{
    //arrange
    LOCK_HANDLE handle = Lock_Init();

    //act
    LOCK_RESULT result = Lock_Deinit(handle);

    //assert
    ASSERT_ARE_EQUAL(LOCK_RESULT, LOCK_OK, result);
}

/* Tests_SRS_LOCK_10_007: [Lock_Deinit on NULL handle passed returns LOCK_ERROR] */
TEST_FUNCTION(LOCK_Lock_NULL_fails)
{
    //arrange

    //act
    LOCK_RESULT result = Lock(NULL);

    ASSERT_ARE_EQUAL(LOCK_RESULT, LOCK_ERROR, result);
}

/* Tests_SRS_LOCK_10_011: [Unlock on NULL handle passed returns LOCK_ERROR] */
TEST_FUNCTION(LOCK_Unlock_NULL_fails)
{
    //arrange

    //act
    LOCK_RESULT result = Unlock(NULL);

    /*Tests_SRS_LOCK_10_011:[ This API on NULL handle passed returns LOCK_ERROR]*/
    ASSERT_ARE_EQUAL(LOCK_RESULT, LOCK_ERROR, result);
}

TEST_FUNCTION(LOCK_DeInit_NULL_fails)
{
    //arrange

    //act
    LOCK_RESULT result = Lock_Deinit(NULL);

    //assert
    ASSERT_ARE_EQUAL(LOCK_RESULT, LOCK_ERROR, result);
}

END_TEST_SUITE(LOCK_UnitTests);
