// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <cstdlib>
#ifdef _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#include <stddef.h>

#include "testrunnerswitcher.h"
#include "tickcounter.h"
#include "micromock.h"
#include "lock.h"
#include "threadapi.h"

static MICROMOCK_MUTEX_HANDLE g_testByTest;
static MICROMOCK_GLOBAL_SEMAPHORE_HANDLE g_dllByDll;

#define GBALLOC_H

#define BUSY_LOOP_TIME      1000000

extern "C" int gballoc_init(void);
extern "C" void gballoc_deinit(void);
extern "C" void* gballoc_malloc(size_t size);
extern "C" void* gballoc_calloc(size_t nmemb, size_t size);
extern "C" void* gballoc_realloc(void* ptr, size_t size);
extern "C" void gballoc_free(void* ptr);

namespace BASEIMPLEMENTATION
{
    /*if malloc is defined as gballoc_malloc at this moment, there'd be serious trouble*/
    #define Lock(x) (LOCK_OK + gballocState - gballocState) /*compiler warning about constant in if condition*/
    #define Unlock(x) (LOCK_OK + gballocState - gballocState)
    #define Lock_Init() (LOCK_HANDLE)0x42
    #define Lock_Deinit(x) (LOCK_OK + gballocState - gballocState)
    #include "gballoc.c"
    #undef Lock
    #undef Unlock
    #undef Lock_Init
    #undef Lock_Deinit
};

TYPED_MOCK_CLASS(CTickCounterMocks, CGlobalMock)
{
public:
    MOCK_STATIC_METHOD_1(, void*, gballoc_malloc, size_t, size)
        void* result2 = BASEIMPLEMENTATION::gballoc_malloc(size);
    MOCK_METHOD_END(void*, result2);

    MOCK_STATIC_METHOD_1(, void, gballoc_free, void*, ptr)
        BASEIMPLEMENTATION::gballoc_free(ptr);
    MOCK_VOID_METHOD_END()
};

DECLARE_GLOBAL_MOCK_METHOD_1(CTickCounterMocks, , void*, gballoc_malloc, size_t, size);
DECLARE_GLOBAL_MOCK_METHOD_1(CTickCounterMocks, , void, gballoc_free, void*, ptr);

BEGIN_TEST_SUITE(tickcounter_unittests)

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
    if (!MicroMockAcquireMutex(g_testByTest))
    {
        ASSERT_FAIL("our mutex is ABANDONED. Failure in test framework");
    }
}

TEST_FUNCTION_CLEANUP(TestMethodCleanup)
{
    if (!MicroMockReleaseMutex(g_testByTest))
    {
        ASSERT_FAIL("failure in test framework at ReleaseMutex");
    }
}

TEST_FUNCTION(tickcounter_create_fails)
{
    ///arrange
    CTickCounterMocks mocks;

    STRICT_EXPECTED_CALL(mocks, gballoc_malloc(IGNORED_NUM_ARG))
        .IgnoreArgument(1)
        .SetFailReturn((void*)NULL);

    ///act
    TICK_COUNTER_HANDLE tickHandle = tickcounter_create();

    ///assert
    ASSERT_IS_NULL(tickHandle);
}

TEST_FUNCTION(tickcounter_create_succeed)
{
    ///arrange
    CTickCounterMocks mocks;

    STRICT_EXPECTED_CALL(mocks, gballoc_malloc(IGNORED_NUM_ARG))
        .IgnoreArgument(1);

    ///act
    TICK_COUNTER_HANDLE tickHandle = tickcounter_create();

    ///assert
    ASSERT_IS_NOT_NULL(tickHandle);
    mocks.AssertActualAndExpectedCalls();

    tickcounter_destroy(tickHandle);
}

TEST_FUNCTION(tickcounter_destroy_tick_counter_NULL_succeed)
{
    ///arrange
    CTickCounterMocks mocks;

    ///act
    tickcounter_destroy(NULL);

    ///assert
}

TEST_FUNCTION(tickcounter_destroy_succeed)
{
    ///arrange
    CTickCounterMocks mocks;
    TICK_COUNTER_HANDLE tickHandle = tickcounter_create();
    mocks.ResetAllCalls();

    STRICT_EXPECTED_CALL(mocks, gballoc_free(IGNORED_PTR_ARG))
        .IgnoreArgument(1);

    ///act
    tickcounter_destroy(tickHandle);

    ///assert
    ASSERT_IS_NOT_NULL(tickHandle);
}

TEST_FUNCTION(tickcounter_get_current_ms_tick_counter_NULL_fail)
{
    ///arrange
    CTickCounterMocks mocks;
    uint64_t current_ms = 0;

    ///act
    int result = tickcounter_get_current_ms(NULL, &current_ms);

    ///assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    mocks.AssertActualAndExpectedCalls();
}

TEST_FUNCTION(tickcounter_get_current_ms_current_ms_NULL_fail)
{
    ///arrange
    CTickCounterMocks mocks;
    TICK_COUNTER_HANDLE tickHandle = tickcounter_create();
    mocks.ResetAllCalls();

    ///act
    int result = tickcounter_get_current_ms(tickHandle, NULL);

    ///assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    mocks.AssertActualAndExpectedCalls();

    tickcounter_destroy(tickHandle);
}

TEST_FUNCTION(tickcounter_get_current_ms_succeed)
{
    ///arrange
    CTickCounterMocks mocks;
    TICK_COUNTER_HANDLE tickHandle = tickcounter_create();
    mocks.ResetAllCalls();

    uint64_t current_ms = 0;

    ///act
    int result = tickcounter_get_current_ms(tickHandle, &current_ms);

    ///assert
    ASSERT_ARE_EQUAL(int, 0, result);
    mocks.AssertActualAndExpectedCalls();

    /// clean
    tickcounter_destroy(tickHandle);
}

//TEST_FUNCTION(tickcounter_get_current_ms_validate_tick_succeed)
//{
//    ///arrange
//    CTickCounterMocks mocks;
//    TICK_COUNTER_HANDLE tickHandle = tickcounter_create();
//    mocks.ResetAllCalls();
//
//    uint64_t first_ms = 0;
//
//    ThreadAPI_Sleep(1250);
//
//    ///act
//    int result = tickcounter_get_current_ms(tickHandle, &first_ms);
//
//    // busy loop here
//    ThreadAPI_Sleep(1250);
//
//    uint64_t next_ms = 0;
//
//    int resultAlso = tickcounter_get_current_ms(tickHandle, &next_ms);
//
//    ///assert
//    ASSERT_ARE_EQUAL(int, 0, result);
//    ASSERT_ARE_EQUAL(int, 0, resultAlso);
//    ASSERT_IS_TRUE(first_ms > 0);
//    ASSERT_IS_TRUE(next_ms > first_ms);
//    mocks.AssertActualAndExpectedCalls();
//
//    /// clean
//    tickcounter_destroy(tickHandle);
//}

END_TEST_SUITE(tickcounter_unittests)
