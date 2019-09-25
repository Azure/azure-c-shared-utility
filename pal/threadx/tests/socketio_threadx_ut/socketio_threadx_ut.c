// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifdef __cplusplus
#include <cstdint>
#else
#include <stdint.h>
#endif

#include "testrunnerswitcher.h"

#undef DECLSPEC_IMPORT

#pragma warning(disable: 4273)

#include "nx_api.h"

static size_t currentmalloc_call;
static size_t whenShallmalloc_fail;

void* my_gballoc_malloc(size_t size)
{
    void* result;
    currentmalloc_call++;
    if (whenShallmalloc_fail > 0)
    {
        if (currentmalloc_call == whenShallmalloc_fail)
        {
            result = NULL;
        }
        else
        {
            result = malloc(size);
        }
    }
    else
    {
        result = malloc(size);
    }
    return result;
}

void my_gballoc_free(void* ptr)
{
    free(ptr);
}

#define ENABLE_MOCKS
#include "azure_c_shared_utility/optionhandler.h"
#undef ENABLE_MOCKS

#include "azure_c_shared_utility/optimize_size.h"
#include "azure_c_shared_utility/shared_util_options.h"
#include "azure_c_shared_utility/socketio.h"
#include "azure_c_shared_utility/tcpsocketconnection_c.h"

#define ENABLE_MOCKS

#include "umock_c/umock_c.h"
#include "umock_c/umocktypes_charptr.h"
#include "azure_c_shared_utility/singlylinkedlist.h"
static bool g_addrinfo_call_fail;
static int g_socket_recv_size_value;

extern NX_PACKET_POOL pool_0;
static int bytes_received = 0;
static char large_data[1600];
static const SINGLYLINKEDLIST_HANDLE TEST_SINGLYLINKEDLIST_HANDLE = (SINGLYLINKEDLIST_HANDLE)0x4242;
static const LIST_ITEM_HANDLE TEST_LIST_ITEM_HANDLE = (LIST_ITEM_HANDLE)0x11;
static const void** list_items = NULL;
static size_t list_item_count = 0;
static size_t list_head_count = 0;
static bool singlylinkedlist_add_called = false;
static size_t callbackContext = 11;

static const char* TEST_BUFFER_VALUE = "test_buffer_value";

#define PORT_NUM 8883
#define HOSTNAME_ARG "global.azure-devices-provisioning.net"

#define TEST_BUFFER_SIZE    17
#define TEST_CALLBACK_CONTEXT   0x951753

typedef enum IO_STATE_TAG
{
    IO_STATE_CLOSED,
    IO_STATE_OPENING,
    IO_STATE_OPEN,
    IO_STATE_CLOSING,
    IO_STATE_ERROR
} IO_STATE;

typedef struct SOCKET_IO_INSTANCE_TAG
{
    TCPSOCKETCONNECTION_HANDLE tcp_socket_connection;
    ON_BYTES_RECEIVED on_bytes_received;
    ON_IO_ERROR on_io_error;
    void* on_bytes_received_context;
    void* on_io_error_context;
    char* hostname;
    NXD_ADDRESS ip_address;
    int port;
    IO_STATE io_state;
    SINGLYLINKEDLIST_HANDLE pending_io_list;
    NX_TCP_SOCKET threadx_tcp_socket;
} SOCKET_IO_INSTANCE;

LIST_ITEM_HANDLE my_singlylinkedlist_get_head_item(SINGLYLINKEDLIST_HANDLE list)
{
    LIST_ITEM_HANDLE listHandle = NULL;
    (void)list;
    if (list_item_count > 0)
    {
        listHandle = (LIST_ITEM_HANDLE)list_items[0];
        list_item_count--;
    }
    return listHandle;
}

LIST_ITEM_HANDLE my_singlylinkedlist_add(SINGLYLINKEDLIST_HANDLE list, const void* item)
{
    const void** items = (const void**)realloc((void*)list_items, (list_item_count + 1) * sizeof(item));
    (void)list;
    if (items != NULL)
    {
        list_items = items;
        list_items[list_item_count++] = item;
    }
    singlylinkedlist_add_called = true;
    return (LIST_ITEM_HANDLE)list_item_count;
}

const void* my_singlylinkedlist_item_get_value(LIST_ITEM_HANDLE item_handle)
{
    const void* resultPtr = NULL;
    if (singlylinkedlist_add_called)
    {
        resultPtr = item_handle;
    }

    return (const void*)resultPtr;
}

LIST_ITEM_HANDLE my_singlylinkedlist_find(SINGLYLINKEDLIST_HANDLE handle, LIST_MATCH_FUNCTION match_function, const void* match_context)
{
    size_t i;
    const void* found_item = NULL;
    (void)handle;
    for (i = 0; i < list_item_count; i++)
    {
        if (match_function((LIST_ITEM_HANDLE)list_items[i], match_context))
        {
            found_item = list_items[i];
            break;
        }
    }

    return (LIST_ITEM_HANDLE)found_item;
}

void my_singlylinkedlist_destroy(SINGLYLINKEDLIST_HANDLE handle)
{
    (void)handle;
    free((void*)list_items);
    list_items = NULL;
}

static void test_on_bytes_received(void* context, const unsigned char* buffer, size_t size)
{
    (void)context;
    (void)buffer;
    (void)size;
    bytes_received = 1;
}

static void test_on_io_open_complete(void* context, IO_OPEN_RESULT open_result)
{
    (void)context;
    (void)open_result;
}

static void test_on_io_close_complete(void* context)
{
    (void)context;
}

static void test_on_io_error(void* context)
{
    (void)context;
}

/* after this point malloc is gballoc */
#include "azure_c_shared_utility/gballoc.h"

static TEST_MUTEX_HANDLE g_testByTest;

MU_DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    ASSERT_FAIL("umock_c reported error :%s", MU_ENUM_TO_STRING(UMOCK_C_ERROR_CODE, error_code));
}

BEGIN_TEST_SUITE(socketio_threadx_unittests)

TEST_SUITE_INITIALIZE(suite_init)
{
    int result;
    g_testByTest = TEST_MUTEX_CREATE();
    ASSERT_IS_NOT_NULL(g_testByTest);

    umock_c_init(on_umock_c_error);

    result = umocktypes_charptr_register_types();
    ASSERT_ARE_EQUAL(int, 0, result);

    REGISTER_UMOCK_ALIAS_TYPE(CONCRETE_IO_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(SINGLYLINKEDLIST_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(LIST_ITEM_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(SOCKET, void*);
    REGISTER_UMOCK_ALIAS_TYPE(PCSTR, char*);
    REGISTER_UMOCK_ALIAS_TYPE(DWORD, unsigned long);
    REGISTER_UMOCK_ALIAS_TYPE(LPVOID, void*);
    REGISTER_UMOCK_ALIAS_TYPE(LPDWORD, void*);

    REGISTER_GLOBAL_MOCK_RETURN(singlylinkedlist_remove, 0);
    REGISTER_GLOBAL_MOCK_RETURN(singlylinkedlist_create, TEST_SINGLYLINKEDLIST_HANDLE);
    REGISTER_GLOBAL_MOCK_HOOK(gballoc_malloc, my_gballoc_malloc);
    REGISTER_GLOBAL_MOCK_HOOK(gballoc_free, my_gballoc_free);
    REGISTER_GLOBAL_MOCK_HOOK(singlylinkedlist_get_head_item, my_singlylinkedlist_get_head_item);
    REGISTER_GLOBAL_MOCK_HOOK(singlylinkedlist_add, my_singlylinkedlist_add);
    REGISTER_GLOBAL_MOCK_HOOK(singlylinkedlist_item_get_value, my_singlylinkedlist_item_get_value);
    REGISTER_GLOBAL_MOCK_HOOK(singlylinkedlist_find, my_singlylinkedlist_find);
    REGISTER_GLOBAL_MOCK_HOOK(singlylinkedlist_destroy, my_singlylinkedlist_destroy);
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

    currentmalloc_call = 0;
    whenShallmalloc_fail = 0;
    list_head_count = 0;
    singlylinkedlist_add_called = false;
    g_addrinfo_call_fail = false;
    //g_socket_send_size_value = -1;
    g_socket_recv_size_value = -1;
}

TEST_FUNCTION_CLEANUP(method_cleanup)
{
    TEST_MUTEX_RELEASE(g_testByTest);
}

static void OnSendComplete(void* context, IO_SEND_RESULT send_result)
{
    (void)context;
    (void)send_result;
}

/* socketio_threadx_create */
TEST_FUNCTION(socketio_create_io_create_parameters_NULL_fails)
{
    // arrange

    // act
    CONCRETE_IO_HANDLE ioHandle = socketio_create(NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(ioHandle);
}

TEST_FUNCTION(socketio_create_singlylinkedlist_create_fails)
{
    // arrange
    SOCKETIO_CONFIG socketConfig = { HOSTNAME_ARG, PORT_NUM, NULL };
    CONCRETE_IO_HANDLE ioHandle;

    EXPECTED_CALL(singlylinkedlist_create()).SetReturn((SINGLYLINKEDLIST_HANDLE)NULL);

    // act
    ioHandle = socketio_create(&socketConfig);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(ioHandle);
}

TEST_FUNCTION(socketio_create_succeeds)
{
    // arrange
    SOCKETIO_CONFIG socketConfig = { HOSTNAME_ARG, PORT_NUM, NULL };
    CONCRETE_IO_HANDLE ioHandle;

    EXPECTED_CALL(singlylinkedlist_create());

    // act
    ioHandle = socketio_create(&socketConfig);

    // assert
    ASSERT_IS_NOT_NULL(ioHandle);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    socketio_destroy(ioHandle);
}

// socketio_threadx_destroy
TEST_FUNCTION(socketio_destroy_socket_io_NULL_succeeds)
{
    // arrange

    // act
    socketio_destroy(NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(socketio_destroy_socket_succeeds)
{
    // arrange
    SOCKETIO_CONFIG socketConfig = { HOSTNAME_ARG, PORT_NUM, NULL };
    CONCRETE_IO_HANDLE ioHandle = socketio_create(&socketConfig);
    socketio_open(ioHandle, test_on_io_open_complete, &callbackContext, test_on_bytes_received, &callbackContext, test_on_io_error, &callbackContext);

    // act
    socketio_destroy(ioHandle);

    // FIXME XWH: need to check status
}

TEST_FUNCTION(socketio_open_socket_io_NULL_fails)
{
    // arrange

    // act
    int result = socketio_open(NULL, test_on_io_open_complete, &callbackContext, test_on_bytes_received, &callbackContext, test_on_io_error, &callbackContext);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
}

TEST_FUNCTION(socketio_open_getaddrinfo_fails)
{
    // arrange
    int result;
    SOCKETIO_CONFIG socketConfig = { "null", PORT_NUM, NULL };
    CONCRETE_IO_HANDLE ioHandle = socketio_create(&socketConfig);

    // act
    result = socketio_open(ioHandle, test_on_io_open_complete, &callbackContext, test_on_bytes_received, &callbackContext, test_on_io_error, &callbackContext);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);

    // cleanup
    socketio_destroy(ioHandle);
}

TEST_FUNCTION(socketio_open_succeeds)
{
    // arrange
    int result;
    SOCKETIO_CONFIG socketConfig = { HOSTNAME_ARG, PORT_NUM, NULL };
    CONCRETE_IO_HANDLE ioHandle = socketio_create(&socketConfig);

    // act
    result = socketio_open(ioHandle, test_on_io_open_complete, &callbackContext, test_on_bytes_received, &callbackContext, test_on_io_error, &callbackContext);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);

    // cleanup
    socketio_destroy(ioHandle);
}

TEST_FUNCTION(socketio_close_socket_io_NULL_fails)
{
    // arrange
    // act
    int result = socketio_close(NULL, test_on_io_close_complete, (void*)0x4242);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
}

TEST_FUNCTION(socketio_close_Succeeds)
{
    // arrange
    SOCKETIO_CONFIG socketConfig = { HOSTNAME_ARG, PORT_NUM, NULL };
    CONCRETE_IO_HANDLE ioHandle = socketio_create(&socketConfig);

    int result = socketio_open(ioHandle, test_on_io_open_complete, &callbackContext, test_on_bytes_received, &callbackContext, test_on_io_error, &callbackContext);

    // act
    result = socketio_close(ioHandle, test_on_io_close_complete, (void*)0x4242);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);

    // cleanup
    socketio_destroy(ioHandle);
}

TEST_FUNCTION(socketio_send_socket_io_fails)
{
    // arrange

    // act
    int result = socketio_send(NULL, (const void*)TEST_BUFFER_VALUE, TEST_BUFFER_SIZE, OnSendComplete, (void*)TEST_CALLBACK_CONTEXT);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
}

TEST_FUNCTION(socketio_send_buffer_NULL_fails)
{
    // arrange
    SOCKETIO_CONFIG socketConfig = { HOSTNAME_ARG, PORT_NUM, NULL };
    CONCRETE_IO_HANDLE ioHandle = socketio_create(&socketConfig);

    int result = socketio_open(ioHandle, test_on_io_open_complete, &callbackContext, test_on_bytes_received, &callbackContext, test_on_io_error, &callbackContext);

    umock_c_reset_all_calls();

    // act
    result = socketio_send(ioHandle, NULL, TEST_BUFFER_SIZE, OnSendComplete, (void*)TEST_CALLBACK_CONTEXT);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);

    // cleanup
    socketio_destroy(ioHandle);
}

TEST_FUNCTION(socketio_send_size_zero_fails)
{
    // arrange
    SOCKETIO_CONFIG socketConfig = { HOSTNAME_ARG, PORT_NUM, NULL };
    CONCRETE_IO_HANDLE ioHandle = socketio_create(&socketConfig);

    int result = socketio_open(ioHandle, test_on_io_open_complete, &callbackContext, test_on_bytes_received, &callbackContext, test_on_io_error, &callbackContext);

    umock_c_reset_all_calls();

    // act
    result = socketio_send(ioHandle, (const void*)TEST_BUFFER_VALUE, 0, OnSendComplete, (void*)TEST_CALLBACK_CONTEXT);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);

    // cleanup
    socketio_destroy(ioHandle);
}

TEST_FUNCTION(socketio_send_succeeds)
{
    // arrange
    SOCKETIO_CONFIG socketConfig = { HOSTNAME_ARG, PORT_NUM, NULL };
    CONCRETE_IO_HANDLE ioHandle = socketio_create(&socketConfig);

    int result = socketio_open(ioHandle, test_on_io_open_complete, &callbackContext, test_on_bytes_received, &callbackContext, test_on_io_error, &callbackContext);

    // act
    result = socketio_send(ioHandle, (const void*)TEST_BUFFER_VALUE, TEST_BUFFER_SIZE, OnSendComplete, (void*)TEST_CALLBACK_CONTEXT);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);

    // cleanup
    socketio_destroy(ioHandle);
}

TEST_FUNCTION(socketio_send_data_pending_succeeds)
{
NX_PACKET *packet_list = NX_NULL;
NX_PACKET *packet_ptr = NX_NULL;
NX_PACKET *new_packet = NX_NULL;

    // arrange
    SOCKETIO_CONFIG socketConfig = { HOSTNAME_ARG, PORT_NUM, NULL };
    CONCRETE_IO_HANDLE ioHandle = socketio_create(&socketConfig);

    int result = socketio_open(ioHandle, test_on_io_open_complete, &callbackContext, test_on_bytes_received, &callbackContext, test_on_io_error, &callbackContext);

    umock_c_reset_all_calls();

    EXPECTED_CALL(singlylinkedlist_get_head_item(IGNORED_PTR_ARG));
    EXPECTED_CALL(singlylinkedlist_add(IGNORED_PTR_ARG, IGNORED_PTR_ARG));

    while (pool_0.nx_packet_pool_available - 1)
    {
        nx_packet_allocate(&pool_0, &new_packet, NX_TCP_PACKET, NX_NO_WAIT);
        if (!packet_list)
        {
            packet_ptr = new_packet;
            packet_list = packet_ptr;
        }
        else
        {
            packet_ptr -> nx_packet_queue_next = new_packet;
            packet_ptr = packet_ptr -> nx_packet_queue_next;
        }
    }

    // act
    result = socketio_send(ioHandle, (const void*)large_data, sizeof(large_data), OnSendComplete, (void*)TEST_CALLBACK_CONTEXT);

    while (packet_list)
    {
        packet_ptr = packet_list -> nx_packet_queue_next;
        nx_packet_release(packet_list);
        packet_list = packet_ptr;
    }

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    list_head_count = list_item_count;
    socketio_destroy(ioHandle);
}

TEST_FUNCTION(socketio_dowork_socket_io_NULL_fails)
{
    // arrange

    // act
    socketio_dowork(NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(socketio_dowork_succeeds)
{
    // arrange
    SOCKETIO_CONFIG socketConfig = { HOSTNAME_ARG, PORT_NUM, NULL };
    CONCRETE_IO_HANDLE ioHandle = socketio_create(&socketConfig);

    (void)socketio_open(ioHandle, test_on_io_open_complete, &callbackContext, test_on_bytes_received, &callbackContext, test_on_io_error, &callbackContext);

    // reset bytes received flag
    bytes_received = 0;

    // act
    socketio_dowork(ioHandle);

    // assert
    ASSERT_ARE_EQUAL(int , 0, bytes_received);

    socketio_destroy(ioHandle);
}

TEST_FUNCTION(socketio_dowork_recv_bytes_succeeds)
{
NX_PACKET *packet_ptr;
UCHAR tcp_data[20] = { 0x18, 0x3e, 0x00, 0x50, 0x62, 0xf3, 0xa5, 0x46, 
                       0x54, 0x7e, 0x0c, 0xe7, 0x00, 0x01, 0x10, 0x50, 
                       0xf3, 0x3a, 0x00, 0x00};

    // arrange
    SOCKETIO_CONFIG socketConfig = { HOSTNAME_ARG, PORT_NUM, NULL };
    CONCRETE_IO_HANDLE ioHandle = socketio_create(&socketConfig);
    SOCKET_IO_INSTANCE* socket_io_instance = (SOCKET_IO_INSTANCE*)ioHandle;

    umock_c_reset_all_calls();

    (void)socketio_open(ioHandle, test_on_io_open_complete, &callbackContext, test_on_bytes_received, &callbackContext, test_on_io_error, &callbackContext);

    nx_packet_allocate(&pool_0, &packet_ptr, NX_TCP_PACKET, NX_NO_WAIT);
    nx_packet_data_append(packet_ptr, tcp_data, sizeof(tcp_data), &pool_0, NX_NO_WAIT);
    packet_ptr -> nx_packet_queue_next = (NX_PACKET *)0xBBBBBBBB;
    socket_io_instance -> threadx_tcp_socket.nx_tcp_socket_receive_queue_head = packet_ptr;
    socket_io_instance -> threadx_tcp_socket.nx_tcp_socket_receive_queue_tail = packet_ptr;
    socket_io_instance -> threadx_tcp_socket.nx_tcp_socket_receive_queue_count++;

    // reset bytes received flag
    bytes_received = 0;
    EXPECTED_CALL(singlylinkedlist_get_head_item(IGNORED_PTR_ARG));

    // act
    socketio_dowork(ioHandle);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int , 1, bytes_received);

    socketio_destroy(ioHandle);
}

END_TEST_SUITE(socketio_threadx_unittests)
