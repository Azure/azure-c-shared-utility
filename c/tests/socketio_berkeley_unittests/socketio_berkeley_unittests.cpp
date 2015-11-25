// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <cstdint>
#include "testrunnerswitcher.h"
#include "micromock.h"
#include "micromockcharstararenullterminatedstrings.h"
#include "socketio.h"
#include "list.h"
#include "lock.h"

#undef DECLSPEC_IMPORT

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

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

static bool g_fail_alloc_calls;
static bool g_addrinfo_call_fail;
//static int g_socket_send_size_value;
static int g_socket_recv_size_value;

static const LIST_HANDLE TEST_LIST_HANDLE = (LIST_HANDLE)0x4242;
static const LIST_ITEM_HANDLE TEST_LIST_ITEM_HANDLE = (LIST_ITEM_HANDLE)0x11;
static const void** list_items = NULL;
static size_t list_item_count = 0;
static int test_socket = 0x4243;

static size_t list_head_count = 0;
static int PORT_NUM = 80;
static bool list_add_called = false;
static const char* HOSTNAME_ARG = "hostname";
static size_t callbackContext = 11;

static addrinfo TEST_ADDR_INFO = { AI_PASSIVE, AF_INET, SOCK_STREAM, IPPROTO_TCP, 128, (sockaddr*)0x11, NULL, NULL };

static const char* TEST_BUFFER_VALUE = "test_buffer_value";
#define TEST_BUFFER_SIZE    17
#define TEST_CALLBACK_CONTEXT   0x951753

TYPED_MOCK_CLASS(socketio_mocks, CGlobalMock)
{
public:
    // gballoc
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

    // list mocks
    MOCK_STATIC_METHOD_0(, LIST_HANDLE, list_create)
    MOCK_METHOD_END(LIST_HANDLE, TEST_LIST_HANDLE);
    MOCK_STATIC_METHOD_1(, void, list_destroy, LIST_HANDLE, list)
    MOCK_VOID_METHOD_END();
    MOCK_STATIC_METHOD_2(, int, list_remove, LIST_HANDLE, list, LIST_ITEM_HANDLE, item)
    MOCK_METHOD_END(int, 0);
    MOCK_STATIC_METHOD_1(, LIST_ITEM_HANDLE, list_get_head_item, LIST_HANDLE, list)
        LIST_ITEM_HANDLE listHandle = NULL;
        if (list_head_count > 0)
        {
            listHandle = TEST_LIST_ITEM_HANDLE;
            list_head_count--;
        }
    MOCK_METHOD_END(LIST_ITEM_HANDLE, listHandle);
    MOCK_STATIC_METHOD_2(, LIST_ITEM_HANDLE, list_add, LIST_HANDLE, list, const void*, item)
        const void** items = (const void**)realloc(list_items, (list_item_count + 1) * sizeof(item));
        if (items != NULL)
        {
            list_items = items;
            list_items[list_item_count++] = item;
        }
    MOCK_METHOD_END(LIST_ITEM_HANDLE, (LIST_ITEM_HANDLE)list_item_count);
    MOCK_STATIC_METHOD_1(, const void*, list_item_get_value, LIST_ITEM_HANDLE, item_handle)
        const void* resultPtr = NULL;
        if (list_add_called)
        {
            resultPtr = item_handle;
        }
    MOCK_METHOD_END(const void*, (const void*)resultPtr);
    MOCK_STATIC_METHOD_3(, LIST_ITEM_HANDLE, list_find, LIST_HANDLE, handle, LIST_MATCH_FUNCTION, match_function, const void*, match_context)
        size_t i;
        const void* found_item = NULL;
        for (i = 0; i < list_item_count; i++)
        {
            if (match_function((LIST_ITEM_HANDLE)list_items[i], match_context))
            {
                found_item = list_items[i];
                break;
            }
        }
    MOCK_METHOD_END(LIST_ITEM_HANDLE, (LIST_ITEM_HANDLE)found_item);
    MOCK_STATIC_METHOD_3(, int, list_remove_matching_item, LIST_HANDLE, handle, LIST_MATCH_FUNCTION, match_function, const void*, match_context)
        size_t i;
        int res = __LINE__;
        for (i = 0; i < list_item_count; i++)
        {
            if (match_function((LIST_ITEM_HANDLE)list_items[i], match_context))
            {
                (void)memcpy(&list_items[i], &list_items[i + 1], (list_item_count - i - 1) * sizeof(const void*));
                list_item_count--;
                res = 0;
                break;
            }
        }
    MOCK_METHOD_END(int, res);

    // ws2 mocks
    MOCK_STATIC_METHOD_3(, int, socket, int, af, int, type, int, protocol)
    MOCK_METHOD_END(int, test_socket);
    MOCK_STATIC_METHOD_1(, int, close, int, s)
    MOCK_METHOD_END(int, 0);
    MOCK_STATIC_METHOD_3(, int, connect, int, s, const struct sockaddr*, addr, socklen_t, addrlen)
    MOCK_METHOD_END(int, 0);
    MOCK_STATIC_METHOD_4(, ssize_t, recv, int, s, void*, buf, size_t, len, int, flags)
    MOCK_METHOD_END(ssize_t, 0);
    MOCK_STATIC_METHOD_4(, ssize_t, send, int, s, const void*, buf, size_t, len, int, flags)
    MOCK_METHOD_END(ssize_t, len);
    MOCK_STATIC_METHOD_4(, int, getaddrinfo, const char*, node, const char*, service, const struct addrinfo*, hints, struct addrinfo**, res)
        int callRet;
        if (!g_addrinfo_call_fail)
        {
            *res = (addrinfo*)malloc(sizeof(addrinfo));
            memcpy(*res, &TEST_ADDR_INFO, sizeof(addrinfo));
            callRet = 0;
        }
        else
        {
            *res = NULL;
            callRet = __LINE__;
        }
    MOCK_METHOD_END(int, callRet);
    //MOCK_STATIC_METHOD_3(, int, fcntl, int, fd, int, cmd, ...)
    //MOCK_METHOD_END(int, 0);
    MOCK_STATIC_METHOD_1(, void, freeaddrinfo, addrinfo*, pResult)
        if (pResult != NULL)
        {
            free(pResult);
        }
    MOCK_VOID_METHOD_END();
};

extern "C"
{
    DECLARE_GLOBAL_MOCK_METHOD_1(socketio_mocks, , void*, gballoc_malloc, size_t, size);
    DECLARE_GLOBAL_MOCK_METHOD_1(socketio_mocks, , void, gballoc_free, void*, ptr);

    DECLARE_GLOBAL_MOCK_METHOD_0(socketio_mocks, , LIST_HANDLE, list_create);
    DECLARE_GLOBAL_MOCK_METHOD_1(socketio_mocks, , void, list_destroy, LIST_HANDLE, list);
    DECLARE_GLOBAL_MOCK_METHOD_1(socketio_mocks, , LIST_ITEM_HANDLE, list_get_head_item, LIST_HANDLE, list);
    DECLARE_GLOBAL_MOCK_METHOD_2(socketio_mocks, , int, list_remove, LIST_HANDLE, list, LIST_ITEM_HANDLE, item);
    DECLARE_GLOBAL_MOCK_METHOD_2(socketio_mocks, , LIST_ITEM_HANDLE, list_add, LIST_HANDLE, list, const void*, item);
    DECLARE_GLOBAL_MOCK_METHOD_1(socketio_mocks, , const void*, list_item_get_value, LIST_ITEM_HANDLE, item_handle);
    DECLARE_GLOBAL_MOCK_METHOD_3(socketio_mocks, , LIST_ITEM_HANDLE, list_find, LIST_HANDLE, handle, LIST_MATCH_FUNCTION, match_function, const void*, match_context);
    DECLARE_GLOBAL_MOCK_METHOD_3(socketio_mocks, , int, list_remove_matching_item, LIST_HANDLE, handle, LIST_MATCH_FUNCTION, match_function, const void*, match_context);

    DECLARE_GLOBAL_MOCK_METHOD_3(socketio_mocks, , int, socket, int, af, int, type, int, protocol);
    DECLARE_GLOBAL_MOCK_METHOD_1(socketio_mocks, , int, close, int, s);
    DECLARE_GLOBAL_MOCK_METHOD_3(socketio_mocks, , int, connect, int, s, const struct sockaddr*, addr, socklen_t, addrlen);
    DECLARE_GLOBAL_MOCK_METHOD_4(socketio_mocks, , ssize_t, recv, int, s, void*, buf, size_t, len, int, flags);
    DECLARE_GLOBAL_MOCK_METHOD_4(socketio_mocks, , ssize_t, send, int, s, const void*, buf, size_t, len, int, flags);
    DECLARE_GLOBAL_MOCK_METHOD_4(socketio_mocks, , int, getaddrinfo, const char*, node, const char*, service, const struct addrinfo*, hints, struct addrinfo**, res);
    //DECLARE_GLOBAL_MOCK_METHOD_3(socketio_mocks, , int WSAAPI, fcntl, int, fd, int, cmd, ...)
    DECLARE_GLOBAL_MOCK_METHOD_1(socketio_mocks, , void, freeaddrinfo, addrinfo*, pResult);
}

MICROMOCK_MUTEX_HANDLE test_serialize_mutex;

BEGIN_TEST_SUITE(socketio_win32_unittests)

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

static void OnBytesRecieved(void* context, const void* buffer, size_t size)
{
    (void)context;
    (void)buffer;
    (void)size;
}

static void OnIoStateChanged(void* context, IO_STATE new_io_state, IO_STATE previous_io_state)
{
    (void)context;
    (void)new_io_state;
    (void)previous_io_state;
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

    SOCKETIO_CONFIG socketConfig = { HOSTNAME_ARG, PORT_NUM };

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

    SOCKETIO_CONFIG socketConfig = { HOSTNAME_ARG, PORT_NUM };

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

    SOCKETIO_CONFIG socketConfig = { HOSTNAME_ARG, PORT_NUM };
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

    SOCKETIO_CONFIG socketConfig = { HOSTNAME_ARG, PORT_NUM };

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

    SOCKETIO_CONFIG socketConfig = { HOSTNAME_ARG, PORT_NUM };
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

    SOCKETIO_CONFIG socketConfig = { HOSTNAME_ARG, PORT_NUM };
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

    SOCKETIO_CONFIG socketConfig = { HOSTNAME_ARG, PORT_NUM };
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

    SOCKETIO_CONFIG socketConfig = { HOSTNAME_ARG, PORT_NUM };
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
//    SOCKETIO_CONFIG socketConfig = { HOSTNAME_ARG, PORT_NUM };
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
    SOCKETIO_CONFIG socketConfig = { HOSTNAME_ARG, PORT_NUM };
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
    SOCKETIO_CONFIG socketConfig = { HOSTNAME_ARG, PORT_NUM };
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
    SOCKETIO_CONFIG socketConfig = { HOSTNAME_ARG, PORT_NUM };
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
//    SOCKETIO_CONFIG socketConfig = { HOSTNAME_ARG, PORT_NUM };
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
//    SOCKETIO_CONFIG socketConfig = { HOSTNAME_ARG, PORT_NUM };
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
//    SOCKETIO_CONFIG socketConfig = { HOSTNAME_ARG, PORT_NUM };
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
//    SOCKETIO_CONFIG socketConfig = { HOSTNAME_ARG, PORT_NUM };
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

END_TEST_SUITE(socketio_win32_unittests)
