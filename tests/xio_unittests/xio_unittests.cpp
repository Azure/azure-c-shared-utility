// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "testrunnerswitcher.h"
#include "micromock.h"
#include "xio.h"
#include "lock.h"

#define TEST_CONCRETE_IO_HANDLE (CONCRETE_IO_HANDLE)0x4242

#define GBALLOC_H
extern "C" int gballoc_init(void);
extern "C" void gballoc_deinit(void);
extern "C" void* gballoc_malloc(size_t size);
extern "C" void* gballoc_calloc(size_t nmemb, size_t size);
extern "C" void* gballoc_realloc(void* ptr, size_t size);
extern "C" void gballoc_free(void* ptr);

static bool g_fail_alloc_calls;

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

TYPED_MOCK_CLASS(io_mocks, CGlobalMock)
{
public:
    MOCK_STATIC_METHOD_1(, void*, gballoc_malloc, size_t, size)
        void* ptr = NULL;
        if (!g_fail_alloc_calls)
        {
            ptr = BASEIMPLEMENTATION::gballoc_malloc(size);
        }
    MOCK_METHOD_END(void*, ptr);

    MOCK_STATIC_METHOD_1(, void, gballoc_free, void*, ptr)
        BASEIMPLEMENTATION::gballoc_free(ptr);
    MOCK_VOID_METHOD_END()

    /* io interface mocks */
    MOCK_STATIC_METHOD_2(, CONCRETE_IO_HANDLE, test_xio_create, void*, xio_create_parameters, LOGGER_LOG, logger_log)
    MOCK_METHOD_END(CONCRETE_IO_HANDLE, TEST_CONCRETE_IO_HANDLE);
    MOCK_STATIC_METHOD_1(, void, test_xio_destroy, CONCRETE_IO_HANDLE, handle)
    MOCK_VOID_METHOD_END();
    MOCK_STATIC_METHOD_7(, int, test_xio_open, CONCRETE_IO_HANDLE, handle, ON_IO_OPEN_COMPLETE, on_io_open_complete, void*, on_io_open_complete_context, ON_BYTES_RECEIVED, on_bytes_received, void*, on_bytes_received_context, ON_IO_ERROR, on_io_error, void*, on_io_error_context)
    MOCK_METHOD_END(int, 0);
    MOCK_STATIC_METHOD_3(, int, test_xio_close, CONCRETE_IO_HANDLE, handle, ON_IO_CLOSE_COMPLETE, on_io_close_complete, void*, callback_context)
    MOCK_METHOD_END(int, 0);
    MOCK_STATIC_METHOD_5(, int, test_xio_send, CONCRETE_IO_HANDLE, handle, const void*, buffer, size_t, size, ON_SEND_COMPLETE, on_send_complete, void*, callback_context)
    MOCK_METHOD_END(int, 0);
    MOCK_STATIC_METHOD_1(, void, test_xio_dowork, CONCRETE_IO_HANDLE, handle)
    MOCK_VOID_METHOD_END();
    MOCK_STATIC_METHOD_3(, int, test_xio_setoption, CONCRETE_IO_HANDLE, handle, const char*, optionName, const void*, value)
    MOCK_METHOD_END(int, 0);
};

DECLARE_GLOBAL_MOCK_METHOD_1(io_mocks, , void*, gballoc_malloc, size_t, size);
DECLARE_GLOBAL_MOCK_METHOD_1(io_mocks, , void, gballoc_free, void*, ptr);

extern "C"
{
    DECLARE_GLOBAL_MOCK_METHOD_2(io_mocks, , CONCRETE_IO_HANDLE, test_xio_create, void*, xio_create_parameters, LOGGER_LOG, logger_log);
    DECLARE_GLOBAL_MOCK_METHOD_1(io_mocks, , void, test_xio_destroy, CONCRETE_IO_HANDLE, handle);
    DECLARE_GLOBAL_MOCK_METHOD_7(io_mocks, , int, test_xio_open, CONCRETE_IO_HANDLE, handle, ON_IO_OPEN_COMPLETE, on_io_open_complete, void*, on_io_open_complete_context, ON_BYTES_RECEIVED, on_bytes_received, void*, on_bytes_received_context, ON_IO_ERROR, on_io_error, void*, on_io_error_context);
    DECLARE_GLOBAL_MOCK_METHOD_3(io_mocks, , int, test_xio_close, CONCRETE_IO_HANDLE, handle, ON_IO_CLOSE_COMPLETE, on_io_close_complete, void*, callback_context);
    DECLARE_GLOBAL_MOCK_METHOD_5(io_mocks, , int, test_xio_send, CONCRETE_IO_HANDLE, handle, const void*, buffer, size_t, size, ON_SEND_COMPLETE, on_send_complete, void*, callback_context);
    DECLARE_GLOBAL_MOCK_METHOD_1(io_mocks, , void, test_xio_dowork, CONCRETE_IO_HANDLE, handle);
    DECLARE_GLOBAL_MOCK_METHOD_3(io_mocks, , int, test_xio_setoption, CONCRETE_IO_HANDLE, handle, const char*, optionName, const void*, value);

    void test_on_bytes_received(void* context, const unsigned char* buffer, size_t size)
    {
        (void)context;
        (void)buffer;
        (void)size;
    }

    void test_on_io_open_complete(void* context, IO_OPEN_RESULT open_result)
    {
        (void)context;
        (void)open_result;
    }

    void test_on_io_close_complete(void* context)
    {
        (void)context;
    }

    void test_on_io_error(void* context)
    {
        (void)context;
    }

    void test_on_send_complete(void* context, IO_SEND_RESULT send_result)
    {
        (void)context;
        (void)send_result;
    }

    void test_logger_log(unsigned int options, char* format, ...)
    {
        (void)options;
        (void)format;
    }
}

const IO_INTERFACE_DESCRIPTION test_io_description =
{
    test_xio_create,
    test_xio_destroy,
    test_xio_open,
    test_xio_close,
    test_xio_send,
    test_xio_dowork,
    test_xio_setoption
};

MICROMOCK_MUTEX_HANDLE test_serialize_mutex;

BEGIN_TEST_SUITE(xio_unittests)

TEST_SUITE_INITIALIZE(suite_init)
{
    test_serialize_mutex = MicroMockCreateMutex();
    ASSERT_IS_NOT_NULL(test_serialize_mutex);
}

TEST_SUITE_CLEANUP(suite_cleanup)
{
    MicroMockDestroyMutex(test_serialize_mutex);
}

TEST_FUNCTION_INITIALIZE(method_init)
{
    if (!MicroMockAcquireMutex(test_serialize_mutex))
    {
        ASSERT_FAIL("Could not acquire test serialization mutex.");
    }
    g_fail_alloc_calls = false;
}

TEST_FUNCTION_CLEANUP(method_cleanup)
{
    if (!MicroMockReleaseMutex(test_serialize_mutex))
    {
        ASSERT_FAIL("Could not release test serialization mutex.");
    }
}

/* xio_create */

/* Tests_SRS_XIO_01_001: [xio_create shall return on success a non-NULL handle to a new IO interface.] */
/* Tests_SRS_XIO_01_002: [In order to instantiate the concrete IO implementation the function concrete_xio_create from the io_interface_description shall be called, passing the xio_create_parameters and logger_log arguments.] */
TEST_FUNCTION(xio_create_with_all_args_except_interface_description_NULL_succeeds)
{
    // arrange
    io_mocks mocks;

    EXPECTED_CALL(mocks, gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(mocks, test_xio_create(NULL, NULL));

    // act
    XIO_HANDLE result = xio_create(&test_io_description, NULL, NULL);

    // assert
    ASSERT_IS_NOT_NULL(result);
    mocks.AssertActualAndExpectedCalls();

    // cleanup
    xio_destroy(result);
}

/* Tests_SRS_XIO_01_002: [In order to instantiate the concrete IO implementation the function concrete_xio_create from the io_interface_description shall be called, passing the xio_create_parameters and logger_log arguments.] */
TEST_FUNCTION(xio_create_passes_the_args_to_the_concrete_io_implementation)
{
    // arrange
    io_mocks mocks;

    EXPECTED_CALL(mocks, gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(mocks, test_xio_create((void*)0x4243, test_logger_log));

    // act
    XIO_HANDLE result = xio_create(&test_io_description, (void*)0x4243, test_logger_log);

    // assert
    ASSERT_IS_NOT_NULL(result);
    mocks.AssertActualAndExpectedCalls();

    // cleanup
    xio_destroy(result);
}

/* Tests_SRS_XIO_01_016: [If the underlying concrete_xxio_create call fails, xxio_create shall return NULL.] */
TEST_FUNCTION(when_concrete_xxio_create_fails_then_xio_create_fails)
{
    // arrange
    io_mocks mocks;

    STRICT_EXPECTED_CALL(mocks, test_xio_create(NULL, NULL))
        .SetReturn((CONCRETE_IO_HANDLE)NULL);
    EXPECTED_CALL(mocks, gballoc_malloc(IGNORED_NUM_ARG));
    EXPECTED_CALL(mocks, gballoc_free(IGNORED_PTR_ARG));

    // act
    XIO_HANDLE result = xio_create(&test_io_description, NULL, NULL);

    // assert
    ASSERT_IS_NULL(result);
}

/* Tests_SRS_XIO_01_003: [If the argument io_interface_description is NULL, xio_create shall return NULL.] */
TEST_FUNCTION(when_io_interface_description_is_NULL_then_xio_create_fails)
{
    // arrange
    io_mocks mocks;

    // act
    XIO_HANDLE result = xio_create(NULL, NULL, NULL);

    // assert
    ASSERT_IS_NULL(result);
}

/* Tests_SRS_XIO_01_004: [If any io_interface_description member is NULL, xio_create shall return NULL.] */
TEST_FUNCTION(when_concrete_xio_create_is_NULL_then_xio_create_fails)
{
    // arrange
    io_mocks mocks;
    const IO_INTERFACE_DESCRIPTION io_description_null =
    {
        NULL,
        test_xio_destroy,
        test_xio_open,
        test_xio_close,
        test_xio_send,
        test_xio_dowork,
        test_xio_setoption
    };

    // act
    XIO_HANDLE result = xio_create(&io_description_null, NULL, NULL);

    // assert
    ASSERT_IS_NULL(result);
}

/* Tests_SRS_XIO_01_004: [If any io_interface_description member is NULL, xio_create shall return NULL.] */
TEST_FUNCTION(when_concrete_xio_destroy_is_NULL_then_xio_create_fails)
{
    // arrange
    io_mocks mocks;
    const IO_INTERFACE_DESCRIPTION io_description_null =
    {
        test_xio_create,
        NULL,
        test_xio_open,
        test_xio_close,
        test_xio_send,
        test_xio_dowork,
        test_xio_setoption
    };

    // act
    XIO_HANDLE result = xio_create(&io_description_null, NULL, NULL);

    // assert
    ASSERT_IS_NULL(result);
}

/* Tests_SRS_XIO_01_004: [If any io_interface_description member is NULL, xio_create shall return NULL.] */
TEST_FUNCTION(when_concrete_xio_open_is_NULL_then_xio_create_fails)
{
    // arrange
    io_mocks mocks;
    const IO_INTERFACE_DESCRIPTION io_description_null =
    {
        test_xio_create,
        test_xio_destroy,
        NULL,
        test_xio_close,
        test_xio_send,
        test_xio_dowork,
        test_xio_setoption
    };

    // act
    XIO_HANDLE result = xio_create(&io_description_null, NULL, NULL);

    // assert
    ASSERT_IS_NULL(result);
}

/* Tests_SRS_XIO_01_004: [If any io_interface_description member is NULL, xio_create shall return NULL.] */
TEST_FUNCTION(when_concrete_xio_close_is_NULL_then_xio_create_fails)
{
    // arrange
    io_mocks mocks;
    const IO_INTERFACE_DESCRIPTION io_description_null =
    {
        test_xio_create,
        test_xio_destroy,
        test_xio_open,
        NULL,
        test_xio_send,
        test_xio_dowork,
        test_xio_setoption
    };

    // act
    XIO_HANDLE result = xio_create(&io_description_null, NULL, NULL);

    // assert
    ASSERT_IS_NULL(result);
}

/* Tests_SRS_XIO_01_004: [If any io_interface_description member is NULL, xio_create shall return NULL.] */
TEST_FUNCTION(when_concrete_xio_send_is_NULL_then_xio_create_fails)
{
    // arrange
    io_mocks mocks;
    const IO_INTERFACE_DESCRIPTION io_description_null =
    {
        test_xio_create,
        test_xio_destroy,
        test_xio_open,
        test_xio_close,
        NULL,
        test_xio_dowork,
        test_xio_setoption
    };

    // act
    XIO_HANDLE result = xio_create(&io_description_null, NULL, NULL);

    // assert
    ASSERT_IS_NULL(result);
}

/* Tests_SRS_XIO_01_004: [If any io_interface_description member is NULL, xio_create shall return NULL.] */
TEST_FUNCTION(when_concrete_xio_dowork_is_NULL_then_xio_create_fails)
{
    // arrange
    io_mocks mocks;
    const IO_INTERFACE_DESCRIPTION io_description_null =
    {
        test_xio_create,
        test_xio_destroy,
        test_xio_open,
        test_xio_close,
        test_xio_send,
        NULL,
        test_xio_setoption
    };

    // act
    XIO_HANDLE result = xio_create(&io_description_null, NULL, NULL);

    // assert
    ASSERT_IS_NULL(result);
}

/* Tests_SRS_XIO_01_004: [If any io_interface_description member is NULL, xio_create shall return NULL.] */
TEST_FUNCTION(when_concrete_xio_setoption_is_NULL_then_xio_create_fails)
{
    // arrange
    io_mocks mocks;
    const IO_INTERFACE_DESCRIPTION io_description_null =
    {
        test_xio_create,
        test_xio_destroy,
        test_xio_open,
        test_xio_close,
        test_xio_send,
        test_xio_dowork,
        NULL
    };

    // act
    XIO_HANDLE result = xio_create(&io_description_null, NULL, NULL);

    // assert
    ASSERT_IS_NULL(result);
}

/* Tests_SRS_XIO_01_017: [If allocating the memory needed for the IO interface fails then xio_create shall return NULL.] */
TEST_FUNCTION(when_allocating_memory_Fails_then_xio_create_fails)
{
    // arrange
    io_mocks mocks;
    g_fail_alloc_calls = true;

    EXPECTED_CALL(mocks, gballoc_malloc(IGNORED_NUM_ARG))
        .SetReturn((void*)NULL);

    // act
    XIO_HANDLE result = xio_create(&test_io_description, NULL, NULL);

    // assert
    ASSERT_IS_NULL(result);
}

/* xio_destroy */

/* Tests_SRS_XIO_01_005: [xio_destroy shall free all resources associated with the IO handle.] */
/* Tests_SRS_XIO_01_006: [xio_destroy shall also call the concrete_xio_destroy function that is member of the io_interface_description argument passed to xio_create, while passing as argument to concrete_xio_destroy the result of the underlying concrete_xio_create handle that was called as part of the xio_create call.] */
TEST_FUNCTION(xio_destroy_calls_concrete_xio_destroy_and_frees_memory)
{
    // arrange
    io_mocks mocks;
    XIO_HANDLE handle = xio_create(&test_io_description, NULL, NULL);
    mocks.ResetAllCalls();

    STRICT_EXPECTED_CALL(mocks, test_xio_destroy(TEST_CONCRETE_IO_HANDLE));
    EXPECTED_CALL(mocks, gballoc_free(IGNORED_PTR_ARG));

    // act
    xio_destroy(handle);

    // assert
}

/* Tests_SRS_XIO_01_007: [If handle is NULL, xio_destroy shall do nothing.] */
TEST_FUNCTION(xio_destroy_with_null_handle_does_nothing)
{
    // arrange
    io_mocks mocks;

    // act
    xio_destroy(NULL);

    // assert
}

/* xio_open */

/* Tests_SRS_XIO_01_019: SRS_XIO_01_019: [xio_open shall call the specific concrete_xio_open function specified in xio_create, passing callback function and context arguments for three events: open completed, bytes received, and IO error.] */
/* Tests_SRS_XIO_01_020: [On success, xio_open shall return 0.] */
TEST_FUNCTION(xio_open_calls_the_underlying_concrete_xio_open_and_succeeds)
{
    // arrange
    io_mocks mocks;
    XIO_HANDLE handle = xio_create(&test_io_description, NULL, NULL);
    mocks.ResetAllCalls();

    STRICT_EXPECTED_CALL(mocks, test_xio_open(TEST_CONCRETE_IO_HANDLE, test_on_io_open_complete, (void*)1, test_on_bytes_received, (void*)2, test_on_io_error, (void*)3));

    // act
    int result = xio_open(handle, test_on_io_open_complete, (void*)1, test_on_bytes_received, (void*)2, test_on_io_error, (void*)3);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    mocks.AssertActualAndExpectedCalls();

    // cleanup
    xio_destroy(handle);
}

/* Tests_SRS_XIO_01_021: [If handle is NULL, xio_open shall return a non-zero value.] */
TEST_FUNCTION(xio_open_with_NULL_handle_fails)
{
    // arrange
    io_mocks mocks;

    // act
    int result = xio_open(NULL, test_on_io_open_complete, (void*)1, test_on_bytes_received, (void*)2, test_on_io_error, (void*)3);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
}

/* Tests_SRS_XIO_01_022: [If the underlying concrete_xio_open fails, xio_open shall return a non-zero value.] */
TEST_FUNCTION(when_the_concrete_xio_open_fails_then_xio_open_fails)
{
    // arrange
    io_mocks mocks;
    XIO_HANDLE handle = xio_create(&test_io_description, NULL, NULL);
    mocks.ResetAllCalls();

    STRICT_EXPECTED_CALL(mocks, test_xio_open(TEST_CONCRETE_IO_HANDLE, test_on_io_open_complete, (void*)1, test_on_bytes_received, (void*)2, test_on_io_error, (void*)3))
        .SetReturn(1);

    // act
    int result = xio_open(handle, test_on_io_open_complete, (void*)1, test_on_bytes_received, (void*)2, test_on_io_error, (void*)3);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    mocks.AssertActualAndExpectedCalls();

    // cleanup
    xio_destroy(handle);
}

/* xio_close */

/* Tests_SRS_XIO_01_023: [xio_close shall call the specific concrete_xio_close function specified in xio_create.] */
/* Tests_SRS_XIO_01_024: [On success, xio_close shall return 0.] */
TEST_FUNCTION(xio_close_calls_the_underlying_concrete_xio_close_and_succeeds)
{
    // arrange
    io_mocks mocks;
    XIO_HANDLE handle = xio_create(&test_io_description, NULL, NULL);
    mocks.ResetAllCalls();

    STRICT_EXPECTED_CALL(mocks, test_xio_close(TEST_CONCRETE_IO_HANDLE, test_on_io_close_complete, (void*)0x4242));

    // act
    int result = xio_close(handle, test_on_io_close_complete, (void*)0x4242);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    mocks.AssertActualAndExpectedCalls();

    // cleanup
    xio_destroy(handle);
}

/* Tests_SRS_XIO_01_025: [If handle is NULL, xio_close shall return a non-zero value.] */
TEST_FUNCTION(xio_close_with_NULL_handle_fails)
{
    // arrange
    io_mocks mocks;

    // act
    int result = xio_close(NULL, test_on_io_close_complete, (void*)0x4242);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
}

/* Tests_SRS_XIO_01_026: [If the underlying concrete_xio_close fails, xio_close shall return a non-zero value.] */
TEST_FUNCTION(when_the_concrete_xio_close_fails_then_xio_close_fails)
{
    // arrange
    io_mocks mocks;
    XIO_HANDLE handle = xio_create(&test_io_description, NULL, NULL);
    mocks.ResetAllCalls();

    STRICT_EXPECTED_CALL(mocks, test_xio_close(TEST_CONCRETE_IO_HANDLE, test_on_io_close_complete, (void*)0x4242))
        .SetReturn(1);

    // act
    int result = xio_close(handle, test_on_io_close_complete, (void*)0x4242);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    mocks.AssertActualAndExpectedCalls();

    // cleanup
    xio_destroy(handle);
}

/* xio_send */

/* Tests_SRS_XIO_01_008: [xio_send shall pass the sequence of bytes pointed to by buffer to the concrete IO implementation specified in xio_create, by calling the concrete_xio_send function while passing down the buffer and size arguments to it.] */
/* Tests_SRS_XIO_01_009: [On success, xio_send shall return 0.] */
TEST_FUNCTION(xio_send_calls_the_underlying_concrete_xio_send_and_succeeds)
{
    // arrange
    io_mocks mocks;
    unsigned char send_data[] = { 0x42, 43 };
    XIO_HANDLE handle = xio_create(&test_io_description, NULL, NULL);
    mocks.ResetAllCalls();

    STRICT_EXPECTED_CALL(mocks, test_xio_send(TEST_CONCRETE_IO_HANDLE, send_data, sizeof(send_data), test_on_send_complete, (void*)0x4242));

    // act
    int result = xio_send(handle, send_data, sizeof(send_data), test_on_send_complete, (void*)0x4242);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    mocks.AssertActualAndExpectedCalls();

    // cleanup
    xio_destroy(handle);
}

/* Tests_SRS_XIO_01_010: [If handle is NULL, xio_send shall return a non-zero value.] */
TEST_FUNCTION(xio_send_with_NULL_handle_fails)
{
    // arrange
    io_mocks mocks;
    unsigned char send_data[] = { 0x42, 43 };
    mocks.ResetAllCalls();

    // act
    int result = xio_send(NULL, send_data, sizeof(send_data), test_on_send_complete, (void*)0x4242);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
}

/* Tests_SRS_XIO_01_015: [If the underlying concrete_xio_send fails, xio_send shall return a non-zero value.] */
TEST_FUNCTION(when_the_concrete_xio_send_fails_then_xio_send_fails)
{
    // arrange
    io_mocks mocks;
    unsigned char send_data[] = { 0x42, 43 };
    XIO_HANDLE handle = xio_create(&test_io_description, NULL, NULL);
    mocks.ResetAllCalls();

    STRICT_EXPECTED_CALL(mocks, test_xio_send(TEST_CONCRETE_IO_HANDLE, send_data, sizeof(send_data), test_on_send_complete, (void*)0x4242))
        .SetReturn(42);

    // act
    int result = xio_send(handle, send_data, sizeof(send_data), test_on_send_complete, (void*)0x4242);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    mocks.AssertActualAndExpectedCalls();

    // cleanup
    xio_destroy(handle);
}

/* Tests_SRS_XIO_01_011: [No error check shall be performed on buffer and size.] */
TEST_FUNCTION(xio_send_with_NULL_buffer_and_nonzero_length_passes_the_args_down_and_succeeds)
{
    // arrange
    io_mocks mocks;
    XIO_HANDLE handle = xio_create(&test_io_description, NULL, NULL);
    mocks.ResetAllCalls();

    STRICT_EXPECTED_CALL(mocks, test_xio_send(TEST_CONCRETE_IO_HANDLE, NULL, 1, test_on_send_complete, (void*)0x4242));

    // act
    int result = xio_send(handle, NULL, 1, test_on_send_complete, (void*)0x4242);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    mocks.AssertActualAndExpectedCalls();

    // cleanup
    xio_destroy(handle);
}

/* Tests_SRS_XIO_01_011: [No error check shall be performed on buffer and size.] */
TEST_FUNCTION(xio_send_with_NULL_buffer_and_zero_length_passes_the_args_down_and_succeeds)
{
    // arrange
    io_mocks mocks;
    XIO_HANDLE handle = xio_create(&test_io_description, NULL, NULL);
    mocks.ResetAllCalls();

    STRICT_EXPECTED_CALL(mocks, test_xio_send(TEST_CONCRETE_IO_HANDLE, NULL, 0, test_on_send_complete, (void*)0x4242));

    // act
    int result = xio_send(handle, NULL, 0, test_on_send_complete, (void*)0x4242);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    mocks.AssertActualAndExpectedCalls();

    // cleanup
    xio_destroy(handle);
}

/* Tests_SRS_XIO_01_011: [No error check shall be performed on buffer and size.] */
TEST_FUNCTION(xio_send_with_non_NULL_buffer_and_zero_length_passes_the_args_down_and_succeeds)
{
    // arrange
    io_mocks mocks;
    unsigned char send_data[] = { 0x42, 43 };
    XIO_HANDLE handle = xio_create(&test_io_description, NULL, NULL);
    mocks.ResetAllCalls();

    STRICT_EXPECTED_CALL(mocks, test_xio_send(TEST_CONCRETE_IO_HANDLE, send_data, 0, test_on_send_complete, (void*)0x4242));

    // act
    int result = xio_send(handle, send_data, 0, test_on_send_complete, (void*)0x4242);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    mocks.AssertActualAndExpectedCalls();

    // cleanup
    xio_destroy(handle);
}

/* xio_dowork */

/* Tests_SRS_XIO_01_012: [xio_dowork shall call the concrete IO implementation specified in xio_create, by calling the concrete_xio_dowork function.] */
TEST_FUNCTION(xio_dowork_calls_the_concrete_dowork_and_succeeds)
{
    // arrange
    io_mocks mocks;
    unsigned char send_data[] = { 0x42, 43 };
    XIO_HANDLE handle = xio_create(&test_io_description, NULL, NULL);
    mocks.ResetAllCalls();

    STRICT_EXPECTED_CALL(mocks, test_xio_dowork(TEST_CONCRETE_IO_HANDLE));

    // act
    xio_dowork(handle);

    // assert
    mocks.AssertActualAndExpectedCalls();

    // cleanup
    xio_destroy(handle);
}

/* Tests_SRS_XIO_01_018: [When the handle argument is NULL, xio_dowork shall do nothing.] */
TEST_FUNCTION(xio_dowork_with_NULL_handle_does_nothing)
{
    // arrange
    io_mocks mocks;

    // act
    xio_dowork(NULL);

    // assert
    // uMock checks the calls
}

/* Tests_SRS_XIO_03_030: [If the xio argumnent or the optionName argument is NULL, xio_setoption shall return a non-zero value.] */
TEST_FUNCTION(xio_setoption_with_NULL_handle_fails)
{
    // arrange
    io_mocks mocks;
    const char* optionName = "TheOptionName";
    const void* optionValue = (void*)1;

    mocks.ResetAllCalls();

    // act
    int result = xio_setoption(NULL, optionName, optionValue);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
}

/* Tests_SRS_XIO_03_030: [If the xio argumnent or the optionName argument is NULL, xio_setoption shall return a non-zero value.] */
TEST_FUNCTION(xio_setoption_with_NULL_optionName_fails)
{
    // arrange
    io_mocks mocks;
    const char* optionName = "TheOptionName";
    const void* optionValue = (void*)1;
    XIO_HANDLE handle = xio_create(&test_io_description, NULL, NULL);

    mocks.ResetAllCalls();

    // act
    int result = xio_setoption(handle, NULL, optionValue);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    mocks.AssertActualAndExpectedCalls();

    // cleanup
    xio_destroy(handle);
}

/* Tests_SRS_XIO_003_028: [xio_setoption shall pass the optionName and value to the concrete IO implementation specified in xio_create by invoking the concrete_xio_setoption function.] */
/* Tests_SRS_XIO_03_029: [xio_setoption shall return 0 upon success.] */
TEST_FUNCTION(xio_setoption_with_valid_args_passes_the_args_down_and_succeeds)
{
    // arrange
    io_mocks mocks;
    const char* optionName = "TheOptionName";
    const void* optionValue = (void*)1;
    XIO_HANDLE handle = xio_create(&test_io_description, NULL, NULL);

    mocks.ResetAllCalls();

    STRICT_EXPECTED_CALL(mocks, test_xio_setoption(TEST_CONCRETE_IO_HANDLE, optionName, optionValue));

    // act
    int result = xio_setoption(handle, optionName, optionValue);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    mocks.AssertActualAndExpectedCalls();

    // cleanup
    xio_destroy(handle);
}

/* Tests_SRS_XIO_03_031: [If the underlying concrete_xio_setoption fails, xio_setOption shall return a non-zero value.] */
TEST_FUNCTION(xio_setoption_fails_when_concrete_xio_setoption_fails)
{
    // arrange
    io_mocks mocks;
    const char* optionName = "TheOptionName";
    const void* optionValue = (void*)1;
    XIO_HANDLE handle = xio_create(&test_io_description, NULL, NULL);

    mocks.ResetAllCalls();

    STRICT_EXPECTED_CALL(mocks, test_xio_setoption(TEST_CONCRETE_IO_HANDLE, optionName, optionValue))
        .SetReturn(42);

    // act
    int result = xio_setoption(handle, optionName, optionValue);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    mocks.AssertActualAndExpectedCalls();

    // cleanup
    xio_destroy(handle);
}

END_TEST_SUITE(xio_unittests)
