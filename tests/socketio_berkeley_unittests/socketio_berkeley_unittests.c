// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdint.h>
#include "testrunnerswitcher.h"

#define ENABLE_MOCKS

#include "azure_c_shared_utility/list.h"
#include "azure_c_shared_utility/gballoc.h"

#undef ENABLE_MOCKS

#include "azure_c_shared_utility/socketio.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

TEST_MUTEX_HANDLE test_serialize_mutex;

BEGIN_TEST_SUITE(socketio_berkeley_unittests)

/* Seems like the below tests require a full blown rewrite */

#if 0

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
    list_head_count = 0;
    list_add_called = false;
    g_addrinfo_call_fail = false;
    //g_socket_send_size_value = -1;
    g_socket_recv_size_value = -1;
}

TEST_FUNCTION_CLEANUP(method_cleanup)
{
    if (!MicroMockReleaseMutex(test_serialize_mutex))
    {
        ASSERT_FAIL("Could not release test serialization mutex.");
    }
}

static void OnBytesRecieved(void* context, const unsigned char* buffer, size_t size)
{
    (void)context;
    (void)buffer;
    (void)size;
}

static void PrintLogFunction(unsigned int options, char* format, ...)
{
    (void)options;
    (void)format;
}

static void OnSendComplete(void* context, IO_SEND_RESULT send_result)
{
    (void)context;
    (void)send_result;
}

/* socketio_win32_create */
TEST_FUNCTION(socketio_create_io_create_parameters_NULL_fails)
{
    // arrange
    socketio_mocks mocks;

    // act
    CONCRETE_IO_HANDLE ioHandle = socketio_create(NULL, PrintLogFunction);

    // assert
    ASSERT_IS_NULL(ioHandle);
}

TEST_FUNCTION(socketio_create_list_create_fails)
{
    // arrange
    socketio_mocks mocks;

    EXPECTED_CALL(mocks, gballoc_malloc(IGNORED_NUM_ARG));
    EXPECTED_CALL(mocks, list_create()).SetReturn((LIST_HANDLE)NULL);
    EXPECTED_CALL(mocks, gballoc_free(IGNORED_PTR_ARG));

    SOCKETIO_CONFIG socketConfig = { HOSTNAME_ARG, PORT_NUM, NULL };

    // act
    CONCRETE_IO_HANDLE ioHandle = socketio_create(&socketConfig, PrintLogFunction);

    // assert
    ASSERT_IS_NULL(ioHandle);
}

TEST_FUNCTION(socketio_create_succeeds)
{
    // arrange
    socketio_mocks mocks;

    EXPECTED_CALL(mocks, gballoc_malloc(IGNORED_NUM_ARG));
    EXPECTED_CALL(mocks, list_create());
    EXPECTED_CALL(mocks, gballoc_malloc(IGNORED_NUM_ARG));

    SOCKETIO_CONFIG socketConfig = { HOSTNAME_ARG, PORT_NUM, NULL };

    // act
    CONCRETE_IO_HANDLE ioHandle = socketio_create(&socketConfig, PrintLogFunction);

    // assert
    ASSERT_IS_NOT_NULL(ioHandle);
    mocks.AssertActualAndExpectedCalls();

    socketio_destroy(ioHandle);
}

// socketio_win32_destroy 
TEST_FUNCTION(socketio_destroy_socket_io_NULL_succeeds)
{
    // arrange
    socketio_mocks mocks;

    // act
    socketio_destroy(NULL);

    // assert
}

TEST_FUNCTION(socketio_destroy_socket_succeeds)
{
    // arrange
    socketio_mocks mocks;

    SOCKETIO_CONFIG socketConfig = { HOSTNAME_ARG, PORT_NUM, NULL };
    CONCRETE_IO_HANDLE ioHandle = socketio_create(&socketConfig, PrintLogFunction);

    mocks.ResetAllCalls();

    EXPECTED_CALL(mocks, close(IGNORED_NUM_ARG));
    EXPECTED_CALL(mocks, list_get_head_item(IGNORED_PTR_ARG))
        .ExpectedAtLeastTimes(2);
    EXPECTED_CALL(mocks, list_item_get_value(IGNORED_PTR_ARG));
    EXPECTED_CALL(mocks, list_remove(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    EXPECTED_CALL(mocks, list_destroy(IGNORED_PTR_ARG));
    EXPECTED_CALL(mocks, gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(mocks, gballoc_free(IGNORED_PTR_ARG));

    list_head_count = 1;

    // act
    socketio_destroy(ioHandle);

    // assert
}

TEST_FUNCTION(socketio_open_socket_io_NULL_fails)
{
    // arrange
    socketio_mocks mocks;

    SOCKETIO_CONFIG socketConfig = { HOSTNAME_ARG, PORT_NUM, NULL };

    mocks.ResetAllCalls();

    // act
    int result = socketio_open(NULL, OnBytesRecieved, OnIoStateChanged, &callbackContext);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
}

TEST_FUNCTION(socketio_open_socket_fails)
{
    // arrange
    socketio_mocks mocks;

    SOCKETIO_CONFIG socketConfig = { HOSTNAME_ARG, PORT_NUM, NULL };
    CONCRETE_IO_HANDLE ioHandle = socketio_create(&socketConfig, PrintLogFunction);

    mocks.ResetAllCalls();

    EXPECTED_CALL(mocks, socket(IGNORED_NUM_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .SetReturn(-1);

    // act
    int result = socketio_open(ioHandle, OnBytesRecieved, OnIoStateChanged, &callbackContext);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    mocks.AssertActualAndExpectedCalls();

    socketio_destroy(ioHandle);
}


TEST_FUNCTION(socketio_open_getaddrinfo_fails)
{
    // arrange
    socketio_mocks mocks;

    SOCKETIO_CONFIG socketConfig = { HOSTNAME_ARG, PORT_NUM, NULL };
    CONCRETE_IO_HANDLE ioHandle = socketio_create(&socketConfig, PrintLogFunction);

    mocks.ResetAllCalls();

    g_addrinfo_call_fail = true;
    EXPECTED_CALL(mocks, socket(IGNORED_NUM_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG));
    EXPECTED_CALL(mocks, getaddrinfo(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    EXPECTED_CALL(mocks, close(IGNORED_NUM_ARG));

    // act
    int result = socketio_open(ioHandle, OnBytesRecieved, OnIoStateChanged, &callbackContext);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    mocks.AssertActualAndExpectedCalls();

    socketio_destroy(ioHandle);
}

TEST_FUNCTION(socketio_open_connect_fails)
{
    // arrange
    socketio_mocks mocks;

    SOCKETIO_CONFIG socketConfig = { HOSTNAME_ARG, PORT_NUM, NULL };
    CONCRETE_IO_HANDLE ioHandle = socketio_create(&socketConfig, PrintLogFunction);

    mocks.ResetAllCalls();

    EXPECTED_CALL(mocks, socket(IGNORED_NUM_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG));
    EXPECTED_CALL(mocks, getaddrinfo(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    EXPECTED_CALL(mocks, connect(IGNORED_NUM_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG))
        .SetReturn(-1);
    EXPECTED_CALL(mocks, close(IGNORED_NUM_ARG));
    EXPECTED_CALL(mocks, freeaddrinfo(IGNORED_PTR_ARG));

    // act
    int result = socketio_open(ioHandle, OnBytesRecieved, OnIoStateChanged, &callbackContext);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    mocks.AssertActualAndExpectedCalls();

    socketio_destroy(ioHandle);
}

TEST_FUNCTION(socketio_open_ioctlsocket_fails)
{
    // arrange
    socketio_mocks mocks;

    SOCKETIO_CONFIG socketConfig = { HOSTNAME_ARG, PORT_NUM, NULL };
    CONCRETE_IO_HANDLE ioHandle = socketio_create(&socketConfig, PrintLogFunction);

    mocks.ResetAllCalls();

    EXPECTED_CALL(mocks, socket(IGNORED_NUM_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG));
    EXPECTED_CALL(mocks, getaddrinfo(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    EXPECTED_CALL(mocks, connect(IGNORED_NUM_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    //EXPECTED_CALL(mocks, fcntl(IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG))
    //    .SetReturn(-1);
    EXPECTED_CALL(mocks, freeaddrinfo(IGNORED_PTR_ARG));
    EXPECTED_CALL(mocks, close(IGNORED_NUM_ARG));

    // act
    int result = socketio_open(ioHandle, OnBytesRecieved, OnIoStateChanged, &callbackContext);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    mocks.AssertActualAndExpectedCalls();

    socketio_destroy(ioHandle);
}

//TEST_FUNCTION(socketio_open_succeeds)
//{
//    // arrange
//    socketio_mocks mocks;
//
//    SOCKETIO_CONFIG socketConfig = { HOSTNAME_ARG, PORT_NUM, NULL };
//    CONCRETE_IO_HANDLE ioHandle = socketio_create(&socketConfig, PrintLogFunction);
//
//    mocks.ResetAllCalls();
//
//    EXPECTED_CALL(mocks, socket(IGNORED_NUM_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG));
//    EXPECTED_CALL(mocks, getaddrinfo(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
//    EXPECTED_CALL(mocks, connect(IGNORED_NUM_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG));
//    EXPECTED_CALL(mocks, freeaddrinfo(IGNORED_PTR_ARG));
//
//    // act
//    int result = socketio_open(ioHandle, OnBytesRecieved, OnIoStateChanged, &callbackContext);
//
//    // assert
//    ASSERT_ARE_EQUAL(int, 0, result);
//    mocks.AssertActualAndExpectedCalls();
//
//    socketio_destroy(ioHandle);
//}

TEST_FUNCTION(socketio_close_socket_io_NULL_fails)
{
    // arrange
    socketio_mocks mocks;

    // act
    int result = socketio_close(NULL);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
}

TEST_FUNCTION(socketio_close_Succeeds)
{
    // arrange
    socketio_mocks mocks;
    SOCKETIO_CONFIG socketConfig = { HOSTNAME_ARG, PORT_NUM, NULL };
    CONCRETE_IO_HANDLE ioHandle = socketio_create(&socketConfig, PrintLogFunction);

    int result = socketio_open(ioHandle, OnBytesRecieved, OnIoStateChanged, &callbackContext);

    mocks.ResetAllCalls();

    EXPECTED_CALL(mocks, close(IGNORED_NUM_ARG));

    // act
    result = socketio_close(ioHandle);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
}

TEST_FUNCTION(socketio_send_socket_io_fails)
{
    // arrange
    socketio_mocks mocks;

    // act
    int result = socketio_send(NULL, (const void*)TEST_BUFFER_VALUE, TEST_BUFFER_SIZE, OnSendComplete, (void*)TEST_CALLBACK_CONTEXT);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
}

TEST_FUNCTION(socketio_send_buffer_NULL_fails)
{
    // arrange
    socketio_mocks mocks;
    SOCKETIO_CONFIG socketConfig = { HOSTNAME_ARG, PORT_NUM, NULL };
    CONCRETE_IO_HANDLE ioHandle = socketio_create(&socketConfig, PrintLogFunction);

    int result = socketio_open(ioHandle, OnBytesRecieved, OnIoStateChanged, &callbackContext);

    mocks.ResetAllCalls();

    // act
    result = socketio_send(ioHandle, NULL, TEST_BUFFER_SIZE, OnSendComplete, (void*)TEST_CALLBACK_CONTEXT);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
}

TEST_FUNCTION(socketio_send_size_zero_fails)
{
    // arrange
    socketio_mocks mocks;
    SOCKETIO_CONFIG socketConfig = { HOSTNAME_ARG, PORT_NUM, NULL };
    CONCRETE_IO_HANDLE ioHandle = socketio_create(&socketConfig, PrintLogFunction);

    int result = socketio_open(ioHandle, OnBytesRecieved, OnIoStateChanged, &callbackContext);

    mocks.ResetAllCalls();

    // act
    result = socketio_send(ioHandle, (const void*)TEST_BUFFER_VALUE, 0, OnSendComplete, (void*)TEST_CALLBACK_CONTEXT);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
}

// TBD:  To be implemented when fcntl is mocked
//TEST_FUNCTION(socketio_send_succeeds)
//{
//    // arrange
//    socketio_mocks mocks;
//    SOCKETIO_CONFIG socketConfig = { HOSTNAME_ARG, PORT_NUM, NULL };
//    CONCRETE_IO_HANDLE ioHandle = socketio_create(&socketConfig, PrintLogFunction);
//
//    int result = socketio_open(ioHandle, OnBytesRecieved, OnIoStateChanged, &callbackContext);
//
//    mocks.ResetAllCalls();
//
//    EXPECTED_CALL(mocks, list_get_head_item(IGNORED_PTR_ARG));
//    EXPECTED_CALL(mocks, send(IGNORED_NUM_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG));
//
//    // act
//    result = socketio_send(ioHandle, (const void*)TEST_BUFFER_VALUE, TEST_BUFFER_SIZE, OnSendComplete, (void*)TEST_CALLBACK_CONTEXT);
//
//    // assert
//    ASSERT_ARE_EQUAL(int, 0, result);
//    mocks.AssertActualAndExpectedCalls();
//
//    socketio_destroy(ioHandle);
//}

// TBD:  To be implemented when fcntl is mocked
//TEST_FUNCTION(socketio_send_returns_1_succeeds)
//{
//    // arrange
//    socketio_mocks mocks;
//    SOCKETIO_CONFIG socketConfig = { HOSTNAME_ARG, PORT_NUM, NULL };
//    CONCRETE_IO_HANDLE ioHandle = socketio_create(&socketConfig, PrintLogFunction);
//
//    int result = socketio_open(ioHandle, OnBytesRecieved, OnIoStateChanged, &callbackContext);
//    ASSERT_ARE_EQUAL(int, 0, result);
//
//    mocks.ResetAllCalls();
//
//    EXPECTED_CALL(mocks, list_get_head_item(IGNORED_PTR_ARG));
//    EXPECTED_CALL(mocks, send(IGNORED_NUM_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG)).SetReturn(1);
//    EXPECTED_CALL(mocks, gballoc_malloc(IGNORED_NUM_ARG));
//    EXPECTED_CALL(mocks, gballoc_malloc(IGNORED_NUM_ARG));
//    EXPECTED_CALL(mocks, list_add(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
//
//    // act
//    result = socketio_send(ioHandle, (const void*)TEST_BUFFER_VALUE, TEST_BUFFER_SIZE, OnSendComplete, (void*)TEST_CALLBACK_CONTEXT);
//
//    // assert
//    ASSERT_ARE_EQUAL(int, 0, result);
//    mocks.AssertActualAndExpectedCalls();
//
//    socketio_destroy(ioHandle);
//}

TEST_FUNCTION(socketio_dowork_socket_io_NULL_fails)
{
    // arrange
    socketio_mocks mocks;

    // act
    socketio_dowork(NULL);

    // assert
}

// TBD:  To be implemented when fcntl is mocked
//TEST_FUNCTION(socketio_dowork_succeeds)
//{
//    // arrange
//    socketio_mocks mocks;
//    SOCKETIO_CONFIG socketConfig = { HOSTNAME_ARG, PORT_NUM, NULL };
//    CONCRETE_IO_HANDLE ioHandle = socketio_create(&socketConfig, PrintLogFunction);
//
//    int result = socketio_open(ioHandle, OnBytesRecieved, OnIoStateChanged, &callbackContext);
//
//    mocks.ResetAllCalls();
//
//    EXPECTED_CALL(mocks, list_get_head_item(IGNORED_PTR_ARG));
//    EXPECTED_CALL(mocks, recv(IGNORED_NUM_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG));
//
//    // act
//    socketio_dowork(ioHandle);
//
//    // assert
//    mocks.AssertActualAndExpectedCalls();
//
//    socketio_destroy(ioHandle);
//}

// TBD:  To be implemented when fcntl is mocked
//TEST_FUNCTION(socketio_dowork_recv_bytes_succeeds)
//{
//    // arrange
//    socketio_mocks mocks;
//    SOCKETIO_CONFIG socketConfig = { HOSTNAME_ARG, PORT_NUM, NULL };
//    CONCRETE_IO_HANDLE ioHandle = socketio_create(&socketConfig, PrintLogFunction);
//
//    int result = socketio_open(ioHandle, OnBytesRecieved, OnIoStateChanged, &callbackContext);
//
//    mocks.ResetAllCalls();
//
//    EXPECTED_CALL(mocks, list_get_head_item(IGNORED_PTR_ARG));
//    EXPECTED_CALL(mocks, recv(IGNORED_NUM_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
//        .CopyOutArgumentBuffer(2, "t", 1)
//        .SetReturn(1);
//    EXPECTED_CALL(mocks, recv(IGNORED_NUM_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG));
//
//    // act
//    socketio_dowork(ioHandle);
//
//    // assert
//    mocks.AssertActualAndExpectedCalls();
//
//    socketio_destroy(ioHandle);
//}

#endif

END_TEST_SUITE(socketio_berkeley_unittests)

