// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <cstdlib>
#ifdef _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#include <stddef.h>

#include "testrunnerswitcher.h"
#include "micromock.h"
#include "crt_abstractions.h"
#include "condition.h"
#include "lock.h"
//#include <thr/threads.h>

#define TEST_LOCK_HANDLE    (LOCK_HANDLE)0x4443
#define CONDITION_WAIT_MS   2000
DEFINE_MICROMOCK_ENUM_TO_STRING(COND_RESULT, COND_RESULT_VALUES);

static MICROMOCK_MUTEX_HANDLE g_testByTest;
static MICROMOCK_GLOBAL_SEMAPHORE_HANDLE g_dllByDll;

/*TYPED_MOCK_CLASS(conditionMocks, CGlobalMock)
{
public:
    MOCK_STATIC_METHOD_0(, LOCK_HANDLE, Lock_Init)
    MOCK_METHOD_END(LOCK_HANDLE, TEST_LOCK_HANDLE);

    MOCK_STATIC_METHOD_1(, LOCK_RESULT, Lock_Deinit, LOCK_HANDLE, handle)
    MOCK_METHOD_END(LOCK_RESULT, LOCK_OK);

//MOCK_STATIC_METHOD_1(, int, cnd_broadcast, cnd_t*, cond)
//MOCK_METHOD_END(int, thrd_success);
};

DECLARE_GLOBAL_MOCK_METHOD_0(conditionMocks, , LOCK_HANDLE, Lock_Init);
DECLARE_GLOBAL_MOCK_METHOD_1(conditionMocks, , LOCK_RESULT, Lock_Deinit, LOCK_HANDLE, handle);*/

COND_RESULT Condition_Handle_ToString(COND_HANDLE handle)
{
    COND_RESULT result = COND_OK;
    if (handle == NULL)
    {
        result = COND_ERROR;
    }
    return result;
}

BEGIN_TEST_SUITE(Condition_UnitTests)

TEST_SUITE_INITIALIZE(a)
{
    INITIALIZE_MEMORY_DEBUG(g_dllByDll);
    g_testByTest = MicroMockCreateMutex();
    ASSERT_IS_NOT_NULL(g_testByTest);
}

TEST_SUITE_CLEANUP(b)
{
    MicroMockDestroyMutex(g_testByTest);
    DEINITIALIZE_MEMORY_DEBUG(g_dllByDll);
}

TEST_FUNCTION(Condition_Init_Success)
{
    //arrange
    COND_HANDLE handle = NULL;

    //act
    handle = Condition_Init();

    //assert
    ASSERT_IS_NOT_NULL(handle);

    //free
    Condition_Deinit(handle);
}

TEST_FUNCTION(Condition_Post_Handle_NULL_Failure)
{
    //arrange
    COND_HANDLE handle = NULL;
    COND_RESULT result;

    //act
    result = Condition_Post(NULL);

    //assert
    ASSERT_ARE_EQUAL(COND_RESULT, COND_INVALID_ARG, result);

    //free
}

TEST_FUNCTION(Condition_Post_Handle_Succeed)
{
    //arrange
    COND_HANDLE handle = NULL;
    COND_RESULT result;

    handle = Condition_Init();

    //act
    result = Condition_Post(handle);

    //assert
    ASSERT_ARE_EQUAL(COND_RESULT, COND_OK, result);

    //free
    Condition_Deinit(handle);
}

TEST_FUNCTION(Condition_Wait_Handle_NULL_Fail)
{
    //arrange
    //COND_HANDLE handle = NULL;
    COND_RESULT result;
    LOCK_HANDLE lock;

    //handle = Condition_Init();
    lock = Lock_Init();

    //act
    result = Condition_Wait(NULL, lock, 0);

    //assert
    ASSERT_ARE_EQUAL(COND_RESULT, COND_INVALID_ARG, result);

    //free
    //Condition_Deinit(handle);
    Lock_Deinit(lock);
}

TEST_FUNCTION(Condition_Wait_LOCK_NULL_Fail)
{
    //arrange
    COND_HANDLE handle = NULL;
    COND_RESULT result;
    //LOCK_HANDLE lock;

    handle = Condition_Init();
    //lock = Lock_Init();

    //act
    result = Condition_Wait(handle, NULL, 0);

    //assert
    ASSERT_ARE_EQUAL(COND_RESULT, COND_INVALID_ARG, result);

    //free
    Condition_Deinit(handle);
    //Lock_Deinit(lock);
}

TEST_FUNCTION(Condition_Wait_LOCK_NULL_Ms_Fail)
{
    //arrange
    COND_HANDLE handle = NULL;
    COND_RESULT result;

    handle = Condition_Init();

    //act
    result = Condition_Wait(handle, NULL, CONDITION_WAIT_MS);

    //assert
    ASSERT_ARE_EQUAL(COND_RESULT, COND_INVALID_ARG, result);

    //free
    Condition_Deinit(handle);
}

TEST_FUNCTION(Condition_Deinit_Fail)
{
    //arrange
    COND_HANDLE handle = NULL;

    //act

    //assert

    //free
    Condition_Deinit(handle);
}

/*TEST_FUNCTION(Condition_Wait_Succeed)
{
    //arrange
    COND_HANDLE handle = NULL;
    COND_RESULT result;
    LOCK_HANDLE lockHandle;
    
    handle = Condition_Init();
    lockHandle = Lock_Init();
    Lock(lockHandle);

    // Need to signal the condition
    cnd_signal( (cnd_t*)handle);

    //act
    result = Condition_Wait(handle, lockHandle, 0);

    //assert
    ASSERT_ARE_EQUAL(COND_RESULT, COND_OK, result);

    //free
    Condition_Deinit(handle);
    Lock_Deinit(lockHandle);
}*/

END_TEST_SUITE(Condition_UnitTests);
