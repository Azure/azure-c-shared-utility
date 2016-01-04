// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

//
// PUT NO INCLUDES BEFORE HERE !!!!
//
#include <cstdlib>
#ifdef _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#include <stddef.h>

//
// PUT NO CLIENT LIBRARY INCLUDES BEFORE HERE !!!!
//
#include "testrunnerswitcher.h"
#include "constbuffer.h"
#include "buffer_.h"
#include "micromock.h"
#include "lock.h"

#ifdef _MSC_VER
#pragma warning(disable:4505)
#endif

static MICROMOCK_MUTEX_HANDLE g_testByTest;
static MICROMOCK_GLOBAL_SEMAPHORE_HANDLE g_dllByDll;

#define GBALLOC_H

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

static const char* buffer1 = "le buffer no 1";
static const char* buffer2 = NULL;
static const char* buffer3 = "three";

#define BUFFER1_HANDLE (void*)1
#define BUFFER1_u_char ((unsigned char*)buffer1)
#define BUFFER1_length strlen(buffer1)

#define BUFFER2_HANDLE (void*)2
#define BUFFER2_u_char ((unsigned char*)buffer2)
#define BUFFER2_length ((size_t)0)

#define BUFFER3_HANDLE (void*)3
#define BUFFER3_u_char ((unsigned char*)buffer3)
#define BUFFER3_length ((size_t)0)

static size_t currentmalloc_call = 0;
static size_t whenShallmalloc_fail = 0;

TYPED_MOCK_CLASS(CMocks, CGlobalMock)
{
public:

    MOCK_STATIC_METHOD_1(, void*, gballoc_malloc, size_t, size)
        void* result2;
    currentmalloc_call++;
    if (whenShallmalloc_fail > 0)
    {
        if (currentmalloc_call == whenShallmalloc_fail)
        {
            result2 = NULL;
        }
        else
        {
            result2 = BASEIMPLEMENTATION::gballoc_malloc(size);
        }
    }
    else
    {
        result2 = BASEIMPLEMENTATION::gballoc_malloc(size);
    }
    MOCK_METHOD_END(void*, result2);

    MOCK_STATIC_METHOD_1(, void, gballoc_free, void*, ptr)
        BASEIMPLEMENTATION::gballoc_free(ptr);
    MOCK_VOID_METHOD_END()

    MOCK_STATIC_METHOD_1(, unsigned char*, BUFFER_u_char, BUFFER_HANDLE, handle);
    unsigned char* result2;
    if (handle == BUFFER1_HANDLE)
    {
        result2 = BUFFER1_u_char;
    }
    else
    {
        ASSERT_FAIL("who am I?");
    }
    MOCK_METHOD_END(unsigned char*, result2)

    MOCK_STATIC_METHOD_1(, size_t, BUFFER_length, BUFFER_HANDLE, handle);
    size_t result2;
    if (handle == BUFFER1_HANDLE)
    {
        result2 = BUFFER1_length;
    }
    else
    {
        ASSERT_FAIL("who am I?");
    }
    MOCK_METHOD_END(size_t, result2)
};

DECLARE_GLOBAL_MOCK_METHOD_1(CMocks, , void*, gballoc_malloc, size_t, size);
DECLARE_GLOBAL_MOCK_METHOD_1(CMocks, , void, gballoc_free, void*, ptr);

DECLARE_GLOBAL_MOCK_METHOD_1(CMocks, , unsigned char*, BUFFER_u_char, BUFFER_HANDLE, handle);
DECLARE_GLOBAL_MOCK_METHOD_1(CMocks, , size_t, BUFFER_length, BUFFER_HANDLE, handle);

BEGIN_TEST_SUITE(constbuffer_unittests)

    TEST_SUITE_INITIALIZE(setsBufferTempSize)
    {
        INITIALIZE_MEMORY_DEBUG(g_dllByDll);
        g_testByTest = MicroMockCreateMutex();
        ASSERT_IS_NOT_NULL(g_testByTest);
    }

    TEST_SUITE_CLEANUP(TestClassCleanup)
    {
        MicroMockDestroyMutex(g_testByTest);
        DEINITIALIZE_MEMORY_DEBUG(g_dllByDll);
    }

    TEST_FUNCTION_INITIALIZE(f)
    {
        if (!MicroMockAcquireMutex(g_testByTest))
        {
            ASSERT_FAIL("our mutex is ABANDONED. Failure in test framework");
        }

        currentmalloc_call = 0;
        whenShallmalloc_fail = 0;

    }

    TEST_FUNCTION_CLEANUP(cleans)
    {
        if (!MicroMockReleaseMutex(g_testByTest))
        {
            ASSERT_FAIL("failure in test framework at ReleaseMutex");
        }
    }

    /*Tests_SRS_CONSTBUFFER_02_001: [If source is NULL and size is different than 0 then CONSTBUFFER_Create shall fail and return NULL.]*/
    TEST_FUNCTION(CONSTBUFFER_Create_with_invalid_args_fails)
    {
        ///arrange
        
        ///act
        auto handle = CONSTBUFFER_Create(NULL, 1);

        ///assert
        ASSERT_IS_NULL(handle);

        ///cleanup
    }

    /*Tests_SRS_CONSTBUFFER_02_002: [Otherwise, CONSTBUFFER_Create shall create a copy of the memory area pointed to by source having size bytes.]*/
    /*Tests_SRS_CONSTBUFFER_02_009: [Otherwise, CONSTBUFFER_CreateFromBuffer shall return a non-NULL handle.]*/
    TEST_FUNCTION(CONSTBUFFER_Create_succeeds)
    {
        ///arrange
        CMocks mocks;

        ///act
        /*this is the handle*/
        STRICT_EXPECTED_CALL(mocks, gballoc_malloc(IGNORED_NUM_ARG))
            .IgnoreArgument(1);
        /*this is the content*/
        STRICT_EXPECTED_CALL(mocks, gballoc_malloc(BUFFER1_length));

        auto handle = CONSTBUFFER_Create(BUFFER1_u_char, BUFFER1_length);

        ///assert
        ASSERT_IS_NOT_NULL(handle);
        /*testing the "copy"*/
        auto content = CONSTBUFFER_GetContent(handle);
        ASSERT_ARE_EQUAL(size_t, BUFFER1_length, content->size);
        ASSERT_ARE_EQUAL(int, 0, memcmp(BUFFER1_u_char, content->buffer, BUFFER1_length));
        /*testing that it is a copy and not a pointer assignment*/
        ASSERT_ARE_NOT_EQUAL(void_ptr, BUFFER1_u_char, content->buffer);
        mocks.AssertActualAndExpectedCalls();

        ///cleanup
        CONSTBUFFER_Destroy(handle);
    }

    /*Tests_SRS_CONSTBUFFER_02_003: [If creating the copy fails then CONSTBUFFER_Create shall return NULL.]*/
    TEST_FUNCTION(CONSTBUFFER_Create_fails_when_malloc_fails_1)
    {
        ///arrange
        CMocks mocks;

        ///act
        /*this is the handle*/
        STRICT_EXPECTED_CALL(mocks, gballoc_malloc(IGNORED_NUM_ARG))
            .IgnoreArgument(1);
        STRICT_EXPECTED_CALL(mocks, gballoc_free(IGNORED_PTR_ARG))
            .IgnoreArgument(1);
        /*this is the content*/
        whenShallmalloc_fail = 2;
        STRICT_EXPECTED_CALL(mocks, gballoc_malloc(BUFFER1_length));

        auto handle = CONSTBUFFER_Create(BUFFER1_u_char, BUFFER1_length);

        ///assert
        ASSERT_IS_NULL(handle);
        mocks.AssertActualAndExpectedCalls();

        ///cleanup
    }

    /*Tests_SRS_CONSTBUFFER_02_003: [If creating the copy fails then CONSTBUFFER_Create shall return NULL.]*/
    TEST_FUNCTION(CONSTBUFFER_Create_fails_when_malloc_fails_2)
    {
        ///arrange
        CMocks mocks;

        ///act
        /*this is the handle*/
        whenShallmalloc_fail = 1;
        STRICT_EXPECTED_CALL(mocks, gballoc_malloc(IGNORED_NUM_ARG))
            .IgnoreArgument(1);

        auto handle = CONSTBUFFER_Create(BUFFER1_u_char, BUFFER1_length);

        ///assert
        ASSERT_IS_NULL(handle);
        mocks.AssertActualAndExpectedCalls();

        ///cleanup
    }

    /*Tests_SRS_CONSTBUFFER_02_010: [The non-NULL handle returned by CONSTBUFFER_CreateFromBuffer shall have its ref count set to "1".]*/
    TEST_FUNCTION(CONSTBUFFER_Create_is_ref_counted_1)
    {
        ///arrange
        CMocks mocks;
        auto handle = CONSTBUFFER_Create(BUFFER1_u_char, BUFFER1_length);
        mocks.ResetAllCalls();
        ///act

        /*this is the content*/
        STRICT_EXPECTED_CALL(mocks, gballoc_free(IGNORED_PTR_ARG))
            .IgnoreArgument(1);
        /*this is the handle*/
        STRICT_EXPECTED_CALL(mocks, gballoc_free(IGNORED_PTR_ARG))
            .IgnoreArgument(1);

        CONSTBUFFER_Destroy(handle);

        ///assert
        mocks.AssertActualAndExpectedCalls();

        ///cleanup
    }

    /*Tests_SRS_CONSTBUFFER_02_002: [Otherwise, CONSTBUFFER_Create shall create a copy of the memory area pointed to by source having size bytes.]*/
    /*Tests_SRS_CONSTBUFFER_02_009: [Otherwise, CONSTBUFFER_CreateFromBuffer shall return a non-NULL handle.]*/
    TEST_FUNCTION(CONSTBUFFER_Create_from_0_size_succeeds_1)
    {
        ///arrange
        CMocks mocks;

        ///act
        /*this is the handle*/
        STRICT_EXPECTED_CALL(mocks, gballoc_malloc(IGNORED_NUM_ARG))
            .IgnoreArgument(1);

        auto handle = CONSTBUFFER_Create(BUFFER2_u_char, BUFFER2_length);

        ///assert
        ASSERT_IS_NOT_NULL(handle);
        /*testing the "copy"*/
        auto content = CONSTBUFFER_GetContent(handle);
        ASSERT_ARE_EQUAL(size_t, BUFFER2_length, content->size);
        /*testing that it is a copy and not a pointer assignment*/
        mocks.AssertActualAndExpectedCalls();

        ///cleanup
        CONSTBUFFER_Destroy(handle);
    }

    /*Tests_SRS_CONSTBUFFER_02_002: [Otherwise, CONSTBUFFER_Create shall create a copy of the memory area pointed to by source having size bytes.]*/
    /*Tests_SRS_CONSTBUFFER_02_009: [Otherwise, CONSTBUFFER_CreateFromBuffer shall return a non-NULL handle.]*/
    TEST_FUNCTION(CONSTBUFFER_Create_from_0_size_succeeds_2)
    {
        ///arrange
        CMocks mocks;

        ///act
        /*this is the handle*/
        STRICT_EXPECTED_CALL(mocks, gballoc_malloc(IGNORED_NUM_ARG))
            .IgnoreArgument(1);

        auto handle = CONSTBUFFER_Create(BUFFER3_u_char, BUFFER3_length);

        ///assert
        ASSERT_IS_NOT_NULL(handle);
        /*testing the "copy"*/
        auto content = CONSTBUFFER_GetContent(handle);
        ASSERT_ARE_EQUAL(size_t, BUFFER3_length, content->size);
        /*testing that it is a copy and not a pointer assignment*/
        mocks.AssertActualAndExpectedCalls();

        ///cleanup
        CONSTBUFFER_Destroy(handle);
    }

    /*Tests_SRS_CONSTBUFFER_02_011: [If constbufferHandle is NULL then CONSTBUFFER_GetContent shall return NULL.]*/
    TEST_FUNCTION(CONSTBUFFER_GetContent_with_NULL_returns_NULL)
    {
        ///arrange
        CMocks mocks;

        ///act
        auto content = CONSTBUFFER_GetContent(NULL);

        ///assert
        ASSERT_IS_NULL(content);
        mocks.AssertActualAndExpectedCalls();

        ///cleanup
    }

    /*Tests_SRS_CONSTBUFFER_02_012: [Otherwise, CONSTBUFFER_GetContent shall return a const CONSTBUFFER* that matches byte by byte the original bytes used to created the const buffer and has the same length.]*/
    TEST_FUNCTION(CONSTBUFFER_GetContent_succeeds_1)
    {
        ///arrange
        CMocks mocks;
        auto handle = CONSTBUFFER_Create(BUFFER1_u_char, BUFFER1_length);
        mocks.ResetAllCalls();

        ///act
        auto content = CONSTBUFFER_GetContent(handle);

        ///assert
        ASSERT_IS_NOT_NULL(content);
        /*testing the "copy"*/
        ASSERT_ARE_EQUAL(size_t, BUFFER1_length, content->size);
        ASSERT_ARE_EQUAL(int, 0, memcmp(BUFFER1_u_char, content->buffer, BUFFER1_length));
        /*testing that it is a copy and not a pointer assignment*/
        ASSERT_ARE_NOT_EQUAL(void_ptr, BUFFER1_u_char, content->buffer);
        mocks.AssertActualAndExpectedCalls();

        ///cleanup
        CONSTBUFFER_Destroy(handle);
    }

    /*Tests_SRS_CONSTBUFFER_02_012: [Otherwise, CONSTBUFFER_GetContent shall return a const CONSTBUFFER* that matches byte by byte the original bytes used to created the const buffer and has the same length.]*/
    TEST_FUNCTION(CONSTBUFFER_GetContent_succeeds_2)
    {
        ///arrange
        CMocks mocks;
        auto handle = CONSTBUFFER_Create(NULL, 0);
        mocks.ResetAllCalls();

        ///act
        auto content = CONSTBUFFER_GetContent(handle);

        ///assert
        ASSERT_IS_NOT_NULL(content);
        /*testing the "copy"*/
        ASSERT_ARE_EQUAL(size_t, 0, content->size);
        mocks.AssertActualAndExpectedCalls();

        ///cleanup
        CONSTBUFFER_Destroy(handle);
    }

    /*Tests_SRS_CONSTBUFFER_02_013: [If constbufferHandle is NULL then CONSTBUFFER_Clone shall fail and return NULL.]*/
    TEST_FUNCTION(CONSTBUFFER_Clone_with_NULL_returns_NULL)
    {
        ///arrange
        CMocks mocks;

        ///act
        auto handle = CONSTBUFFER_Clone(NULL);

        ///assert
        ASSERT_IS_NULL(handle);

        ///cleanup
    }

    /*Tests_SRS_CONSTBUFFER_02_014: [Otherwise, CONSTBUFFER_Clone shall increment the reference count and return constbufferHandle.]*/
    TEST_FUNCTION(CONSTBUFFER_Clone_increments_ref_count_1)
    {
        ///arrange
        CMocks mocks;
        auto handle = CONSTBUFFER_Create(BUFFER1_u_char, BUFFER1_length);
        mocks.ResetAllCalls();

        ///act
        auto clone = CONSTBUFFER_Clone(handle);

        ///assert
        ASSERT_ARE_EQUAL(void_ptr, handle, clone);
        mocks.AssertActualAndExpectedCalls();

        ///cleanup
        CONSTBUFFER_Destroy(handle);
        CONSTBUFFER_Destroy(clone);
    }

    /*Tests_SRS_CONSTBUFFER_02_014: [Otherwise, CONSTBUFFER_Clone shall increment the reference count and return constbufferHandle.]*/
    /*Tests_SRS_CONSTBUFFER_02_016: [Otherwise, CONSTBUFFER_Destroy shall decrement the refcount on the constbufferHandle handle.]*/
    TEST_FUNCTION(CONSTBUFFER_Clone_increments_ref_count_2)
    {
        ///arrange
        CMocks mocks;
        auto handle = CONSTBUFFER_Create(BUFFER1_u_char, BUFFER1_length);
        auto clone = CONSTBUFFER_Clone(handle);
        mocks.ResetAllCalls();

        ///act
        CONSTBUFFER_Destroy(clone); /*only a dec_Ref is expected here, so no effects*/

        ///assert
        mocks.AssertActualAndExpectedCalls();

        ///cleanup
        CONSTBUFFER_Destroy(handle);
    }

    /*Tests_SRS_CONSTBUFFER_02_014: [Otherwise, CONSTBUFFER_Clone shall increment the reference count and return constbufferHandle.]*/
    /*Tests_SRS_CONSTBUFFER_02_016: [Otherwise, CONSTBUFFER_Destroy shall decrement the refcount on the constbufferHandle handle.]*/
    TEST_FUNCTION(CONSTBUFFER_Clone_increments_ref_count_3)
    {
        ///arrange
        CMocks mocks;
        auto handle = CONSTBUFFER_Create(BUFFER1_u_char, BUFFER1_length);
        auto clone = CONSTBUFFER_Clone(handle);
        CONSTBUFFER_Destroy(handle); /*only a dec_Ref is expected here, so no effects*/
        mocks.ResetAllCalls();

        ///act
        /*this is the content*/
        STRICT_EXPECTED_CALL(mocks, gballoc_free(IGNORED_PTR_ARG))
            .IgnoreArgument(1);
        /*this is the handle*/
        STRICT_EXPECTED_CALL(mocks, gballoc_free(IGNORED_PTR_ARG))
            .IgnoreArgument(1);
        CONSTBUFFER_Destroy(clone);

        ///assert
        mocks.AssertActualAndExpectedCalls();

        ///cleanup
        CONSTBUFFER_Destroy(handle);
    }

    /*Tests_SRS_CONSTBUFFER_02_015: [If constbufferHandle is NULL then CONSTBUFFER_Destroy shall do nothing.]*/
    TEST_FUNCTION(CONSTBUFFER_Destroy_with_NULL_argument_does_nothing)
    {
        ///arrange
        CMocks mocks;
        
        ///act
        CONSTBUFFER_Destroy(NULL);

        ///assert
        mocks.AssertActualAndExpectedCalls();

        ///cleanup
    }

END_TEST_SUITE(constbuffer_unittests)
