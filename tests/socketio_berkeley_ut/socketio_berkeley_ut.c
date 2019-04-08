// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifdef __cplusplus
#include <cstdlib>
#include <cstddef>
#include <cstdint>
#else
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#endif

static void* my_gballoc_malloc(size_t size)
{
    return malloc(size);
}

static void my_gballoc_free(void* ptr)
{
    free(ptr);
}

#include "testrunnerswitcher.h"
#include "testrunnerswitcher.h"
#include "umock_c.h"
#include "umocktypes_charptr.h"
#include "umocktypes_stdint.h"
#include "umock_c_negative_tests.h"
#include "azure_macro_utils/macro_utils.h"

#define ENABLE_MOCKS

#include "azure_c_shared_utility/singlylinkedlist.h"
#include "azure_c_shared_utility/gballoc.h"
#include "azure_c_shared_utility/optionhandler.h"
#include "azure_c_shared_utility/gbnetwork.h"

#undef ENABLE_MOCKS

#include "azure_c_shared_utility/socketio.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

static TEST_MUTEX_HANDLE g_testByTest;
static const char* HOSTNAME_ARG = "hostname";
static int PORT_NUM = 23456;
static void* TEST_USER_CTX = (void*)0x98765;
static void* TEST_SOCKET_PTR;

// SOCKETIO_SETOPTION TESTS WERE WORKING BEFORE SWITCH TO umock_c...need to finish the conversion

#ifdef __cplusplus
extern "C"
{
#endif

    SINGLYLINKEDLIST_HANDLE real_singlylinkedlist_create(void);
    void real_singlylinkedlist_destroy(SINGLYLINKEDLIST_HANDLE list);
    LIST_ITEM_HANDLE real_singlylinkedlist_add(SINGLYLINKEDLIST_HANDLE list, const void* item);
    int real_singlylinkedlist_remove(SINGLYLINKEDLIST_HANDLE list, LIST_ITEM_HANDLE item_handle);
    LIST_ITEM_HANDLE real_singlylinkedlist_get_head_item(SINGLYLINKEDLIST_HANDLE list);
    LIST_ITEM_HANDLE real_singlylinkedlist_get_next_item(LIST_ITEM_HANDLE item_handle);
    LIST_ITEM_HANDLE real_singlylinkedlist_find(SINGLYLINKEDLIST_HANDLE list, LIST_MATCH_FUNCTION match_function, const void* match_context);
    const void* real_singlylinkedlist_item_get_value(LIST_ITEM_HANDLE item_handle);
    int real_singlylinkedlist_foreach(SINGLYLINKEDLIST_HANDLE list, LIST_ACTION_FUNCTION action_function, const void* match_context);
    int real_singlylinkedlist_remove_if(SINGLYLINKEDLIST_HANDLE list, LIST_CONDITION_FUNCTION condition_function, const void* match_context);

    int gbnetwork_fcntl(int fildes, int cmd, ...);
    int gbnetwork_fcntl(int fildes, int cmd, ...)
    {
        (void)fildes;
        (void)cmd;
        return 0;
    }

#ifdef __cplusplus
}
#endif

static int my_gbnetwork_socket(int socket_family, int socket_type, int protocol)
{
    (void)socket_family;
    (void)socket_type;
    (void)protocol;
    TEST_SOCKET_PTR = my_gballoc_malloc(1);
    return 1;
}

static int my_gbnetwork_close(int socket)
{
    (void)socket;
    my_gballoc_free(TEST_SOCKET_PTR);
    TEST_SOCKET_PTR = NULL;
    return 0;
}

static int my_gbnetwork_getaddrinfo(const char* node, const char* service, const struct addrinfo* hints, struct addrinfo** res)
{
    (void)node;
    (void)service;
    (void)hints;
    *res = (struct addrinfo*)my_gballoc_malloc(1);
    return 0;
}

static void my_gbnetwork_freeaddrinfo(struct addrinfo *res)
{
    my_gballoc_free(res);
}

#if 0

static CONCRETE_IO_HANDLE setup_socket()
{
    SOCKETIO_CONFIG socketConfig = { HOSTNAME_ARG, PORT_NUM, NULL };
    CONCRETE_IO_HANDLE ioHandle = socketio_create(&socketConfig, PrintLogFunction);
    int result = socketio_open(ioHandle, test_on_io_open_complete, &callbackContext,
        test_on_bytes_received, &callbackContext, test_on_io_error, &callbackContext);
    ASSERT_ARE_EQUAL(int, 0, result);
    return ioHandle;
}

static void verify_mocks_and_destroy_socket(CONCRETE_IO_HANDLE ioHandle)
{
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    socketio_destroy(ioHandle);
}

TEST_FUNCTION(socketio_setoption_fails_when_handle_is_null)
{
    // arrange
    int irrelevant = 1;

    // act
    int result = socketio_setoption(NULL, "tcp_keepalive", &irrelevant);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(socketio_setoption_fails_when_option_name_is_null)
{
    // arrange
    int irrelevant = 1;

    CONCRETE_IO_HANDLE ioHandle = setup_socket();

    umock_c_reset_all_calls();

    // act
    int result = socketio_setoption(ioHandle, NULL, &irrelevant);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    verify_mocks_and_destroy_socket(ioHandle);
}

TEST_FUNCTION(socketio_setoption_fails_when_value_is_null)
{
    // arrange
    CONCRETE_IO_HANDLE ioHandle = setup_socket();

    umock_c_reset_all_calls();

    // act
    int result = socketio_setoption(ioHandle, "tcp_keepalive", NULL);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    verify_mocks_and_destroy_socket(ioHandle);
}

TEST_FUNCTION(socketio_setoption_fails_when_it_receives_an_unsupported_option)
{
    // arrange
    int irrelevant = 1;

    CONCRETE_IO_HANDLE ioHandle = setup_socket();

    umock_c_reset_all_calls();

    // act
    int result = socketio_setoption(ioHandle, "unsupported_option_name", &irrelevant);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    verify_mocks_and_destroy_socket(ioHandle);
}

TEST_FUNCTION(socketio_setoption_passes_tcp_keepalive_to_setsockopt)
{
    // arrange
    CONCRETE_IO_HANDLE ioHandle = setup_socket();

    umock_c_reset_all_calls();

    int onoff = -42;

    STRICT_EXPECTED_CALL(setsockopt(*(int*)ioHandle, SOL_SOCKET, SO_KEEPALIVE,
        &onoff, sizeof(int)));

    // act
    int result = socketio_setoption(ioHandle, "tcp_keepalive", &onoff);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    verify_mocks_and_destroy_socket(ioHandle);
}

TEST_FUNCTION(socketio_setoption_passes_tcp_keepalive_time_to_setsockopt)
{
    // arrange
    CONCRETE_IO_HANDLE ioHandle = setup_socket();

    umock_c_reset_all_calls();

    int time = 3;

    STRICT_EXPECTED_CALL(setsockopt(*(int*)ioHandle, SOL_TCP, TCP_KEEPIDLE,
        &time, sizeof(int)));

    // act
    int result = socketio_setoption(ioHandle, "tcp_keepalive_time", &time);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    verify_mocks_and_destroy_socket(ioHandle);
}

TEST_FUNCTION(socketio_setoption_passes_tcp_keepalive_interval_to_setsockopt)
{
    // arrange
    CONCRETE_IO_HANDLE ioHandle = setup_socket();

    umock_c_reset_all_calls();

    int interval = 15;

    STRICT_EXPECTED_CALL(setsockopt(*(int*)ioHandle, SOL_TCP, TCP_KEEPINTVL,
        &interval, sizeof(int)));

    // act
    int result = socketio_setoption(ioHandle, "tcp_keepalive_interval", &interval);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    verify_mocks_and_destroy_socket(ioHandle);
}

#endif

static void on_bytes_recv(void* context, const unsigned char* buffer, size_t size)
{
    (void)context;
    (void)buffer;
    (void)size;
}

static void on_io_open_complete(void* context, IO_OPEN_RESULT open_result)
{
    (void)context;
    (void)open_result;
}

static void on_io_error(void* context)
{
    (void)context;
}

static void OnSendComplete(void* context, IO_SEND_RESULT send_result)
{
    (void)context;
    (void)send_result;
}

MU_DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    ASSERT_FAIL("umock_c reported error :%s", MU_ENUM_TO_STRING(UMOCK_C_ERROR_CODE, error_code));
}

/* Seems like the below tests require a full blown rewrite */

BEGIN_TEST_SUITE(socketio_berkeley_ut)

    TEST_SUITE_INITIALIZE(suite_init)
    {
        g_testByTest = TEST_MUTEX_CREATE();
        ASSERT_IS_NOT_NULL(g_testByTest);

        (void)umock_c_init(on_umock_c_error);
        //(void)umocktypes_bool_register_types();
        //(void)umocktypes_stdint_register_types();

        REGISTER_UMOCK_ALIAS_TYPE(SINGLYLINKEDLIST_HANDLE, void*);
        REGISTER_UMOCK_ALIAS_TYPE(LIST_ITEM_HANDLE, void*);

        REGISTER_UMOCK_ALIAS_TYPE(socklen_t, int);
        REGISTER_UMOCK_ALIAS_TYPE(fd_set*__restrict, void*);
        REGISTER_UMOCK_ALIAS_TYPE(struct timeval*__restrict, void*);

        REGISTER_GLOBAL_MOCK_HOOK(gballoc_malloc, my_gballoc_malloc);
        REGISTER_GLOBAL_MOCK_FAIL_RETURN(gballoc_malloc, NULL);
        REGISTER_GLOBAL_MOCK_HOOK(gballoc_free, my_gballoc_free);

        REGISTER_GLOBAL_MOCK_HOOK(gbnetwork_socket, my_gbnetwork_socket);
        REGISTER_GLOBAL_MOCK_FAIL_RETURN(gbnetwork_socket, -1);
        REGISTER_GLOBAL_MOCK_HOOK(gbnetwork_close, my_gbnetwork_close);
        REGISTER_GLOBAL_MOCK_RETURN(gbnetwork_connect, 0);
        REGISTER_GLOBAL_MOCK_FAIL_RETURN(gbnetwork_connect, __LINE__);
        REGISTER_GLOBAL_MOCK_HOOK(gbnetwork_getaddrinfo, my_gbnetwork_getaddrinfo);
        REGISTER_GLOBAL_MOCK_FAIL_RETURN(gbnetwork_getaddrinfo, __LINE__);
        REGISTER_GLOBAL_MOCK_HOOK(gbnetwork_freeaddrinfo, my_gbnetwork_freeaddrinfo);

        REGISTER_GLOBAL_MOCK_HOOK(singlylinkedlist_create, real_singlylinkedlist_create);
        REGISTER_GLOBAL_MOCK_FAIL_RETURN(singlylinkedlist_create, NULL);
        REGISTER_GLOBAL_MOCK_HOOK(singlylinkedlist_destroy, real_singlylinkedlist_destroy);
        REGISTER_GLOBAL_MOCK_HOOK(singlylinkedlist_add, real_singlylinkedlist_add);
        REGISTER_GLOBAL_MOCK_HOOK(singlylinkedlist_get_head_item, real_singlylinkedlist_get_head_item);
        REGISTER_GLOBAL_MOCK_FAIL_RETURN(singlylinkedlist_get_head_item, NULL);
        REGISTER_GLOBAL_MOCK_HOOK(singlylinkedlist_remove, real_singlylinkedlist_remove);
        REGISTER_GLOBAL_MOCK_FAIL_RETURN(singlylinkedlist_remove, __LINE__);
        REGISTER_GLOBAL_MOCK_HOOK(singlylinkedlist_item_get_value, real_singlylinkedlist_item_get_value);
        REGISTER_GLOBAL_MOCK_HOOK(singlylinkedlist_get_next_item, real_singlylinkedlist_get_next_item);
        REGISTER_GLOBAL_MOCK_HOOK(singlylinkedlist_find, real_singlylinkedlist_find);
        REGISTER_GLOBAL_MOCK_HOOK(singlylinkedlist_foreach, real_singlylinkedlist_foreach);
        REGISTER_GLOBAL_MOCK_HOOK(singlylinkedlist_remove_if, real_singlylinkedlist_remove_if);

        // REGISTER_GLOBAL_MOCK_RETURN(mqtt_client_connect, 0);
        // REGISTER_GLOBAL_MOCK_FAIL_RETURN(mqtt_client_connect, __LINE__);

    }

    TEST_SUITE_CLEANUP(suite_cleanup)
    {
        umock_c_deinit();

        TEST_MUTEX_DESTROY(g_testByTest);
    }

    TEST_FUNCTION_INITIALIZE(method_init)
    {
        if (TEST_MUTEX_ACQUIRE(g_testByTest))
        {
            ASSERT_FAIL("Could not acquire test serialization mutex.");
        }
        umock_c_reset_all_calls();
        /*list_head_count = 0;
        list_add_called = false;
        g_addrinfo_call_fail = false;
        //g_socket_send_size_value = -1;
        g_socket_recv_size_value = -1;*/
    }

    TEST_FUNCTION_CLEANUP(method_cleanup)
    {
        TEST_MUTEX_RELEASE(g_testByTest);
    }

    static void setup_socket_create_mocks(void)
    {
        STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
        STRICT_EXPECTED_CALL(singlylinkedlist_create());
        STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    }

    /* socketio_win32_create */
    TEST_FUNCTION(socketio_create_io_create_parameters_NULL_fails)
    {
        // arrange

        // act
        CONCRETE_IO_HANDLE ioHandle = socketio_create(NULL);

        // assert
        ASSERT_IS_NULL(ioHandle);
    }

    TEST_FUNCTION(socketio_create_succeeds)
    {
        // arrange
        setup_socket_create_mocks();

        SOCKETIO_CONFIG socketConfig = { HOSTNAME_ARG, PORT_NUM, NULL };

        // act
        CONCRETE_IO_HANDLE ioHandle = socketio_create(&socketConfig);

        // assert
        ASSERT_IS_NOT_NULL(ioHandle);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        socketio_destroy(ioHandle);
    }

    TEST_FUNCTION(socketio_create_list_create_fails)
    {
        int negativeTestsInitResult = umock_c_negative_tests_init();
        ASSERT_ARE_EQUAL(int, 0, negativeTestsInitResult);

        // arrange
        setup_socket_create_mocks();

        SOCKETIO_CONFIG socketConfig = { HOSTNAME_ARG, PORT_NUM, NULL };
        umock_c_negative_tests_snapshot();

        size_t count = umock_c_negative_tests_call_count();
        for (size_t index = 0; index < count; index++)
        {
            umock_c_negative_tests_reset();
            umock_c_negative_tests_fail_call(index);

            // act
            CONCRETE_IO_HANDLE ioHandle = socketio_create(&socketConfig);

            //assert
            ASSERT_IS_NULL(ioHandle, "socketio_create failure in test %zu/%zu", index, count);
        }
    }

    // socketio_win32_destroy
    TEST_FUNCTION(socketio_destroy_socket_io_NULL_succeeds)
    {
        // arrange

        // act
        socketio_destroy(NULL);

        // assert
    }

    TEST_FUNCTION(socketio_destroy_socket_no_items_succeeds)
    {
        // arrange
        SOCKETIO_CONFIG socketConfig = { HOSTNAME_ARG, PORT_NUM, NULL };
        CONCRETE_IO_HANDLE ioHandle = socketio_create(&socketConfig);
        umock_c_reset_all_calls();

        STRICT_EXPECTED_CALL(singlylinkedlist_get_head_item(IGNORED_PTR_ARG)).SetReturn(NULL);
        STRICT_EXPECTED_CALL(singlylinkedlist_destroy(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

        // act
        socketio_destroy(ioHandle);

        // assert
    }

    // TEST_FUNCTION(socketio_destroy_socket_succeeds)
    // {
    //     // arrange
    //     SOCKETIO_CONFIG socketConfig = { HOSTNAME_ARG, PORT_NUM, NULL };
    //     CONCRETE_IO_HANDLE ioHandle = socketio_create(&socketConfig);
    //     umock_c_reset_all_calls();

    //     STRICT_EXPECTED_CALL(singlylinkedlist_get_head_item(IGNORED_PTR_ARG)).SetReturn(NULL);
    //     STRICT_EXPECTED_CALL(singlylinkedlist_destroy(IGNORED_PTR_ARG));
    //     STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    //     STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    //     // act
    //     socketio_destroy(ioHandle);

    //     // assert
    // }

    TEST_FUNCTION(socketio_open_socket_io_NULL_fails)
    {
        // arrange

        // act
        int result = socketio_open(NULL, on_io_open_complete, TEST_USER_CTX, on_bytes_recv, TEST_USER_CTX, on_io_error, TEST_USER_CTX);

        // assert
        ASSERT_ARE_NOT_EQUAL(int, 0, result);
    }

    // TEST_FUNCTION(socketio_open_socket_fails)
    // {
    //     // arrange
    //     socketio_mocks mocks;

    //     SOCKETIO_CONFIG socketConfig = { HOSTNAME_ARG, PORT_NUM, NULL };
    //     CONCRETE_IO_HANDLE ioHandle = socketio_create(&socketConfig, PrintLogFunction);

    //     mocks.ResetAllCalls();

    //     STRICT_EXPECTED_CALL(socket(IGNORED_NUM_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
    //         .SetReturn(-1);

    //     // act
    //     int result = socketio_open(ioHandle, OnBytesReceived, OnIoStateChanged, &callbackContext);

    //     // assert
    //     ASSERT_ARE_NOT_EQUAL(int, 0, result);
    //     mocks.AssertActualAndExpectedCalls();

    //     socketio_destroy(ioHandle);
    // }

    /*TEST_FUNCTION(socketio_open_succeeds)
    {
        // arrange
        SOCKETIO_CONFIG socketConfig = { HOSTNAME_ARG, PORT_NUM, NULL };
        CONCRETE_IO_HANDLE ioHandle = socketio_create(&socketConfig);
        umock_c_reset_all_calls();

        STRICT_EXPECTED_CALL(socket(IGNORED_NUM_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG));
        STRICT_EXPECTED_CALL(getaddrinfo(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(connect(IGNORED_NUM_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG));
        STRICT_EXPECTED_CALL(freeaddrinfo(IGNORED_PTR_ARG));

        STRICT_EXPECTED_CALL(select(IGNORED_NUM_ARG, NULL, IGNORED_PTR_ARG, NULL, IGNORED_PTR_ARG));


        // act
        int result = socketio_open(ioHandle, on_io_open_complete, TEST_USER_CTX, on_bytes_recv, TEST_USER_CTX, on_io_error, TEST_USER_CTX);

        // assert
        ASSERT_ARE_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        socketio_destroy(ioHandle);
    }*/

    TEST_FUNCTION(socketio_close_socket_io_NULL_fails)
    {
        // arrange

        // act
        int result = socketio_close(NULL);

        // assert
        ASSERT_ARE_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    }

    #if 0
TEST_FUNCTION(socketio_close_Succeeds)
{
    // arrange
    socketio_mocks mocks;
    SOCKETIO_CONFIG socketConfig = { HOSTNAME_ARG, PORT_NUM, NULL };
    CONCRETE_IO_HANDLE ioHandle = socketio_create(&socketConfig, PrintLogFunction);

    int result = socketio_open(ioHandle, OnBytesReceived, OnIoStateChanged, &callbackContext);

    mocks.ResetAllCalls();

    STRICT_EXPECTED_CALL(close(IGNORED_NUM_ARG));

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

    int result = socketio_open(ioHandle, OnBytesReceived, OnIoStateChanged, &callbackContext);

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

    int result = socketio_open(ioHandle, OnBytesReceived, OnIoStateChanged, &callbackContext);

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
//    int result = socketio_open(ioHandle, OnBytesReceived, OnIoStateChanged, &callbackContext);
//
//    mocks.ResetAllCalls();
//
//    STRICT_EXPECTED_CALL(singlylinkedlist_get_head_item(IGNORED_PTR_ARG));
//    STRICT_EXPECTED_CALL(send(IGNORED_NUM_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG));
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
//    int result = socketio_open(ioHandle, OnBytesReceived, OnIoStateChanged, &callbackContext);
//    ASSERT_ARE_EQUAL(int, 0, result);
//
//    mocks.ResetAllCalls();
//
//    STRICT_EXPECTED_CALL(singlylinkedlist_get_head_item(IGNORED_PTR_ARG));
//    STRICT_EXPECTED_CALL(send(IGNORED_NUM_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG)).SetReturn(1);
//    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
//    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
//    STRICT_EXPECTED_CALL(singlylinkedlist_add(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
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
//    int result = socketio_open(ioHandle, OnBytesReceived, OnIoStateChanged, &callbackContext);
//
//    mocks.ResetAllCalls();
//
//    STRICT_EXPECTED_CALL(singlylinkedlist_get_head_item(IGNORED_PTR_ARG));
//    STRICT_EXPECTED_CALL(recv(IGNORED_NUM_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG));
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
//    int result = socketio_open(ioHandle, OnBytesReceived, OnIoStateChanged, &callbackContext);
//
//    mocks.ResetAllCalls();
//
//    STRICT_EXPECTED_CALL(singlylinkedlist_get_head_item(IGNORED_PTR_ARG));
//    STRICT_EXPECTED_CALL(recv(IGNORED_NUM_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
//        .CopyOutArgumentBuffer(2, "t", 1)
//        .SetReturn(1);
//    STRICT_EXPECTED_CALL(recv(IGNORED_NUM_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG));
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

END_TEST_SUITE(socketio_berkeley_ut)

