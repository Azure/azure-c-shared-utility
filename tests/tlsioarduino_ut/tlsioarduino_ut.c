// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.


#include <stdlib.h>
#ifdef _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

static int currentmalloc_call = 0;
static int whenShallmalloc_fail = 0;

void* my_gballoc_malloc(size_t size)
{
	void* result;
	currentmalloc_call++;
	if (whenShallmalloc_fail > 0)
	{
		if (currentmalloc_call >= whenShallmalloc_fail)
		{
			currentmalloc_call--;
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

void* my_gballoc_realloc(void* ptr, size_t size)
{
	void* newptr = realloc(ptr, size);

	if (ptr == NULL)
	{
		currentmalloc_call++;
	}

	return newptr;
}

void my_gballoc_free(void* ptr)
{
	currentmalloc_call--;
	free(ptr);
}

#include <stddef.h>
#include <time.h>

#include "testrunnerswitcher.h"
#include "umock_c.h"
#include "umocktypes_charptr.h"
#include "azure_c_shared_utility/macro_utils.h"
#include "azure_c_shared_utility/xio.h"
#include "azure_c_shared_utility/tlsio.h"
#define ENABLE_MOCKS
#include "azure_c_shared_utility/gballoc.h"
#include "../../adapters/sslClient_arduino.h"
#include "azure_c_shared_utility/optionhandler.h"
#undef ENABLE_MOCKS
#include "../../adapters/tlsio_arduino.h"

#define TEST_CREATE_CONNECTION_HOST_NAME (const char*)"https://test.azure-devices.net"
#define TEST_CREATE_CONNECTION_PORT (int)443

static const TLSIO_CONFIG tlsioConfig = { TEST_CREATE_CONNECTION_HOST_NAME, TEST_CREATE_CONNECTION_PORT };

static const int ConnectReturnList_true[1] = { (const int)true };
static const int ConnectReturnList_false[1] = { (const int)false };

static const bool ConnectedReturnList_t[1] = { true };
static const bool ConnectedReturnList_f[1] = { false };
static const bool ConnectedReturnList_ft[2] = { false, true };
static const bool ConnectedReturnList_ff[2] = { false, false };
static const bool ConnectedReturnList_ftf[3] = { false, true, false };
static const bool ConnectedReturnList_ftt[3] = { false, true, true };
static const bool ConnectedReturnList_ftft[4] = { false, true, false, true };
static const bool ConnectedReturnList_ftff[4] = { false, true, false, false };
static const bool ConnectedReturnList_12f[12] = { false, false, false, false, false, false, false, false, false, false, false, false };
static const bool ConnectedReturnList_11ft[12] = { false, false, false, false, false, false, false, false, false, false, false, true };
static const bool ConnectedReturnList_f12t[13] = { false, true, true, true, true, true, true, true, true, true, true, true, true };
static const bool ConnectedReturnList_f11tf[13] = { false, true, true, true, true, true, true, true, true, true, true, true, false };

static const char* SendBuffer = "Message to send";
size_t SendBufferSize = 16;
static const size_t SendReturnList_once[1] = { (const size_t)16 };
static const size_t SendReturnList_once_zero[1] = { (const size_t)0 };
static const size_t SendReturnList_twice[2] = { (const size_t)10, (const size_t)6 };
static const size_t SendReturnList_twice_zero[2] = { (const size_t)10, (const size_t)0 };

static const char* ReceivedBuffer = (const char*)"This is the received buffer";
static const int ReceivedReturnList[1] = { (const int)28 };

static void* CallbackContext;
static SSLCLIENT_HANDLE SSLClientHandel;

DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    char temp_str[256];
    (void)snprintf(temp_str, sizeof(temp_str), "umock_c reported error :%s", ENUM_TO_STRING(UMOCK_C_ERROR_CODE, error_code));
    ASSERT_FAIL(temp_str);
}

#define SSLCLIENT_HANDEL_NULL (SSLCLIENT_HANDLE)NULL
static SSLCLIENT_HANDLE my_sslClient_new_return;
SSLCLIENT_HANDLE my_sslClient_new(void)
{
	return my_sslClient_new_return;
}

void my_sslClient_delete(SSLCLIENT_HANDLE handle)
{
	if (handle == NULL)
	{
		ASSERT_FAIL("NULL handle in the sslClient_delete");
	}
}

void my_sslClient_setTimeout(SSLCLIENT_HANDLE handle, unsigned long timeout)
{
	if(handle == NULL)
	{
		ASSERT_FAIL("NULL handle in the sslClient_setTimeout");
	}
	else
	{
		(void)(timeout);
	}
}

static const bool* my_sslClient_connected_return_list;
static int my_sslClient_connected_count;
uint8_t my_sslClient_connected(SSLCLIENT_HANDLE handle)
{
	if (handle == NULL)
	{
		ASSERT_FAIL("NULL handle in the sslClient_connected");
	}
	return my_sslClient_connected_return_list[my_sslClient_connected_count++];
}

static const int* my_sslClient_connect_return_list;
static int my_sslClient_connect_count;
int my_sslClient_connect(SSLCLIENT_HANDLE handle, uint32_t ipAddress, uint16_t port)
{
	if (handle == NULL)
	{
		ASSERT_FAIL("NULL handle in the sslClient_connect");
	}
	else
	{
		(void)(ipAddress);
		(void)(port);
	}
	return my_sslClient_connect_return_list[my_sslClient_connect_count++];
}

void my_sslClient_stop(SSLCLIENT_HANDLE handle)
{
	if (handle == NULL)
	{
		ASSERT_FAIL("NULL handle in the sslClient_stop");
	}
}

static const size_t* my_sslClient_send_return_list;
static int my_sslClient_send_count;
size_t my_sslClient_write(SSLCLIENT_HANDLE handle, const uint8_t *buf, size_t size)
{
	if (handle == NULL)
	{
		ASSERT_FAIL("NULL handle in the sslClient_write");
	}
	else
	{
		(void)(buf);
		(void)(size);
	}
	return my_sslClient_send_return_list[my_sslClient_send_count++];
}

static const int* my_sslClient_read_return_list;
static const uint8_t* my_sslClient_read_buffer;
static int my_sslClient_read_count;
int my_sslClient_read(SSLCLIENT_HANDLE handle, uint8_t *buf, size_t size)
{
	if (handle == NULL)
	{
		ASSERT_FAIL("NULL handle in the sslClient_read");
	}
	else
	{
        const char* readPtr = (const char*)my_sslClient_read_buffer;
        for (int i = 0; i < my_sslClient_read_count; i++)
        {
            readPtr += my_sslClient_read_return_list[i];
        }
        strncpy((char*)buf, readPtr, size);
	}
	return my_sslClient_read_return_list[my_sslClient_read_count++];
}

#define SSLCLIENT_IP_ADDRESS (uint32_t)0x11223344
#define SSLCLIENT_IP_ADDRESS_SUCCEED (uint8_t)true
#define SSLCLIENT_IP_ADDRESS_FAILED (uint8_t)false
static uint8_t my_sslClient_hostByName_return;
uint8_t my_sslClient_hostByName(const char* hostName, uint32_t* ipAddress)
{
    if (hostName == NULL)
    {
        ASSERT_FAIL("NULL hostName in the sslClient_hostByName");
    }
    else
    {
        (*ipAddress) = SSLCLIENT_IP_ADDRESS;
    }
    return my_sslClient_hostByName_return;
}

static void* on_bytes_received_context;
static char on_bytes_received_buffer[100];
static size_t on_bytes_received_buffer_size;
static int on_bytes_received_count;
static void on_bytes_received(void* context, const unsigned char* buffer, size_t size)
{
    on_bytes_received_count++;
    on_bytes_received_context = context;
    strncpy(on_bytes_received_buffer, (const char*)buffer, size);
    on_bytes_received_buffer[size] = '\0';
    on_bytes_received_buffer_size = size;
}

static void* on_send_complete_context;
static IO_SEND_RESULT on_send_complete_result;
static int on_send_complete_count;
static void on_send_complete(void* context, IO_SEND_RESULT send_result)
{
    on_send_complete_count++;
    on_send_complete_context = context;
    on_send_complete_result = send_result;
}

static void* on_io_open_complete_context;
static IO_OPEN_RESULT on_io_open_complete_result;
static int on_io_open_complete_count;
static void on_io_open_complete(void* context, IO_OPEN_RESULT open_result)
{
    on_io_open_complete_count++;
    on_io_open_complete_context = context;
    on_io_open_complete_result = open_result;
}

static void* on_io_close_complete_context;
static int on_io_close_complete_count;
static void on_io_close_complete(void* context)
{
    on_io_close_complete_count++;
    on_io_close_complete_context = context;
}

static void* on_io_error_context;
static int on_io_error_count;
static void on_io_error(void* context)
{
    on_io_error_count++;
    on_io_error_context = context;
}

#define ASSERT_CALLBACK_COUNTERS(error, open, close, send, received) \
        ASSERT_ARE_EQUAL(int, error, on_io_error_count);\
        ASSERT_ARE_EQUAL(int, open, on_io_open_complete_count);\
        ASSERT_ARE_EQUAL(int, close, on_io_close_complete_count);\
        ASSERT_ARE_EQUAL(int, send, on_send_complete_count);\
        ASSERT_ARE_EQUAL(int, received, on_bytes_received_count);

TEST_DEFINE_ENUM_TYPE(TLSIO_ARDUINO_STATE, TLSIO_ARDUINO_STATE_VALUES);
IMPLEMENT_UMOCK_C_ENUM_TYPE(TLSIO_ARDUINO_STATE, TLSIO_ARDUINO_STATE_VALUES);

TEST_DEFINE_ENUM_TYPE(IO_OPEN_RESULT, IO_OPEN_RESULT_VALUES);
IMPLEMENT_UMOCK_C_ENUM_TYPE(IO_OPEN_RESULT, IO_OPEN_RESULT_VALUES);



static TEST_MUTEX_HANDLE g_testByTest;
static TEST_MUTEX_HANDLE g_dllByDll;

BEGIN_TEST_SUITE(tlsioarduino_ut)

    TEST_SUITE_INITIALIZE(a)
    {
        int result;
        TEST_INITIALIZE_MEMORY_DEBUG(g_dllByDll);
        g_testByTest = TEST_MUTEX_CREATE();
        ASSERT_IS_NOT_NULL(g_testByTest);

        (void)umock_c_init(on_umock_c_error);

        result = umocktypes_charptr_register_types();
		ASSERT_ARE_EQUAL(int, 0, result);

		REGISTER_UMOCK_ALIAS_TYPE(SSLCLIENT_HANDLE, void*);
        REGISTER_UMOCK_ALIAS_TYPE(CONCRETE_IO_HANDLE, void*);
        REGISTER_UMOCK_ALIAS_TYPE(OPTIONHANDLER_HANDLE, void*);
        REGISTER_UMOCK_ALIAS_TYPE(uint32_t, unsigned int);
        REGISTER_UMOCK_ALIAS_TYPE(uint16_t, unsigned short);
        REGISTER_UMOCK_ALIAS_TYPE(uint8_t, unsigned char);
        REGISTER_UMOCK_ALIAS_TYPE(TLSIO_ARDUINO_STATE, int);
        REGISTER_UMOCK_ALIAS_TYPE(IO_OPEN_RESULT, int);
        
        

		REGISTER_GLOBAL_MOCK_HOOK(gballoc_malloc, my_gballoc_malloc);
		REGISTER_GLOBAL_MOCK_HOOK(gballoc_realloc, my_gballoc_realloc);
		REGISTER_GLOBAL_MOCK_HOOK(gballoc_free, my_gballoc_free);

		REGISTER_GLOBAL_MOCK_HOOK(sslClient_new, my_sslClient_new);
		REGISTER_GLOBAL_MOCK_HOOK(sslClient_delete, my_sslClient_delete);
		REGISTER_GLOBAL_MOCK_HOOK(sslClient_setTimeout, my_sslClient_setTimeout);
		REGISTER_GLOBAL_MOCK_HOOK(sslClient_connected, my_sslClient_connected);
        REGISTER_GLOBAL_MOCK_HOOK(sslClient_connect, my_sslClient_connect);
        REGISTER_GLOBAL_MOCK_HOOK(sslClient_stop, my_sslClient_stop);
		REGISTER_GLOBAL_MOCK_HOOK(sslClient_write, my_sslClient_write);
		REGISTER_GLOBAL_MOCK_HOOK(sslClient_read, my_sslClient_read);
        REGISTER_GLOBAL_MOCK_HOOK(sslClient_hostByName, my_sslClient_hostByName);

        CallbackContext = malloc(1);
        ASSERT_IS_NOT_NULL(CallbackContext);

        SSLClientHandel = (SSLCLIENT_HANDLE)malloc(1);
        ASSERT_IS_NOT_NULL(SSLClientHandel);
}

    TEST_SUITE_CLEANUP(TestClassCleanup)
    {
        umock_c_deinit();

        TEST_MUTEX_DESTROY(g_testByTest);
        TEST_DEINITIALIZE_MEMORY_DEBUG(g_dllByDll);
    }

    TEST_FUNCTION_INITIALIZE(initialize)
    {
        if (TEST_MUTEX_ACQUIRE(g_testByTest))
        {
            ASSERT_FAIL("Could not acquire test serialization mutex.");
        }

        umock_c_reset_all_calls();
        currentmalloc_call = 0;
        whenShallmalloc_fail = 0;

        on_bytes_received_count = 0;
        on_send_complete_count = 0;
        on_io_open_complete_count = 0;
        on_io_close_complete_count = 0;
        on_io_error_count = 0;

        my_sslClient_connected_count = 0;
        my_sslClient_connect_count = 0;
        my_sslClient_send_count = 0;
        my_sslClient_read_count = 0;
    }

    TEST_FUNCTION_CLEANUP(cleans)
    {
        TEST_MUTEX_RELEASE(g_testByTest);
    }


    /* Tests_SRS_TLSIO_ARDUINO_21_078: [ The tlsio_arduino_retrieveoptions shall not do anything, and return NULL. ]*/
    TEST_FUNCTION(tlsio_arduino_retrieveoptions__succeed)
    {
        ///arrange
        OPTIONHANDLER_HANDLE result;
        const IO_INTERFACE_DESCRIPTION* tlsioInterfaces = tlsio_arduino_get_interface_description();
        ASSERT_IS_NOT_NULL(tlsioInterfaces);

        ///act
        result = tlsioInterfaces->concrete_io_retrieveoptions(NULL);

        ///assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_IS_NULL(result);

        ///cleanup
    }

    /* Tests_SRS_TLSIO_ARDUINO_21_077: [ The tlsio_arduino_setoption shall not do anything, and return 0. ]*/
    TEST_FUNCTION(tlsio_arduino_setoption__succeed)
    {
        ///arrange
        int result;
        const IO_INTERFACE_DESCRIPTION* tlsioInterfaces = tlsio_arduino_get_interface_description();
        ASSERT_IS_NOT_NULL(tlsioInterfaces);

        ///act
        result = tlsioInterfaces->concrete_io_setoption(NULL, NULL, NULL);

        ///assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_ARE_EQUAL(int, 0, result);

        ///cleanup
    }

    /* Tests_SRS_TLSIO_ARDUINO_21_074: [ If the tlsio_handle is NULL, the tlsio_arduino_dowork shall not do anything. ]*/
    TEST_FUNCTION(tlsio_arduino_dowork__NULL_tlsio_handle__failed)
    {
        ///arrange
        const IO_INTERFACE_DESCRIPTION* tlsioInterfaces = tlsio_arduino_get_interface_description();
        ASSERT_IS_NOT_NULL(tlsioInterfaces);

        ///act
        tlsioInterfaces->concrete_io_dowork(NULL);

        ///assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
    }

    /* Tests_SRS_TLSIO_ARDUINO_21_073: [ If the tlsio state is TLSIO_ARDUINO_STATE_ERROR, the tlsio_arduino_dowork shall not do anything. ]*/
    TEST_FUNCTION(tlsio_arduino_dowork__on_TLSIO_ARDUINO_STATE_ERROR__failed)
    {
        ///arrange
        CONCRETE_IO_HANDLE tlsioHandle;
        int result;
        const IO_INTERFACE_DESCRIPTION* tlsioInterfaces = tlsio_arduino_get_interface_description();
        ASSERT_IS_NOT_NULL(tlsioInterfaces);

        my_sslClient_new_return = SSLClientHandel;
        my_sslClient_hostByName_return = SSLCLIENT_IP_ADDRESS_SUCCEED;
        my_sslClient_connected_return_list = (const bool*)ConnectedReturnList_ft;
        my_sslClient_connect_return_list = (const int*)ConnectReturnList_true;

        STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG)).IgnoreArgument(1);
        STRICT_EXPECTED_CALL(sslClient_new());
        STRICT_EXPECTED_CALL(sslClient_setTimeout(SSLClientHandel, 10000));
        STRICT_EXPECTED_CALL(sslClient_hostByName(TEST_CREATE_CONNECTION_HOST_NAME, IGNORED_PTR_ARG)).IgnoreArgument(2);
        STRICT_EXPECTED_CALL(sslClient_connected(SSLClientHandel));
        STRICT_EXPECTED_CALL(sslClient_connect(SSLClientHandel, SSLCLIENT_IP_ADDRESS, TEST_CREATE_CONNECTION_PORT));
        STRICT_EXPECTED_CALL(sslClient_connected(SSLClientHandel));

        tlsioHandle = tlsioInterfaces->concrete_io_create((void*)&tlsioConfig);
        ASSERT_IS_NOT_NULL(tlsioHandle);
        result = tlsioInterfaces->concrete_io_open(
            tlsioHandle,
            on_io_open_complete, CallbackContext,
            on_bytes_received, CallbackContext,
            on_io_error, CallbackContext);
        result = tlsioInterfaces->concrete_io_open(
            tlsioHandle,
            on_io_open_complete, CallbackContext,
            on_bytes_received, CallbackContext,
            on_io_error, CallbackContext);
        ASSERT_ARE_NOT_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(int, (int)TLSIO_ARDUINO_STATE_ERROR, (int)tlsio_arduino_get_state(tlsioHandle));

        ///act
        tlsioInterfaces->concrete_io_dowork(tlsioHandle);

        ///assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_ARE_EQUAL(int, 1, currentmalloc_call);
        ASSERT_CALLBACK_COUNTERS(1, 2, 0, 0, 0);
        ASSERT_ARE_EQUAL(int, (int)TLSIO_ARDUINO_STATE_ERROR, (int)tlsio_arduino_get_state(tlsioHandle));

        ///cleanup
        tlsioInterfaces->concrete_io_destroy(tlsioHandle);
    }

    /* Tests_SRS_TLSIO_ARDUINO_21_072: [ If the tlsio state is TLSIO_ARDUINO_STATE_CLOSED, and ssl client is connected, the tlsio_arduino_dowork shall change the state to TLSIO_ARDUINO_STATE_ERROR, call on_io_error. ]*/
    TEST_FUNCTION(tlsio_arduino_dowork__on_TLSIO_ARDUINO_STATE_CLOSED__never_opened__failed)
    {
        ///arrange
        CONCRETE_IO_HANDLE tlsioHandle;
        const IO_INTERFACE_DESCRIPTION* tlsioInterfaces = tlsio_arduino_get_interface_description();
        ASSERT_IS_NOT_NULL(tlsioInterfaces);

        my_sslClient_connected_return_list = (const bool*)ConnectedReturnList_f;
        my_sslClient_new_return = SSLClientHandel;
        my_sslClient_hostByName_return = SSLCLIENT_IP_ADDRESS_SUCCEED;

        STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG)).IgnoreArgument(1);
        STRICT_EXPECTED_CALL(sslClient_new());
        STRICT_EXPECTED_CALL(sslClient_setTimeout(SSLClientHandel, 10000));
        STRICT_EXPECTED_CALL(sslClient_hostByName(TEST_CREATE_CONNECTION_HOST_NAME, IGNORED_PTR_ARG)).IgnoreArgument(2);
        STRICT_EXPECTED_CALL(sslClient_connected(SSLClientHandel));

        tlsioHandle = tlsioInterfaces->concrete_io_create((void*)&tlsioConfig);
        ASSERT_IS_NOT_NULL(tlsioHandle);
        ASSERT_ARE_EQUAL(int, (int)TLSIO_ARDUINO_STATE_CLOSED, (int)tlsio_arduino_get_state(tlsioHandle));

        ///act
        tlsioInterfaces->concrete_io_dowork(tlsioHandle);

        ///assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_ARE_EQUAL(int, 1, currentmalloc_call);
        ASSERT_CALLBACK_COUNTERS(0, 0, 0, 0, 0);
        ASSERT_ARE_EQUAL(int, (int)TLSIO_ARDUINO_STATE_CLOSED, (int)tlsio_arduino_get_state(tlsioHandle));

        ///cleanup
        tlsioInterfaces->concrete_io_destroy(tlsioHandle);
    }

    /* Tests_SRS_TLSIO_ARDUINO_21_072: [ If the tlsio state is TLSIO_ARDUINO_STATE_CLOSED, and ssl client is connected, the tlsio_arduino_dowork shall change the state to TLSIO_ARDUINO_STATE_ERROR, call on_io_error. ]*/
    TEST_FUNCTION(tlsio_arduino_dowork__on_TLSIO_ARDUINO_STATE_CLOSED_ssl_connected__failed)
    {
        ///arrange
        CONCRETE_IO_HANDLE tlsioHandle;
        int result;
        const IO_INTERFACE_DESCRIPTION* tlsioInterfaces = tlsio_arduino_get_interface_description();
        ASSERT_IS_NOT_NULL(tlsioInterfaces);

        my_sslClient_new_return = SSLClientHandel;
        my_sslClient_hostByName_return = SSLCLIENT_IP_ADDRESS_SUCCEED;
        my_sslClient_connected_return_list = (const bool*)ConnectedReturnList_ftft;
        my_sslClient_connect_return_list = (const int*)ConnectReturnList_true;

        STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG)).IgnoreArgument(1);
        STRICT_EXPECTED_CALL(sslClient_new());
        STRICT_EXPECTED_CALL(sslClient_setTimeout(SSLClientHandel, 10000));
        STRICT_EXPECTED_CALL(sslClient_hostByName(TEST_CREATE_CONNECTION_HOST_NAME, IGNORED_PTR_ARG)).IgnoreArgument(2);
        STRICT_EXPECTED_CALL(sslClient_connected(SSLClientHandel));
        STRICT_EXPECTED_CALL(sslClient_connect(SSLClientHandel, SSLCLIENT_IP_ADDRESS, TEST_CREATE_CONNECTION_PORT));
        STRICT_EXPECTED_CALL(sslClient_connected(SSLClientHandel));
        STRICT_EXPECTED_CALL(sslClient_stop(SSLClientHandel));
        STRICT_EXPECTED_CALL(sslClient_connected(SSLClientHandel));
        STRICT_EXPECTED_CALL(sslClient_connected(SSLClientHandel));

        tlsioHandle = tlsioInterfaces->concrete_io_create((void*)&tlsioConfig);
        ASSERT_IS_NOT_NULL(tlsioHandle);
        result = tlsioInterfaces->concrete_io_open(
            tlsioHandle,
            on_io_open_complete, CallbackContext,
            on_bytes_received, CallbackContext,
            on_io_error, CallbackContext);
        ASSERT_ARE_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(int, (int)TLSIO_ARDUINO_STATE_OPEN, (int)tlsio_arduino_get_state(tlsioHandle));

        result = tlsioInterfaces->concrete_io_close(tlsioHandle, on_io_close_complete, CallbackContext);
        ASSERT_ARE_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(int, (int)TLSIO_ARDUINO_STATE_CLOSED, (int)tlsio_arduino_get_state(tlsioHandle));

        ///act
        tlsioInterfaces->concrete_io_dowork(tlsioHandle);

        ///assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_ARE_EQUAL(int, 1, currentmalloc_call);
        ASSERT_CALLBACK_COUNTERS(1, 1, 1, 0, 0);
        ASSERT_ARE_EQUAL(int, (int)TLSIO_ARDUINO_STATE_ERROR, (int)tlsio_arduino_get_state(tlsioHandle));

        ///cleanup
        tlsioInterfaces->concrete_io_destroy(tlsioHandle);
    }

    /* Tests_SRS_TLSIO_ARDUINO_21_072: [ If the tlsio state is TLSIO_ARDUINO_STATE_CLOSED, and ssl client is connected, the tlsio_arduino_dowork shall change the state to TLSIO_ARDUINO_STATE_ERROR, call on_io_error. ]*/
    TEST_FUNCTION(tlsio_arduino_dowork__on_TLSIO_ARDUINO_STATE_CLOSED__succeed)
    {
        ///arrange
        CONCRETE_IO_HANDLE tlsioHandle;
        int result;
        const IO_INTERFACE_DESCRIPTION* tlsioInterfaces = tlsio_arduino_get_interface_description();
        ASSERT_IS_NOT_NULL(tlsioInterfaces);

        my_sslClient_new_return = SSLClientHandel;
        my_sslClient_hostByName_return = SSLCLIENT_IP_ADDRESS_SUCCEED;
        my_sslClient_connected_return_list = (const bool*)ConnectedReturnList_ftff;
        my_sslClient_connect_return_list = (const int*)ConnectReturnList_true;

        STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG)).IgnoreArgument(1);
        STRICT_EXPECTED_CALL(sslClient_new());
        STRICT_EXPECTED_CALL(sslClient_setTimeout(SSLClientHandel, 10000));
        STRICT_EXPECTED_CALL(sslClient_hostByName(TEST_CREATE_CONNECTION_HOST_NAME, IGNORED_PTR_ARG)).IgnoreArgument(2);
        STRICT_EXPECTED_CALL(sslClient_connected(SSLClientHandel));
        STRICT_EXPECTED_CALL(sslClient_connect(SSLClientHandel, SSLCLIENT_IP_ADDRESS, TEST_CREATE_CONNECTION_PORT));
        STRICT_EXPECTED_CALL(sslClient_connected(SSLClientHandel));
        STRICT_EXPECTED_CALL(sslClient_stop(SSLClientHandel));
        STRICT_EXPECTED_CALL(sslClient_connected(SSLClientHandel));
        STRICT_EXPECTED_CALL(sslClient_connected(SSLClientHandel));

        tlsioHandle = tlsioInterfaces->concrete_io_create((void*)&tlsioConfig);
        ASSERT_IS_NOT_NULL(tlsioHandle);
        result = tlsioInterfaces->concrete_io_open(
            tlsioHandle,
            on_io_open_complete, CallbackContext,
            on_bytes_received, CallbackContext,
            on_io_error, CallbackContext);
        ASSERT_ARE_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(int, (int)TLSIO_ARDUINO_STATE_OPEN, (int)tlsio_arduino_get_state(tlsioHandle));

        result = tlsioInterfaces->concrete_io_close(tlsioHandle, on_io_close_complete, CallbackContext);
        ASSERT_ARE_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(int, (int)TLSIO_ARDUINO_STATE_CLOSED, (int)tlsio_arduino_get_state(tlsioHandle));

        ///act
        tlsioInterfaces->concrete_io_dowork(tlsioHandle);

        ///assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_ARE_EQUAL(int, 1, currentmalloc_call);
        ASSERT_CALLBACK_COUNTERS(0, 1, 1, 0, 0);
        ASSERT_ARE_EQUAL(int, (int)TLSIO_ARDUINO_STATE_CLOSED, (int)tlsio_arduino_get_state(tlsioHandle));

        ///cleanup
        tlsioInterfaces->concrete_io_destroy(tlsioHandle);
    }
    
    /* Tests_SRS_TLSIO_ARDUINO_21_071: [ If the tlsio state is TLSIO_ARDUINO_STATE_OPEN, and ssl client is not connected, the tlsio_arduino_dowork shall change the state to TLSIO_ARDUINO_STATE_ERROR, call on_io_error. ]*/
    TEST_FUNCTION(tlsio_arduino_dowork__lost_connection__failed)
    {
        ///arrange
        CONCRETE_IO_HANDLE tlsioHandle;
        int result;
        const IO_INTERFACE_DESCRIPTION* tlsioInterfaces = tlsio_arduino_get_interface_description();
        ASSERT_IS_NOT_NULL(tlsioInterfaces);

        my_sslClient_new_return = SSLClientHandel;
        my_sslClient_hostByName_return = SSLCLIENT_IP_ADDRESS_SUCCEED;
        my_sslClient_connected_return_list = (const bool*)ConnectedReturnList_ftf;
        my_sslClient_connect_return_list = (const int*)ConnectReturnList_true;
        my_sslClient_send_return_list = (const size_t*)SendReturnList_twice;
        my_sslClient_read_buffer = (const uint8_t*)ReceivedBuffer;
        my_sslClient_read_return_list = (const int*)ReceivedReturnList;

        STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG)).IgnoreArgument(1);
        STRICT_EXPECTED_CALL(sslClient_new());
        STRICT_EXPECTED_CALL(sslClient_setTimeout(SSLClientHandel, 10000));
        STRICT_EXPECTED_CALL(sslClient_hostByName(TEST_CREATE_CONNECTION_HOST_NAME, IGNORED_PTR_ARG)).IgnoreArgument(2);
        STRICT_EXPECTED_CALL(sslClient_connected(SSLClientHandel));
        STRICT_EXPECTED_CALL(sslClient_connect(SSLClientHandel, SSLCLIENT_IP_ADDRESS, TEST_CREATE_CONNECTION_PORT));
        STRICT_EXPECTED_CALL(sslClient_connected(SSLClientHandel));
        STRICT_EXPECTED_CALL(sslClient_write(SSLClientHandel, (uint8_t*)SendBuffer, SendBufferSize));
        STRICT_EXPECTED_CALL(sslClient_write(SSLClientHandel, (uint8_t*)SendBuffer + SendReturnList_twice[0], SendBufferSize - SendReturnList_twice[0]));
        STRICT_EXPECTED_CALL(sslClient_connected(SSLClientHandel));

        tlsioHandle = tlsioInterfaces->concrete_io_create((void*)&tlsioConfig);
        ASSERT_IS_NOT_NULL(tlsioHandle);

        result = tlsioInterfaces->concrete_io_open(
            tlsioHandle,
            on_io_open_complete, CallbackContext,
            on_bytes_received, CallbackContext,
            on_io_error, CallbackContext);
        ASSERT_ARE_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(int, (int)TLSIO_ARDUINO_STATE_OPEN, (int)tlsio_arduino_get_state(tlsioHandle));

        result = tlsioInterfaces->concrete_io_send(tlsioHandle, (const void*)SendBuffer, SendBufferSize, on_send_complete, CallbackContext);
        ASSERT_ARE_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(int, (int)TLSIO_ARDUINO_STATE_OPEN, (int)tlsio_arduino_get_state(tlsioHandle));

        ///act
        tlsioInterfaces->concrete_io_dowork(tlsioHandle);

        ///assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_ARE_EQUAL(int, 1, currentmalloc_call);
        ASSERT_CALLBACK_COUNTERS(1, 1, 0, 1, 0);
        ASSERT_ARE_EQUAL(int, (int)IO_SEND_OK, (int)on_send_complete_result);
        ASSERT_ARE_EQUAL(int, (int)TLSIO_ARDUINO_STATE_ERROR, (int)tlsio_arduino_get_state(tlsioHandle));

        ///cleanup
        result = tlsioInterfaces->concrete_io_close(tlsioHandle, on_io_close_complete, CallbackContext);
        tlsioInterfaces->concrete_io_destroy(tlsioHandle);
    }

    /* Tests_SRS_TLSIO_ARDUINO_21_030: [ The tlsio_arduino_open shall store the provided on_bytes_received callback function address. ]*/
    /* Tests_SRS_TLSIO_ARDUINO_21_031: [ The tlsio_arduino_open shall store the provided on_bytes_received_context handle. ]*/
    /* Tests_SRS_TLSIO_ARDUINO_21_069: [ If the tlsio state is TLSIO_ARDUINO_STATE_OPEN, the tlsio_arduino_dowork shall read data from the ssl client. ]*/
    /* Tests_SRS_TLSIO_ARDUINO_21_070: [ If the tlsio state is TLSIO_ARDUINO_STATE_OPEN, and there are received data in the ssl client, the tlsio_arduino_dowork shall read this data and call the on_bytes_received with the pointer to the buffer with the data. ]*/
    TEST_FUNCTION(tlsio_arduino_dowork__receive_data_without_read_callback__succeed)
    {
        ///arrange
        CONCRETE_IO_HANDLE tlsioHandle;
        int result;
        const IO_INTERFACE_DESCRIPTION* tlsioInterfaces = tlsio_arduino_get_interface_description();
        ASSERT_IS_NOT_NULL(tlsioInterfaces);

        my_sslClient_new_return = SSLClientHandel;
        my_sslClient_hostByName_return = SSLCLIENT_IP_ADDRESS_SUCCEED;
        my_sslClient_connected_return_list = (const bool*)ConnectedReturnList_ftt;
        my_sslClient_connect_return_list = (const int*)ConnectReturnList_true;
        my_sslClient_send_return_list = (const size_t*)SendReturnList_twice;
        my_sslClient_read_buffer = (const uint8_t*)ReceivedBuffer;
        my_sslClient_read_return_list = (const int*)ReceivedReturnList;

        STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG)).IgnoreArgument(1);
        STRICT_EXPECTED_CALL(sslClient_new());
        STRICT_EXPECTED_CALL(sslClient_setTimeout(SSLClientHandel, 10000));
        STRICT_EXPECTED_CALL(sslClient_hostByName(TEST_CREATE_CONNECTION_HOST_NAME, IGNORED_PTR_ARG)).IgnoreArgument(2);
        STRICT_EXPECTED_CALL(sslClient_connected(SSLClientHandel));
        STRICT_EXPECTED_CALL(sslClient_connect(SSLClientHandel, SSLCLIENT_IP_ADDRESS, TEST_CREATE_CONNECTION_PORT));
        STRICT_EXPECTED_CALL(sslClient_connected(SSLClientHandel));
        STRICT_EXPECTED_CALL(sslClient_write(SSLClientHandel, (uint8_t*)SendBuffer, SendBufferSize));
        STRICT_EXPECTED_CALL(sslClient_write(SSLClientHandel, (uint8_t*)SendBuffer + SendReturnList_twice[0], SendBufferSize - SendReturnList_twice[0]));
        STRICT_EXPECTED_CALL(sslClient_connected(SSLClientHandel));
        STRICT_EXPECTED_CALL(sslClient_read(SSLClientHandel, IGNORED_PTR_ARG, IGNORED_NUM_ARG)).IgnoreArgument(2).IgnoreArgument(3);

        tlsioHandle = tlsioInterfaces->concrete_io_create((void*)&tlsioConfig);
        ASSERT_IS_NOT_NULL(tlsioHandle);

        result = tlsioInterfaces->concrete_io_open(
            tlsioHandle,
            NULL, NULL,
            NULL, NULL,
            NULL, NULL);
        ASSERT_ARE_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(int, (int)TLSIO_ARDUINO_STATE_OPEN, (int)tlsio_arduino_get_state(tlsioHandle));

        result = tlsioInterfaces->concrete_io_send(tlsioHandle, (const void*)SendBuffer, SendBufferSize, on_send_complete, CallbackContext);
        ASSERT_ARE_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(int, (int)TLSIO_ARDUINO_STATE_OPEN, (int)tlsio_arduino_get_state(tlsioHandle));

        ///act
        tlsioInterfaces->concrete_io_dowork(tlsioHandle);

        ///assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_ARE_EQUAL(int, 1, currentmalloc_call);
        ASSERT_CALLBACK_COUNTERS(0, 0, 0, 1, 0);
        ASSERT_ARE_EQUAL(int, (int)IO_SEND_OK, (int)on_send_complete_result);
        ASSERT_ARE_EQUAL(int, (int)TLSIO_ARDUINO_STATE_OPEN, (int)tlsio_arduino_get_state(tlsioHandle));

        ///cleanup
        result = tlsioInterfaces->concrete_io_close(tlsioHandle, on_io_close_complete, CallbackContext);
        tlsioInterfaces->concrete_io_destroy(tlsioHandle);
    }

    /* Tests_SRS_TLSIO_ARDUINO_21_030: [ The tlsio_arduino_open shall store the provided on_bytes_received callback function address. ]*/
    /* Tests_SRS_TLSIO_ARDUINO_21_031: [ The tlsio_arduino_open shall store the provided on_bytes_received_context handle. ]*/
    /* Tests_SRS_TLSIO_ARDUINO_21_062: [ The tlsio_arduino_dowork shall execute the async jobs for the tlsio. ]*/
    /* Tests_SRS_TLSIO_ARDUINO_21_069: [ If the tlsio state is TLSIO_ARDUINO_STATE_OPEN, the tlsio_arduino_dowork shall read data from the ssl client. ]*/
    /* Tests_SRS_TLSIO_ARDUINO_21_070: [ If the tlsio state is TLSIO_ARDUINO_STATE_OPEN, and there are received data in the ssl client, the tlsio_arduino_dowork shall read this data and call the on_bytes_received with the pointer to the buffer with the data. ]*/
    /* Tests_SRS_TLSIO_ARDUINO_21_075: [ The tlsio_arduino_dowork shall create a buffer to store the data received from the ssl client. ]*/
    /* Tests_SRS_TLSIO_ARDUINO_21_076: [ The tlsio_arduino_dowork shall delete the buffer to store the data received from the ssl client. ]*/
    TEST_FUNCTION(tlsio_arduino_dowork__receive_data__succeed)
    {
        ///arrange
        CONCRETE_IO_HANDLE tlsioHandle;
        int result;
        const IO_INTERFACE_DESCRIPTION* tlsioInterfaces = tlsio_arduino_get_interface_description();
        ASSERT_IS_NOT_NULL(tlsioInterfaces);

        my_sslClient_new_return = SSLClientHandel;
        my_sslClient_hostByName_return = SSLCLIENT_IP_ADDRESS_SUCCEED;
        my_sslClient_connected_return_list = (const bool*)ConnectedReturnList_ftt;
        my_sslClient_connect_return_list = (const int*)ConnectReturnList_true;
        my_sslClient_send_return_list = (const size_t*)SendReturnList_twice;
        my_sslClient_read_buffer = (const uint8_t*)ReceivedBuffer;
        my_sslClient_read_return_list = (const int*)ReceivedReturnList;

        STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG)).IgnoreArgument(1);
        STRICT_EXPECTED_CALL(sslClient_new());
        STRICT_EXPECTED_CALL(sslClient_setTimeout(SSLClientHandel, 10000));
        STRICT_EXPECTED_CALL(sslClient_hostByName(TEST_CREATE_CONNECTION_HOST_NAME, IGNORED_PTR_ARG)).IgnoreArgument(2);
        STRICT_EXPECTED_CALL(sslClient_connected(SSLClientHandel));
        STRICT_EXPECTED_CALL(sslClient_connect(SSLClientHandel, SSLCLIENT_IP_ADDRESS, TEST_CREATE_CONNECTION_PORT));
        STRICT_EXPECTED_CALL(sslClient_connected(SSLClientHandel));
        STRICT_EXPECTED_CALL(sslClient_write(SSLClientHandel, (uint8_t*)SendBuffer, SendBufferSize));
        STRICT_EXPECTED_CALL(sslClient_write(SSLClientHandel, (uint8_t*)SendBuffer + SendReturnList_twice[0], SendBufferSize - SendReturnList_twice[0]));
        STRICT_EXPECTED_CALL(sslClient_connected(SSLClientHandel));
        STRICT_EXPECTED_CALL(sslClient_read(SSLClientHandel, IGNORED_PTR_ARG, IGNORED_NUM_ARG)).IgnoreArgument(2).IgnoreArgument(3);

        tlsioHandle = tlsioInterfaces->concrete_io_create((void*)&tlsioConfig);
        ASSERT_IS_NOT_NULL(tlsioHandle);

        result = tlsioInterfaces->concrete_io_open(
            tlsioHandle,
            on_io_open_complete, CallbackContext,
            on_bytes_received, CallbackContext,
            on_io_error, CallbackContext);
        ASSERT_ARE_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(int, (int)TLSIO_ARDUINO_STATE_OPEN, (int)tlsio_arduino_get_state(tlsioHandle));

        result = tlsioInterfaces->concrete_io_send(tlsioHandle, (const void*)SendBuffer, SendBufferSize, on_send_complete, CallbackContext);
        ASSERT_ARE_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(int, (int)TLSIO_ARDUINO_STATE_OPEN, (int)tlsio_arduino_get_state(tlsioHandle));

        ///act
        tlsioInterfaces->concrete_io_dowork(tlsioHandle);

        ///assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_ARE_EQUAL(char_ptr, (const char*)my_sslClient_read_buffer, (const char*)on_bytes_received_buffer);
        ASSERT_ARE_EQUAL(size_t, 28, on_bytes_received_buffer_size);
        ASSERT_ARE_EQUAL(int, 1, currentmalloc_call);
        ASSERT_CALLBACK_COUNTERS(0, 1, 0, 1, 1);
        ASSERT_ARE_EQUAL(int, (int)IO_SEND_OK, (int)on_send_complete_result);
        ASSERT_ARE_EQUAL(int, (int)TLSIO_ARDUINO_STATE_OPEN, (int)tlsio_arduino_get_state(tlsioHandle));

        ///cleanup
        result = tlsioInterfaces->concrete_io_close(tlsioHandle, on_io_close_complete, CallbackContext);
        tlsioInterfaces->concrete_io_destroy(tlsioHandle);
    }

    /* Tests_SRS_TLSIO_ARDUINO_21_061: [ If the size is 0, the tlsio_arduino_send shall not do anything, and return _LINE_. ]*/
    TEST_FUNCTION(tlsio_arduino_send__0_size_txBuffer__failed)
    {
        ///arrange
        CONCRETE_IO_HANDLE tlsioHandle;
        int result;
        const IO_INTERFACE_DESCRIPTION* tlsioInterfaces = tlsio_arduino_get_interface_description();
        ASSERT_IS_NOT_NULL(tlsioInterfaces);

        my_sslClient_new_return = SSLClientHandel;
        my_sslClient_hostByName_return = SSLCLIENT_IP_ADDRESS_SUCCEED;
        my_sslClient_connected_return_list = (const bool*)ConnectedReturnList_ft;
        my_sslClient_connect_return_list = (const int*)ConnectReturnList_true;
        my_sslClient_send_return_list = (const size_t*)SendReturnList_once;

        STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG)).IgnoreArgument(1);
        STRICT_EXPECTED_CALL(sslClient_new());
        STRICT_EXPECTED_CALL(sslClient_setTimeout(SSLClientHandel, 10000));
        STRICT_EXPECTED_CALL(sslClient_hostByName(TEST_CREATE_CONNECTION_HOST_NAME, IGNORED_PTR_ARG)).IgnoreArgument(2);
        STRICT_EXPECTED_CALL(sslClient_connected(SSLClientHandel));
        STRICT_EXPECTED_CALL(sslClient_connect(SSLClientHandel, SSLCLIENT_IP_ADDRESS, TEST_CREATE_CONNECTION_PORT));
        STRICT_EXPECTED_CALL(sslClient_connected(SSLClientHandel));

        tlsioHandle = tlsioInterfaces->concrete_io_create((void*)&tlsioConfig);
        ASSERT_IS_NOT_NULL(tlsioHandle);

        result = tlsioInterfaces->concrete_io_open(
            tlsioHandle,
            on_io_open_complete, CallbackContext,
            on_bytes_received, CallbackContext,
            on_io_error, CallbackContext);
        ASSERT_ARE_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(int, (int)TLSIO_ARDUINO_STATE_OPEN, (int)tlsio_arduino_get_state(tlsioHandle));

        ///act
        result = tlsioInterfaces->concrete_io_send(tlsioHandle, (const void*)SendBuffer, 0, on_send_complete, CallbackContext);

        ///assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_ARE_NOT_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(int, 1, currentmalloc_call);
        ASSERT_CALLBACK_COUNTERS(0, 1, 0, 0, 0);
        ASSERT_ARE_EQUAL(int, (int)TLSIO_ARDUINO_STATE_OPEN, (int)tlsio_arduino_get_state(tlsioHandle));

        ///cleanup
        result = tlsioInterfaces->concrete_io_close(tlsioHandle, on_io_close_complete, CallbackContext);
        tlsioInterfaces->concrete_io_destroy(tlsioHandle);
    }

    /* Tests_SRS_TLSIO_ARDUINO_21_060: [ If the buffer is NULL, the tlsio_arduino_send shall not do anything, and return _LINE_. ]*/
    TEST_FUNCTION(tlsio_arduino_send__NULL_txBuffer__failed)
    {
        ///arrange
        CONCRETE_IO_HANDLE tlsioHandle;
        int result;
        const IO_INTERFACE_DESCRIPTION* tlsioInterfaces = tlsio_arduino_get_interface_description();
        ASSERT_IS_NOT_NULL(tlsioInterfaces);

        my_sslClient_new_return = SSLClientHandel;
        my_sslClient_hostByName_return = SSLCLIENT_IP_ADDRESS_SUCCEED;
        my_sslClient_connected_return_list = (const bool*)ConnectedReturnList_ft;
        my_sslClient_connect_return_list = (const int*)ConnectReturnList_true;
        my_sslClient_send_return_list = (const size_t*)SendReturnList_once;

        STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG)).IgnoreArgument(1);
        STRICT_EXPECTED_CALL(sslClient_new());
        STRICT_EXPECTED_CALL(sslClient_setTimeout(SSLClientHandel, 10000));
        STRICT_EXPECTED_CALL(sslClient_hostByName(TEST_CREATE_CONNECTION_HOST_NAME, IGNORED_PTR_ARG)).IgnoreArgument(2);
        STRICT_EXPECTED_CALL(sslClient_connected(SSLClientHandel));
        STRICT_EXPECTED_CALL(sslClient_connect(SSLClientHandel, SSLCLIENT_IP_ADDRESS, TEST_CREATE_CONNECTION_PORT));
        STRICT_EXPECTED_CALL(sslClient_connected(SSLClientHandel));

        tlsioHandle = tlsioInterfaces->concrete_io_create((void*)&tlsioConfig);
        ASSERT_IS_NOT_NULL(tlsioHandle);

        result = tlsioInterfaces->concrete_io_open(
            tlsioHandle,
            on_io_open_complete, CallbackContext,
            on_bytes_received, CallbackContext,
            on_io_error, CallbackContext);
        ASSERT_ARE_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(int, (int)TLSIO_ARDUINO_STATE_OPEN, (int)tlsio_arduino_get_state(tlsioHandle));

        ///act
        result = tlsioInterfaces->concrete_io_send(tlsioHandle, (const void*)NULL, SendBufferSize, on_send_complete, CallbackContext);

        ///assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_ARE_NOT_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(int, 1, currentmalloc_call);
        ASSERT_CALLBACK_COUNTERS(0, 1, 0, 0, 0);
        ASSERT_ARE_EQUAL(int, (int)TLSIO_ARDUINO_STATE_OPEN, (int)tlsio_arduino_get_state(tlsioHandle));

        ///cleanup
        result = tlsioInterfaces->concrete_io_close(tlsioHandle, on_io_close_complete, CallbackContext);
        tlsioInterfaces->concrete_io_destroy(tlsioHandle);
    }

    /* Tests_SRS_TLSIO_ARDUINO_21_059: [ If the tlsio_handle is NULL, the tlsio_arduino_send shall not do anything, and return _LINE_. ]*/
    TEST_FUNCTION(tlsio_arduino_send__NULL_tlsio_handle__failed)
    {
        ///arrange
        int result;

        const IO_INTERFACE_DESCRIPTION* tlsioInterfaces = tlsio_arduino_get_interface_description();
        ASSERT_IS_NOT_NULL(tlsioInterfaces);

        ///act
        result = tlsioInterfaces->concrete_io_send(NULL, (const void*)SendBuffer, SendBufferSize, on_send_complete, CallbackContext);

        ///assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_ARE_NOT_EQUAL(int, 0, result);

        ///cleanup
    }

    /* Tests_SRS_TLSIO_ARDUINO_21_058: [ If the tlsio state is TLSIO_ARDUINO_STATE_ERROR, TLSIO_ARDUINO_STATE_OPENING, TLSIO_ARDUINO_STATE_CLOSING, or TLSIO_ARDUINO_STATE_CLOSED, the tlsio_arduino_send shall call the on_send_complete with IO_SEND_ERROR, and return _LINE_. ]*/
    TEST_FUNCTION(tlsio_arduino_send__on_TLSIO_ARDUINO_STATE_CLOSED__never_opened__failed)
    {
        ///arrange
        CONCRETE_IO_HANDLE tlsioHandle;
        int result;
        const IO_INTERFACE_DESCRIPTION* tlsioInterfaces = tlsio_arduino_get_interface_description();
        ASSERT_IS_NOT_NULL(tlsioInterfaces);

        my_sslClient_new_return = SSLClientHandel;
        my_sslClient_hostByName_return = SSLCLIENT_IP_ADDRESS_SUCCEED;

        STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG)).IgnoreArgument(1);
        STRICT_EXPECTED_CALL(sslClient_new());
        STRICT_EXPECTED_CALL(sslClient_setTimeout(SSLClientHandel, 10000));
        STRICT_EXPECTED_CALL(sslClient_hostByName(TEST_CREATE_CONNECTION_HOST_NAME, IGNORED_PTR_ARG)).IgnoreArgument(2);

        tlsioHandle = tlsioInterfaces->concrete_io_create((void*)&tlsioConfig);
        ASSERT_IS_NOT_NULL(tlsioHandle);
        ASSERT_ARE_EQUAL(int, (int)TLSIO_ARDUINO_STATE_CLOSED, (int)tlsio_arduino_get_state(tlsioHandle));

        ///act
        result = tlsioInterfaces->concrete_io_send(tlsioHandle, (const void*)SendBuffer, SendBufferSize, on_send_complete, CallbackContext);

        ///assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_ARE_NOT_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(int, 1, currentmalloc_call);
        ASSERT_CALLBACK_COUNTERS(0, 0, 0, 0, 0);
        ASSERT_ARE_EQUAL(int, (int)TLSIO_ARDUINO_STATE_CLOSED, (int)tlsio_arduino_get_state(tlsioHandle));

        ///cleanup
        tlsioInterfaces->concrete_io_destroy(tlsioHandle);
    }

    /* Tests_SRS_TLSIO_ARDUINO_21_058: [ If the tlsio state is TLSIO_ARDUINO_STATE_ERROR, TLSIO_ARDUINO_STATE_OPENING, TLSIO_ARDUINO_STATE_CLOSING, or TLSIO_ARDUINO_STATE_CLOSED, the tlsio_arduino_send shall call the on_send_complete with IO_SEND_ERROR, and return _LINE_. ]*/
    TEST_FUNCTION(tlsio_arduino_send__on_TLSIO_ARDUINO_STATE_CLOSED__failed)
    {
        ///arrange
        CONCRETE_IO_HANDLE tlsioHandle;
        int result;
        const IO_INTERFACE_DESCRIPTION* tlsioInterfaces = tlsio_arduino_get_interface_description();
        ASSERT_IS_NOT_NULL(tlsioInterfaces);

        my_sslClient_new_return = SSLClientHandel;
        my_sslClient_hostByName_return = SSLCLIENT_IP_ADDRESS_SUCCEED;
        my_sslClient_connected_return_list = (const bool*)ConnectedReturnList_ftf;
        my_sslClient_connect_return_list = (const int*)ConnectReturnList_true;

        STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG)).IgnoreArgument(1);
        STRICT_EXPECTED_CALL(sslClient_new());
        STRICT_EXPECTED_CALL(sslClient_setTimeout(SSLClientHandel, 10000));
        STRICT_EXPECTED_CALL(sslClient_hostByName(TEST_CREATE_CONNECTION_HOST_NAME, IGNORED_PTR_ARG)).IgnoreArgument(2);
        STRICT_EXPECTED_CALL(sslClient_connected(SSLClientHandel));
        STRICT_EXPECTED_CALL(sslClient_connect(SSLClientHandel, SSLCLIENT_IP_ADDRESS, TEST_CREATE_CONNECTION_PORT));
        STRICT_EXPECTED_CALL(sslClient_connected(SSLClientHandel));
        STRICT_EXPECTED_CALL(sslClient_stop(SSLClientHandel));
        STRICT_EXPECTED_CALL(sslClient_connected(SSLClientHandel));

        tlsioHandle = tlsioInterfaces->concrete_io_create((void*)&tlsioConfig);
        ASSERT_IS_NOT_NULL(tlsioHandle);
        result = tlsioInterfaces->concrete_io_open(
            tlsioHandle,
            on_io_open_complete, CallbackContext,
            on_bytes_received, CallbackContext,
            on_io_error, CallbackContext);
        ASSERT_ARE_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(int, (int)TLSIO_ARDUINO_STATE_OPEN, (int)tlsio_arduino_get_state(tlsioHandle));

        result = tlsioInterfaces->concrete_io_close(tlsioHandle, on_io_close_complete, CallbackContext);
        ASSERT_ARE_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(int, (int)TLSIO_ARDUINO_STATE_CLOSED, (int)tlsio_arduino_get_state(tlsioHandle));

        ///act
        result = tlsioInterfaces->concrete_io_send(tlsioHandle, (const void*)SendBuffer, SendBufferSize, on_send_complete, CallbackContext);

        ///assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_ARE_NOT_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(int, 1, currentmalloc_call);
        ASSERT_CALLBACK_COUNTERS(0, 1, 1, 0, 0);
        ASSERT_ARE_EQUAL(int, (int)TLSIO_ARDUINO_STATE_CLOSED, (int)tlsio_arduino_get_state(tlsioHandle));

        ///cleanup
        tlsioInterfaces->concrete_io_destroy(tlsioHandle);
    }

    /* Tests_SRS_TLSIO_ARDUINO_21_058: [ If the tlsio state is TLSIO_ARDUINO_STATE_ERROR, TLSIO_ARDUINO_STATE_OPENING, TLSIO_ARDUINO_STATE_CLOSING, or TLSIO_ARDUINO_STATE_CLOSED, the tlsio_arduino_send shall call the on_send_complete with IO_SEND_ERROR, and return _LINE_. ]*/
    TEST_FUNCTION(tlsio_arduino_send__on_TLSIO_ARDUINO_STATE_CLOSING__failed)
    {
        ///arrange
        CONCRETE_IO_HANDLE tlsioHandle;
        int result;
        const IO_INTERFACE_DESCRIPTION* tlsioInterfaces = tlsio_arduino_get_interface_description();
        ASSERT_IS_NOT_NULL(tlsioInterfaces);

        my_sslClient_new_return = SSLClientHandel;
        my_sslClient_hostByName_return = SSLCLIENT_IP_ADDRESS_SUCCEED;
        my_sslClient_connected_return_list = (const bool*)ConnectedReturnList_ftt;
        my_sslClient_connect_return_list = (const int*)ConnectReturnList_true;

        STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG)).IgnoreArgument(1);
        STRICT_EXPECTED_CALL(sslClient_new());
        STRICT_EXPECTED_CALL(sslClient_setTimeout(SSLClientHandel, 10000));
        STRICT_EXPECTED_CALL(sslClient_hostByName(TEST_CREATE_CONNECTION_HOST_NAME, IGNORED_PTR_ARG)).IgnoreArgument(2);
        STRICT_EXPECTED_CALL(sslClient_connected(SSLClientHandel));
        STRICT_EXPECTED_CALL(sslClient_connect(SSLClientHandel, SSLCLIENT_IP_ADDRESS, TEST_CREATE_CONNECTION_PORT));
        STRICT_EXPECTED_CALL(sslClient_connected(SSLClientHandel));
        STRICT_EXPECTED_CALL(sslClient_stop(SSLClientHandel));
        STRICT_EXPECTED_CALL(sslClient_connected(SSLClientHandel));

        tlsioHandle = tlsioInterfaces->concrete_io_create((void*)&tlsioConfig);
        ASSERT_IS_NOT_NULL(tlsioHandle);
        result = tlsioInterfaces->concrete_io_open(
            tlsioHandle,
            on_io_open_complete, CallbackContext,
            on_bytes_received, CallbackContext,
            on_io_error, CallbackContext);
        ASSERT_ARE_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(int, (int)TLSIO_ARDUINO_STATE_OPEN, (int)tlsio_arduino_get_state(tlsioHandle));

        result = tlsioInterfaces->concrete_io_close(tlsioHandle, on_io_close_complete, CallbackContext);
        ASSERT_ARE_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(int, (int)TLSIO_ARDUINO_STATE_CLOSING, (int)tlsio_arduino_get_state(tlsioHandle));

        ///act
        result = tlsioInterfaces->concrete_io_send(tlsioHandle, (const void*)SendBuffer, SendBufferSize, on_send_complete, CallbackContext);

        ///assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_ARE_NOT_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(int, 1, currentmalloc_call);
        ASSERT_CALLBACK_COUNTERS(0, 1, 0, 0, 0);
        ASSERT_ARE_EQUAL(int, (int)TLSIO_ARDUINO_STATE_CLOSING, (int)tlsio_arduino_get_state(tlsioHandle));

        ///cleanup
        tlsioInterfaces->concrete_io_destroy(tlsioHandle);
    }

    /* Tests_SRS_TLSIO_ARDUINO_21_058: [ If the tlsio state is TLSIO_ARDUINO_STATE_ERROR, TLSIO_ARDUINO_STATE_OPENING, TLSIO_ARDUINO_STATE_CLOSING, or TLSIO_ARDUINO_STATE_CLOSED, the tlsio_arduino_send shall call the on_send_complete with IO_SEND_ERROR, and return _LINE_. ]*/
    TEST_FUNCTION(tlsio_arduino_send__on_TLSIO_ARDUINO_STATE_OPENING__failed)
    {
        ///arrange
        CONCRETE_IO_HANDLE tlsioHandle;
        int result;
        const IO_INTERFACE_DESCRIPTION* tlsioInterfaces = tlsio_arduino_get_interface_description();
        ASSERT_IS_NOT_NULL(tlsioInterfaces);

        my_sslClient_new_return = SSLClientHandel;
        my_sslClient_hostByName_return = SSLCLIENT_IP_ADDRESS_SUCCEED;
        my_sslClient_connected_return_list = (const bool*)ConnectedReturnList_ff;
        my_sslClient_connect_return_list = (const int*)ConnectReturnList_true;

        STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG)).IgnoreArgument(1);
        STRICT_EXPECTED_CALL(sslClient_new());
        STRICT_EXPECTED_CALL(sslClient_setTimeout(SSLClientHandel, 10000));
        STRICT_EXPECTED_CALL(sslClient_hostByName(TEST_CREATE_CONNECTION_HOST_NAME, IGNORED_PTR_ARG)).IgnoreArgument(2);
        STRICT_EXPECTED_CALL(sslClient_connected(SSLClientHandel));
        STRICT_EXPECTED_CALL(sslClient_connect(SSLClientHandel, SSLCLIENT_IP_ADDRESS, TEST_CREATE_CONNECTION_PORT));
        STRICT_EXPECTED_CALL(sslClient_connected(SSLClientHandel));

        tlsioHandle = tlsioInterfaces->concrete_io_create((void*)&tlsioConfig);
        ASSERT_IS_NOT_NULL(tlsioHandle);
        result = tlsioInterfaces->concrete_io_open(
            tlsioHandle,
            on_io_open_complete, CallbackContext,
            on_bytes_received, CallbackContext,
            on_io_error, CallbackContext);
        ASSERT_ARE_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(int, (int)TLSIO_ARDUINO_STATE_OPENING, (int)tlsio_arduino_get_state(tlsioHandle));

        ///act
        result = tlsioInterfaces->concrete_io_send(tlsioHandle, (const void*)SendBuffer, SendBufferSize, on_send_complete, CallbackContext);

        ///assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_ARE_NOT_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(int, 1, currentmalloc_call);
        ASSERT_CALLBACK_COUNTERS(0, 0, 0, 0, 0);
        ASSERT_ARE_EQUAL(int, (int)TLSIO_ARDUINO_STATE_OPENING, (int)tlsio_arduino_get_state(tlsioHandle));

        ///cleanup
        result = tlsioInterfaces->concrete_io_close(tlsioHandle, on_io_close_complete, CallbackContext);
        tlsioInterfaces->concrete_io_destroy(tlsioHandle);
    }

    /* Tests_SRS_TLSIO_ARDUINO_21_058: [ If the tlsio state is TLSIO_ARDUINO_STATE_ERROR, TLSIO_ARDUINO_STATE_OPENING, TLSIO_ARDUINO_STATE_CLOSING, or TLSIO_ARDUINO_STATE_CLOSED, the tlsio_arduino_send shall call the on_send_complete with IO_SEND_ERROR, and return _LINE_. ]*/
    TEST_FUNCTION(tlsio_arduino_send__on_TLSIO_ARDUINO_STATE_ERROR__failed)
    {
        ///arrange
        CONCRETE_IO_HANDLE tlsioHandle;
        int result;
        const IO_INTERFACE_DESCRIPTION* tlsioInterfaces = tlsio_arduino_get_interface_description();
        ASSERT_IS_NOT_NULL(tlsioInterfaces);

        my_sslClient_new_return = SSLClientHandel;
        my_sslClient_hostByName_return = SSLCLIENT_IP_ADDRESS_SUCCEED;
        my_sslClient_connected_return_list = (const bool*)ConnectedReturnList_ft;
        my_sslClient_connect_return_list = (const int*)ConnectReturnList_true;

        STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG)).IgnoreArgument(1);
        STRICT_EXPECTED_CALL(sslClient_new());
        STRICT_EXPECTED_CALL(sslClient_setTimeout(SSLClientHandel, 10000));
        STRICT_EXPECTED_CALL(sslClient_hostByName(TEST_CREATE_CONNECTION_HOST_NAME, IGNORED_PTR_ARG)).IgnoreArgument(2);
        STRICT_EXPECTED_CALL(sslClient_connected(SSLClientHandel));
        STRICT_EXPECTED_CALL(sslClient_connect(SSLClientHandel, SSLCLIENT_IP_ADDRESS, TEST_CREATE_CONNECTION_PORT));
        STRICT_EXPECTED_CALL(sslClient_connected(SSLClientHandel));

        tlsioHandle = tlsioInterfaces->concrete_io_create((void*)&tlsioConfig);
        ASSERT_IS_NOT_NULL(tlsioHandle);
        result = tlsioInterfaces->concrete_io_open(
            tlsioHandle,
            on_io_open_complete, CallbackContext,
            on_bytes_received, CallbackContext,
            on_io_error, CallbackContext);
        result = tlsioInterfaces->concrete_io_open(
            tlsioHandle,
            on_io_open_complete, CallbackContext,
            on_bytes_received, CallbackContext,
            on_io_error, CallbackContext);
        ASSERT_ARE_NOT_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(int, (int)TLSIO_ARDUINO_STATE_ERROR, (int)tlsio_arduino_get_state(tlsioHandle));

        ///act
        result = tlsioInterfaces->concrete_io_send(tlsioHandle, (const void*)SendBuffer, SendBufferSize, on_send_complete, CallbackContext);

        ///assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_ARE_NOT_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(int, 1, currentmalloc_call);
        ASSERT_CALLBACK_COUNTERS(1, 2, 0, 0, 0);
        ASSERT_ARE_EQUAL(int, (int)TLSIO_ARDUINO_STATE_ERROR, (int)tlsio_arduino_get_state(tlsioHandle));

        ///cleanup
        tlsioInterfaces->concrete_io_destroy(tlsioHandle);
    }

    /* Tests_SRS_TLSIO_ARDUINO_21_056: [ if the ssl was not able to send any byte in the buffer, the tlsio_arduino_send shall call the on_send_complete with IO_SEND_ERROR, and return _LINE_. ]*/
    TEST_FUNCTION(tlsio_arduino_send__2_parts_with_0_byte_tx__failed)
    {
        ///arrange
        CONCRETE_IO_HANDLE tlsioHandle;
        int result;
        const IO_INTERFACE_DESCRIPTION* tlsioInterfaces = tlsio_arduino_get_interface_description();
        ASSERT_IS_NOT_NULL(tlsioInterfaces);

        my_sslClient_new_return = SSLClientHandel;
        my_sslClient_hostByName_return = SSLCLIENT_IP_ADDRESS_SUCCEED;
        my_sslClient_connected_return_list = (const bool*)ConnectedReturnList_ftt;
        my_sslClient_connect_return_list = (const int*)ConnectReturnList_true;
        my_sslClient_send_return_list = (const size_t*)SendReturnList_twice_zero;

        STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG)).IgnoreArgument(1);
        STRICT_EXPECTED_CALL(sslClient_new());
        STRICT_EXPECTED_CALL(sslClient_setTimeout(SSLClientHandel, 10000));
        STRICT_EXPECTED_CALL(sslClient_hostByName(TEST_CREATE_CONNECTION_HOST_NAME, IGNORED_PTR_ARG)).IgnoreArgument(2);
        STRICT_EXPECTED_CALL(sslClient_connected(SSLClientHandel));
        STRICT_EXPECTED_CALL(sslClient_connect(SSLClientHandel, SSLCLIENT_IP_ADDRESS, TEST_CREATE_CONNECTION_PORT));
        STRICT_EXPECTED_CALL(sslClient_connected(SSLClientHandel));
        STRICT_EXPECTED_CALL(sslClient_write(SSLClientHandel, (uint8_t*)SendBuffer, SendBufferSize));
        STRICT_EXPECTED_CALL(sslClient_write(SSLClientHandel, (uint8_t*)SendBuffer + SendReturnList_twice[0], SendBufferSize - SendReturnList_twice[0]));

        tlsioHandle = tlsioInterfaces->concrete_io_create((void*)&tlsioConfig);
        ASSERT_IS_NOT_NULL(tlsioHandle);

        result = tlsioInterfaces->concrete_io_open(
            tlsioHandle,
            on_io_open_complete, CallbackContext,
            on_bytes_received, CallbackContext,
            on_io_error, CallbackContext);
        ASSERT_ARE_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(int, (int)TLSIO_ARDUINO_STATE_OPEN, (int)tlsio_arduino_get_state(tlsioHandle));

        ///act
        result = tlsioInterfaces->concrete_io_send(tlsioHandle, (const void*)SendBuffer, SendBufferSize, on_send_complete, CallbackContext);

        ///assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_ARE_NOT_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(int, 1, currentmalloc_call);
        ASSERT_CALLBACK_COUNTERS(0, 1, 0, 1, 0);
        ASSERT_ARE_EQUAL(int, (int)IO_SEND_ERROR, (int)on_send_complete_result);
        ASSERT_ARE_EQUAL(int, (int)TLSIO_ARDUINO_STATE_OPEN, (int)tlsio_arduino_get_state(tlsioHandle));

        ///cleanup
        result = tlsioInterfaces->concrete_io_close(tlsioHandle, on_io_close_complete, CallbackContext);
        tlsioInterfaces->concrete_io_destroy(tlsioHandle);
    }

    /* Tests_SRS_TLSIO_ARDUINO_21_003: [ The tlsio_arduino shall report the send operation status using the IO_SEND_RESULT enumerator defined in the `xio.h`. ]*/
    /* Tests_SRS_TLSIO_ARDUINO_21_056: [ if the ssl was not able to send any byte in the buffer, the tlsio_arduino_send shall call the on_send_complete with IO_SEND_ERROR, and return _LINE_. ]*/
    TEST_FUNCTION(tlsio_arduino_send__0_bytes_tx__failed)
    {
        ///arrange
        CONCRETE_IO_HANDLE tlsioHandle;
        int result;
        const IO_INTERFACE_DESCRIPTION* tlsioInterfaces = tlsio_arduino_get_interface_description();
        ASSERT_IS_NOT_NULL(tlsioInterfaces);

        my_sslClient_new_return = SSLClientHandel;
        my_sslClient_hostByName_return = SSLCLIENT_IP_ADDRESS_SUCCEED;
        my_sslClient_connected_return_list = (const bool*)ConnectedReturnList_ft;
        my_sslClient_connect_return_list = (const int*)ConnectReturnList_true;
        my_sslClient_send_return_list = (const size_t*)SendReturnList_once_zero;

        STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG)).IgnoreArgument(1);
        STRICT_EXPECTED_CALL(sslClient_new());
        STRICT_EXPECTED_CALL(sslClient_setTimeout(SSLClientHandel, 10000));
        STRICT_EXPECTED_CALL(sslClient_hostByName(TEST_CREATE_CONNECTION_HOST_NAME, IGNORED_PTR_ARG)).IgnoreArgument(2);
        STRICT_EXPECTED_CALL(sslClient_connected(SSLClientHandel));
        STRICT_EXPECTED_CALL(sslClient_connect(SSLClientHandel, SSLCLIENT_IP_ADDRESS, TEST_CREATE_CONNECTION_PORT));
        STRICT_EXPECTED_CALL(sslClient_connected(SSLClientHandel));
        STRICT_EXPECTED_CALL(sslClient_write(SSLClientHandel, (uint8_t*)SendBuffer, SendBufferSize));

        tlsioHandle = tlsioInterfaces->concrete_io_create((void*)&tlsioConfig);
        ASSERT_IS_NOT_NULL(tlsioHandle);

        result = tlsioInterfaces->concrete_io_open(
            tlsioHandle,
            on_io_open_complete, CallbackContext,
            on_bytes_received, CallbackContext,
            on_io_error, CallbackContext);
        ASSERT_ARE_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(int, (int)TLSIO_ARDUINO_STATE_OPEN, (int)tlsio_arduino_get_state(tlsioHandle));

        ///act
        result = tlsioInterfaces->concrete_io_send(tlsioHandle, (const void*)SendBuffer, SendBufferSize, on_send_complete, CallbackContext);

        ///assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_ARE_NOT_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(int, 1, currentmalloc_call);
        ASSERT_CALLBACK_COUNTERS(0, 1, 0, 1, 0);
        ASSERT_ARE_EQUAL(int, (int)IO_SEND_ERROR, (int)on_send_complete_result);
        ASSERT_ARE_EQUAL(int, (int)TLSIO_ARDUINO_STATE_OPEN, (int)tlsio_arduino_get_state(tlsioHandle));

        ///cleanup
        result = tlsioInterfaces->concrete_io_close(tlsioHandle, on_io_close_complete, CallbackContext);
        tlsioInterfaces->concrete_io_destroy(tlsioHandle);
    }

    /* Tests_SRS_TLSIO_ARDUINO_21_055: [ if the ssl was not able to send all data in the buffer, the tlsio_arduino_send shall call the ssl again to send the remaining bytes. ]*/
    /* Tests_SRS_TLSIO_ARDUINO_21_057: [ if the ssl finish to send all bytes in the buffer, the tlsio_arduino_send shall call the on_send_complete with IO_SEND_OK, and return 0 ]*/
    TEST_FUNCTION(tlsio_arduino_send__2_parts__succeed)
    {
        ///arrange
        CONCRETE_IO_HANDLE tlsioHandle;
        int result;
        const IO_INTERFACE_DESCRIPTION* tlsioInterfaces = tlsio_arduino_get_interface_description();
        ASSERT_IS_NOT_NULL(tlsioInterfaces);

        my_sslClient_new_return = SSLClientHandel;
        my_sslClient_hostByName_return = SSLCLIENT_IP_ADDRESS_SUCCEED;
        my_sslClient_connected_return_list = (const bool*)ConnectedReturnList_ft;
        my_sslClient_connect_return_list = (const int*)ConnectReturnList_true;
        my_sslClient_send_return_list = (const size_t*)SendReturnList_twice;

        STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG)).IgnoreArgument(1);
        STRICT_EXPECTED_CALL(sslClient_new());
        STRICT_EXPECTED_CALL(sslClient_setTimeout(SSLClientHandel, 10000));
        STRICT_EXPECTED_CALL(sslClient_hostByName(TEST_CREATE_CONNECTION_HOST_NAME, IGNORED_PTR_ARG)).IgnoreArgument(2);
        STRICT_EXPECTED_CALL(sslClient_connected(SSLClientHandel));
        STRICT_EXPECTED_CALL(sslClient_connect(SSLClientHandel, SSLCLIENT_IP_ADDRESS, TEST_CREATE_CONNECTION_PORT));
        STRICT_EXPECTED_CALL(sslClient_connected(SSLClientHandel));
        STRICT_EXPECTED_CALL(sslClient_write(SSLClientHandel, (uint8_t*)SendBuffer, SendBufferSize));
        STRICT_EXPECTED_CALL(sslClient_write(SSLClientHandel, (uint8_t*)SendBuffer + SendReturnList_twice[0], SendBufferSize - SendReturnList_twice[0]));

        tlsioHandle = tlsioInterfaces->concrete_io_create((void*)&tlsioConfig);
        ASSERT_IS_NOT_NULL(tlsioHandle);

        result = tlsioInterfaces->concrete_io_open(
            tlsioHandle,
            on_io_open_complete, CallbackContext,
            on_bytes_received, CallbackContext,
            on_io_error, CallbackContext);
        ASSERT_ARE_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(int, (int)TLSIO_ARDUINO_STATE_OPEN, (int)tlsio_arduino_get_state(tlsioHandle));

        ///act
        result = tlsioInterfaces->concrete_io_send(tlsioHandle, (const void*)SendBuffer, SendBufferSize, on_send_complete, CallbackContext);

        ///assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_ARE_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(int, 1, currentmalloc_call);
        ASSERT_CALLBACK_COUNTERS(0, 1, 0, 1, 0);
        ASSERT_ARE_EQUAL(int, (int)IO_SEND_OK, (int)on_send_complete_result);
        ASSERT_ARE_EQUAL(int, (int)TLSIO_ARDUINO_STATE_OPEN, (int)tlsio_arduino_get_state(tlsioHandle));

        ///cleanup
        result = tlsioInterfaces->concrete_io_close(tlsioHandle, on_io_close_complete, CallbackContext);
        tlsioInterfaces->concrete_io_destroy(tlsioHandle);
    }

    /* Tests_SRS_TLSIO_ARDUINO_21_052: [ The tlsio_arduino_send shall send all bytes in a buffer to the ssl connectio. ]*/
    /* Tests_SRS_TLSIO_ARDUINO_21_053: [ The tlsio_arduino_send shall use the provided on_send_complete callback function address. ]*/
    /* Tests_SRS_TLSIO_ARDUINO_21_054: [ The tlsio_arduino_send shall use the provided on_send_complete_context handle. ]*/
    TEST_FUNCTION(tlsio_arduino_send__no_send_callback__succeed)
    {
        ///arrange
        CONCRETE_IO_HANDLE tlsioHandle;
        int result;
        const IO_INTERFACE_DESCRIPTION* tlsioInterfaces = tlsio_arduino_get_interface_description();
        ASSERT_IS_NOT_NULL(tlsioInterfaces);

        my_sslClient_new_return = SSLClientHandel;
        my_sslClient_hostByName_return = SSLCLIENT_IP_ADDRESS_SUCCEED;
        my_sslClient_connected_return_list = (const bool*)ConnectedReturnList_ft;
        my_sslClient_connect_return_list = (const int*)ConnectReturnList_true;
        my_sslClient_send_return_list = (const size_t*)SendReturnList_once;

        STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG)).IgnoreArgument(1);
        STRICT_EXPECTED_CALL(sslClient_new());
        STRICT_EXPECTED_CALL(sslClient_setTimeout(SSLClientHandel, 10000));
        STRICT_EXPECTED_CALL(sslClient_hostByName(TEST_CREATE_CONNECTION_HOST_NAME, IGNORED_PTR_ARG)).IgnoreArgument(2);
        STRICT_EXPECTED_CALL(sslClient_connected(SSLClientHandel));
        STRICT_EXPECTED_CALL(sslClient_connect(SSLClientHandel, SSLCLIENT_IP_ADDRESS, TEST_CREATE_CONNECTION_PORT));
        STRICT_EXPECTED_CALL(sslClient_connected(SSLClientHandel));
        STRICT_EXPECTED_CALL(sslClient_write(SSLClientHandel, (uint8_t*)SendBuffer, SendBufferSize));

        tlsioHandle = tlsioInterfaces->concrete_io_create((void*)&tlsioConfig);
        ASSERT_IS_NOT_NULL(tlsioHandle);

        result = tlsioInterfaces->concrete_io_open(
            tlsioHandle,
            on_io_open_complete, CallbackContext,
            on_bytes_received, CallbackContext,
            on_io_error, CallbackContext);
        ASSERT_ARE_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(int, (int)TLSIO_ARDUINO_STATE_OPEN, (int)tlsio_arduino_get_state(tlsioHandle));

        ///act
        result = tlsioInterfaces->concrete_io_send(tlsioHandle, (const void*)SendBuffer, SendBufferSize, NULL, NULL);

        ///assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_ARE_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(int, 1, currentmalloc_call);
        ASSERT_CALLBACK_COUNTERS(0, 1, 0, 0, 0);
        ASSERT_ARE_EQUAL(int, (int)TLSIO_ARDUINO_STATE_OPEN, (int)tlsio_arduino_get_state(tlsioHandle));

        ///cleanup
        result = tlsioInterfaces->concrete_io_close(tlsioHandle, on_io_close_complete, CallbackContext);
        tlsioInterfaces->concrete_io_destroy(tlsioHandle);
    }

    /* Tests_SRS_TLSIO_ARDUINO_21_003: [ The tlsio_arduino shall report the send operation status using the IO_SEND_RESULT enumerator defined in the `xio.h`. ]*/
    /* Tests_SRS_TLSIO_ARDUINO_21_052: [ The tlsio_arduino_send shall send all bytes in a buffer to the ssl connectio. ]*/
    /* Tests_SRS_TLSIO_ARDUINO_21_053: [ The tlsio_arduino_send shall use the provided on_send_complete callback function address. ]*/
    /* Tests_SRS_TLSIO_ARDUINO_21_054: [ The tlsio_arduino_send shall use the provided on_send_complete_context handle. ]*/
    /* Tests_SRS_TLSIO_ARDUINO_21_057: [ if the ssl finish to send all bytes in the buffer, the tlsio_arduino_send shall call the on_send_complete with IO_SEND_OK, and return 0 ]*/
    TEST_FUNCTION(tlsio_arduino_send__succeed)
    {
        ///arrange
        CONCRETE_IO_HANDLE tlsioHandle;
        int result;
        const IO_INTERFACE_DESCRIPTION* tlsioInterfaces = tlsio_arduino_get_interface_description();
        ASSERT_IS_NOT_NULL(tlsioInterfaces);

        my_sslClient_new_return = SSLClientHandel;
        my_sslClient_hostByName_return = SSLCLIENT_IP_ADDRESS_SUCCEED;
        my_sslClient_connected_return_list = (const bool*)ConnectedReturnList_ft;
        my_sslClient_connect_return_list = (const int*)ConnectReturnList_true;
        my_sslClient_send_return_list = (const size_t*)SendReturnList_once;

        STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG)).IgnoreArgument(1);
        STRICT_EXPECTED_CALL(sslClient_new());
        STRICT_EXPECTED_CALL(sslClient_setTimeout(SSLClientHandel, 10000));
        STRICT_EXPECTED_CALL(sslClient_hostByName(TEST_CREATE_CONNECTION_HOST_NAME, IGNORED_PTR_ARG)).IgnoreArgument(2);
        STRICT_EXPECTED_CALL(sslClient_connected(SSLClientHandel));
        STRICT_EXPECTED_CALL(sslClient_connect(SSLClientHandel, SSLCLIENT_IP_ADDRESS, TEST_CREATE_CONNECTION_PORT));
        STRICT_EXPECTED_CALL(sslClient_connected(SSLClientHandel));
        STRICT_EXPECTED_CALL(sslClient_write(SSLClientHandel, (uint8_t*)SendBuffer, SendBufferSize));

        tlsioHandle = tlsioInterfaces->concrete_io_create((void*)&tlsioConfig);
        ASSERT_IS_NOT_NULL(tlsioHandle);

        result = tlsioInterfaces->concrete_io_open(
            tlsioHandle,
            on_io_open_complete, CallbackContext,
            on_bytes_received, CallbackContext,
            on_io_error, CallbackContext);
        ASSERT_ARE_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(int, (int)TLSIO_ARDUINO_STATE_OPEN, (int)tlsio_arduino_get_state(tlsioHandle));

        ///act
        result = tlsioInterfaces->concrete_io_send(tlsioHandle, (const void*)SendBuffer, SendBufferSize, on_send_complete, CallbackContext);

        ///assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_ARE_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(int, 1, currentmalloc_call);
        ASSERT_CALLBACK_COUNTERS(0, 1, 0, 1, 0);
        ASSERT_ARE_EQUAL(int, (int)IO_SEND_OK, (int)on_send_complete_result);
        ASSERT_ARE_EQUAL(void_ptr, CallbackContext, on_send_complete_context);
        ASSERT_ARE_EQUAL(int, (int)TLSIO_ARDUINO_STATE_OPEN, (int)tlsio_arduino_get_state(tlsioHandle));

        ///cleanup
        result = tlsioInterfaces->concrete_io_close(tlsioHandle, on_io_close_complete, CallbackContext);
        tlsioInterfaces->concrete_io_destroy(tlsioHandle);
    }

    /* Tests_SRS_TLSIO_ARDUINO_21_049: [ If the tlsio_handle is NULL, the tlsio_arduino_close shall not do anything, and return _LINE_. ]*/
    TEST_FUNCTION(tlsio_arduino_close__NULL_tlsio_handle__failed)
    {
        ///arrange
        int result;

        const IO_INTERFACE_DESCRIPTION* tlsioInterfaces = tlsio_arduino_get_interface_description();
        ASSERT_IS_NOT_NULL(tlsioInterfaces);

        ///act
        result = tlsioInterfaces->concrete_io_close(NULL, NULL, NULL);

        ///assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_ARE_NOT_EQUAL(int, 0, result);

        ///cleanup
    }

    /* Tests_SRS_TLSIO_ARDUINO_21_025: [ If the tlsio state is TLSIO_ARDUINO_STATE_OPENING, TLSIO_ARDUINO_STATE_OPEN, or TLSIO_ARDUINO_STATE_CLOSING, the tlsio_arduino_destroy shall close destroy the tlsio, but log an error. ]*/
    TEST_FUNCTION(tlsio_arduino_destroy__on_TLSIO_ARDUINO_STATE_CLOSING__failed)
    {
        ///arrange
        CONCRETE_IO_HANDLE tlsioHandle;
        int result;
        const IO_INTERFACE_DESCRIPTION* tlsioInterfaces = tlsio_arduino_get_interface_description();
        ASSERT_IS_NOT_NULL(tlsioInterfaces);

        my_sslClient_new_return = SSLClientHandel;
        my_sslClient_hostByName_return = SSLCLIENT_IP_ADDRESS_SUCCEED;
        my_sslClient_connected_return_list = (const bool*)ConnectedReturnList_ftt;
        my_sslClient_connect_return_list = (const int*)ConnectReturnList_true;

        STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG)).IgnoreArgument(1);
        STRICT_EXPECTED_CALL(sslClient_new());
        STRICT_EXPECTED_CALL(sslClient_setTimeout(SSLClientHandel, 10000));
        STRICT_EXPECTED_CALL(sslClient_hostByName(TEST_CREATE_CONNECTION_HOST_NAME, IGNORED_PTR_ARG)).IgnoreArgument(2);
        STRICT_EXPECTED_CALL(sslClient_connected(SSLClientHandel));
        STRICT_EXPECTED_CALL(sslClient_connect(SSLClientHandel, SSLCLIENT_IP_ADDRESS, TEST_CREATE_CONNECTION_PORT));
        STRICT_EXPECTED_CALL(sslClient_connected(SSLClientHandel));
        STRICT_EXPECTED_CALL(sslClient_stop(SSLClientHandel));
        STRICT_EXPECTED_CALL(sslClient_connected(SSLClientHandel));
        STRICT_EXPECTED_CALL(sslClient_delete(IGNORED_PTR_ARG)).IgnoreArgument(1);
        STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG)).IgnoreArgument(1);

        tlsioHandle = tlsioInterfaces->concrete_io_create((void*)&tlsioConfig);
        ASSERT_IS_NOT_NULL(tlsioHandle);

        result = tlsioInterfaces->concrete_io_open(
            tlsioHandle,
            on_io_open_complete, CallbackContext,
            on_bytes_received, CallbackContext,
            on_io_error, CallbackContext);
        ASSERT_ARE_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(int, (int)TLSIO_ARDUINO_STATE_OPEN, (int)tlsio_arduino_get_state(tlsioHandle));

        result = tlsioInterfaces->concrete_io_close(tlsioHandle, on_io_close_complete, CallbackContext);
        ASSERT_ARE_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(int, (int)TLSIO_ARDUINO_STATE_CLOSING, (int)tlsio_arduino_get_state(tlsioHandle));

        ///act
        tlsioInterfaces->concrete_io_destroy(tlsioHandle);

        ///assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_ARE_EQUAL(int, 0, currentmalloc_call);
        ASSERT_CALLBACK_COUNTERS(0, 1, 0, 0, 0);

        ///cleanup
    }

    /* Tests_SRS_TLSIO_ARDUINO_21_035: [ If the tlsio state is TLSIO_ARDUINO_STATE_ERROR, TLSIO_ARDUINO_STATE_OPENING, TLSIO_ARDUINO_STATE_OPEN, or TLSIO_ARDUINO_STATE_CLOSING, the tlsio_arduino_open shall set the tlsio state as TLSIO_ARDUINO_STATE_ERROR, and return _LINE_. ]*/
    TEST_FUNCTION(tlsio_arduino_open__on_TLSIO_ARDUINO_STATE_CLOSING__failed)
    {
        ///arrange
        CONCRETE_IO_HANDLE tlsioHandle;
        int result;
        const IO_INTERFACE_DESCRIPTION* tlsioInterfaces = tlsio_arduino_get_interface_description();
        ASSERT_IS_NOT_NULL(tlsioInterfaces);

        my_sslClient_new_return = SSLClientHandel;
        my_sslClient_hostByName_return = SSLCLIENT_IP_ADDRESS_SUCCEED;
        my_sslClient_connected_return_list = (const bool*)ConnectedReturnList_ftt;
        my_sslClient_connect_return_list = (const int*)ConnectReturnList_true;

        STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG)).IgnoreArgument(1);
        STRICT_EXPECTED_CALL(sslClient_new());
        STRICT_EXPECTED_CALL(sslClient_setTimeout(SSLClientHandel, 10000));
        STRICT_EXPECTED_CALL(sslClient_hostByName(TEST_CREATE_CONNECTION_HOST_NAME, IGNORED_PTR_ARG)).IgnoreArgument(2);
        STRICT_EXPECTED_CALL(sslClient_connected(SSLClientHandel));
        STRICT_EXPECTED_CALL(sslClient_connect(SSLClientHandel, SSLCLIENT_IP_ADDRESS, TEST_CREATE_CONNECTION_PORT));
        STRICT_EXPECTED_CALL(sslClient_connected(SSLClientHandel));
        STRICT_EXPECTED_CALL(sslClient_stop(SSLClientHandel));
        STRICT_EXPECTED_CALL(sslClient_connected(SSLClientHandel));

        tlsioHandle = tlsioInterfaces->concrete_io_create((void*)&tlsioConfig);
        ASSERT_IS_NOT_NULL(tlsioHandle);

        result = tlsioInterfaces->concrete_io_open(
            tlsioHandle,
            on_io_open_complete, CallbackContext,
            on_bytes_received, CallbackContext,
            on_io_error, CallbackContext);
        ASSERT_ARE_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(int, (int)TLSIO_ARDUINO_STATE_OPEN, (int)tlsio_arduino_get_state(tlsioHandle));

        result = tlsioInterfaces->concrete_io_close(tlsioHandle, on_io_close_complete, CallbackContext);
        ASSERT_ARE_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(int, (int)TLSIO_ARDUINO_STATE_CLOSING, (int)tlsio_arduino_get_state(tlsioHandle));

        ///act
        result = tlsioInterfaces->concrete_io_open(
            tlsioHandle,
            on_io_open_complete, CallbackContext,
            on_bytes_received, CallbackContext,
            on_io_error, CallbackContext);

        ///assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_ARE_NOT_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(int, 1, currentmalloc_call);
        ASSERT_ARE_EQUAL(int, (int)IO_OPEN_ERROR, (int)on_io_open_complete_result);
        ASSERT_CALLBACK_COUNTERS(1, 2, 0, 0, 0);
        ASSERT_ARE_EQUAL(int, (int)TLSIO_ARDUINO_STATE_ERROR, (int)tlsio_arduino_get_state(tlsioHandle));

        ///cleanup
        tlsioInterfaces->concrete_io_destroy(tlsioHandle);
    }

    /* Tests_SRS_TLSIO_ARDUINO_21_048: [ If the tlsio state is TLSIO_ARDUINO_STATE_ERROR, TLSIO_ARDUINO_STATE_OPENING, TLSIO_ARDUINO_STATE_CLOSING, or TLSIO_ARDUINO_STATE_CLOSED, the tlsio_arduino_close shall set the tlsio state as TLSIO_ARDUINO_STATE_ERROR, call the on_io_error, and return _LINE_. ]*/
    TEST_FUNCTION(tlsio_arduino_close__on_TLSIO_ARDUINO_STATE_ERROR_no_error_callback__failed)
    {
        ///arrange
        CONCRETE_IO_HANDLE tlsioHandle;
        int result;
        const IO_INTERFACE_DESCRIPTION* tlsioInterfaces = tlsio_arduino_get_interface_description();
        ASSERT_IS_NOT_NULL(tlsioInterfaces);

        my_sslClient_new_return = SSLClientHandel;
        my_sslClient_hostByName_return = SSLCLIENT_IP_ADDRESS_SUCCEED;
        my_sslClient_connected_return_list = (const bool*)ConnectedReturnList_f;
        my_sslClient_connect_return_list = (const int*)ConnectReturnList_true;

        STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG)).IgnoreArgument(1);
        STRICT_EXPECTED_CALL(sslClient_new());
        STRICT_EXPECTED_CALL(sslClient_setTimeout(SSLClientHandel, 10000));
        STRICT_EXPECTED_CALL(sslClient_hostByName(TEST_CREATE_CONNECTION_HOST_NAME, IGNORED_PTR_ARG)).IgnoreArgument(2);

        tlsioHandle = tlsioInterfaces->concrete_io_create((void*)&tlsioConfig);
        ASSERT_IS_NOT_NULL(tlsioHandle);
        ASSERT_ARE_EQUAL(int, (int)TLSIO_ARDUINO_STATE_CLOSED, (int)tlsio_arduino_get_state(tlsioHandle));

        result = tlsioInterfaces->concrete_io_close(tlsioHandle, on_io_close_complete, CallbackContext);
        ASSERT_ARE_NOT_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(int, (int)TLSIO_ARDUINO_STATE_ERROR, (int)tlsio_arduino_get_state(tlsioHandle));

        ///act
        result = tlsioInterfaces->concrete_io_close(tlsioHandle, on_io_close_complete, CallbackContext);

        ///assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_ARE_NOT_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(int, 1, currentmalloc_call);
        ASSERT_CALLBACK_COUNTERS(0, 0, 0, 0, 0);
        ASSERT_ARE_EQUAL(int, (int)TLSIO_ARDUINO_STATE_ERROR, (int)tlsio_arduino_get_state(tlsioHandle));

        ///cleanup
        tlsioInterfaces->concrete_io_destroy(tlsioHandle);
    }

    /* Tests_SRS_TLSIO_ARDUINO_21_048: [ If the tlsio state is TLSIO_ARDUINO_STATE_ERROR, TLSIO_ARDUINO_STATE_OPENING, TLSIO_ARDUINO_STATE_CLOSING, or TLSIO_ARDUINO_STATE_CLOSED, the tlsio_arduino_close shall set the tlsio state as TLSIO_ARDUINO_STATE_ERROR, call the on_io_error, and return _LINE_. ]*/
    TEST_FUNCTION(tlsio_arduino_close__on_TLSIO_ARDUINO_STATE_OPENING__failed)
    {
        ///arrange
        CONCRETE_IO_HANDLE tlsioHandle;
        int result;
        const IO_INTERFACE_DESCRIPTION* tlsioInterfaces = tlsio_arduino_get_interface_description();
        ASSERT_IS_NOT_NULL(tlsioInterfaces);

        my_sslClient_new_return = SSLClientHandel;
        my_sslClient_hostByName_return = SSLCLIENT_IP_ADDRESS_SUCCEED;
        my_sslClient_connected_return_list = (const bool*)ConnectedReturnList_ff;
        my_sslClient_connect_return_list = (const int*)ConnectReturnList_true;

        STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG)).IgnoreArgument(1);
        STRICT_EXPECTED_CALL(sslClient_new());
        STRICT_EXPECTED_CALL(sslClient_setTimeout(SSLClientHandel, 10000));
        STRICT_EXPECTED_CALL(sslClient_hostByName(TEST_CREATE_CONNECTION_HOST_NAME, IGNORED_PTR_ARG)).IgnoreArgument(2);
        STRICT_EXPECTED_CALL(sslClient_connected(SSLClientHandel));
        STRICT_EXPECTED_CALL(sslClient_connect(SSLClientHandel, SSLCLIENT_IP_ADDRESS, TEST_CREATE_CONNECTION_PORT));
        STRICT_EXPECTED_CALL(sslClient_connected(SSLClientHandel));

        tlsioHandle = tlsioInterfaces->concrete_io_create((void*)&tlsioConfig);
        ASSERT_IS_NOT_NULL(tlsioHandle);

        result = tlsioInterfaces->concrete_io_open(
            tlsioHandle,
            on_io_open_complete, CallbackContext,
            on_bytes_received, CallbackContext,
            on_io_error, CallbackContext);
        ASSERT_ARE_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(int, (int)TLSIO_ARDUINO_STATE_OPENING, (int)tlsio_arduino_get_state(tlsioHandle));

        ///act
        result = tlsioInterfaces->concrete_io_close(tlsioHandle, on_io_close_complete, CallbackContext);

        ///assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_ARE_NOT_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(int, 1, currentmalloc_call);
        ASSERT_CALLBACK_COUNTERS(1, 0, 0, 0, 0);
        ASSERT_ARE_EQUAL(int, (int)TLSIO_ARDUINO_STATE_ERROR, (int)tlsio_arduino_get_state(tlsioHandle));

        ///cleanup
        tlsioInterfaces->concrete_io_destroy(tlsioHandle);
    }

    /* Tests_SRS_TLSIO_ARDUINO_21_048: [ If the tlsio state is TLSIO_ARDUINO_STATE_ERROR, TLSIO_ARDUINO_STATE_OPENING, TLSIO_ARDUINO_STATE_CLOSING, or TLSIO_ARDUINO_STATE_CLOSED, the tlsio_arduino_close shall set the tlsio state as TLSIO_ARDUINO_STATE_ERROR, call the on_io_error, and return _LINE_. ]*/
    TEST_FUNCTION(tlsio_arduino_close__on_TLSIO_ARDUINO_STATE_CLOSING__failed)
    {
        ///arrange
        CONCRETE_IO_HANDLE tlsioHandle;
        int result;
        const IO_INTERFACE_DESCRIPTION* tlsioInterfaces = tlsio_arduino_get_interface_description();
        ASSERT_IS_NOT_NULL(tlsioInterfaces);

        my_sslClient_new_return = SSLClientHandel;
        my_sslClient_hostByName_return = SSLCLIENT_IP_ADDRESS_SUCCEED;
        my_sslClient_connected_return_list = (const bool*)ConnectedReturnList_ftt;
        my_sslClient_connect_return_list = (const int*)ConnectReturnList_true;

        STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG)).IgnoreArgument(1);
        STRICT_EXPECTED_CALL(sslClient_new());
        STRICT_EXPECTED_CALL(sslClient_setTimeout(SSLClientHandel, 10000));
        STRICT_EXPECTED_CALL(sslClient_hostByName(TEST_CREATE_CONNECTION_HOST_NAME, IGNORED_PTR_ARG)).IgnoreArgument(2);
        STRICT_EXPECTED_CALL(sslClient_connected(SSLClientHandel));
        STRICT_EXPECTED_CALL(sslClient_connect(SSLClientHandel, SSLCLIENT_IP_ADDRESS, TEST_CREATE_CONNECTION_PORT));
        STRICT_EXPECTED_CALL(sslClient_connected(SSLClientHandel));
        STRICT_EXPECTED_CALL(sslClient_stop(SSLClientHandel));
        STRICT_EXPECTED_CALL(sslClient_connected(SSLClientHandel));

        tlsioHandle = tlsioInterfaces->concrete_io_create((void*)&tlsioConfig);
        ASSERT_IS_NOT_NULL(tlsioHandle);

        result = tlsioInterfaces->concrete_io_open(
            tlsioHandle,
            on_io_open_complete, CallbackContext,
            on_bytes_received, CallbackContext,
            on_io_error, CallbackContext);
        ASSERT_ARE_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(int, (int)TLSIO_ARDUINO_STATE_OPEN, (int)tlsio_arduino_get_state(tlsioHandle));

        result = tlsioInterfaces->concrete_io_close(tlsioHandle, on_io_close_complete, CallbackContext);
        ASSERT_ARE_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(int, (int)TLSIO_ARDUINO_STATE_CLOSING, (int)tlsio_arduino_get_state(tlsioHandle));

        ///act
        result = tlsioInterfaces->concrete_io_close(tlsioHandle, on_io_close_complete, CallbackContext);

        ///assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_ARE_NOT_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(int, 1, currentmalloc_call);
        ASSERT_CALLBACK_COUNTERS(1, 1, 0, 0, 0);
        ASSERT_ARE_EQUAL(int, (int)TLSIO_ARDUINO_STATE_ERROR, (int)tlsio_arduino_get_state(tlsioHandle));

        ///cleanup
        tlsioInterfaces->concrete_io_destroy(tlsioHandle);
    }

    /* Tests_SRS_TLSIO_ARDUINO_21_048: [ If the tlsio state is TLSIO_ARDUINO_STATE_ERROR, TLSIO_ARDUINO_STATE_OPENING, TLSIO_ARDUINO_STATE_CLOSING, or TLSIO_ARDUINO_STATE_CLOSED, the tlsio_arduino_close shall set the tlsio state as TLSIO_ARDUINO_STATE_ERROR, call the on_io_error, and return _LINE_. ]*/
    TEST_FUNCTION(tlsio_arduino_close__on_TLSIO_ARDUINO_STATE_CLOSED__failed)
    {
        ///arrange
        CONCRETE_IO_HANDLE tlsioHandle;
        int result;
        const IO_INTERFACE_DESCRIPTION* tlsioInterfaces = tlsio_arduino_get_interface_description();
        ASSERT_IS_NOT_NULL(tlsioInterfaces);

        my_sslClient_new_return = SSLClientHandel;
        my_sslClient_hostByName_return = SSLCLIENT_IP_ADDRESS_SUCCEED;
        my_sslClient_connected_return_list = (const bool*)ConnectedReturnList_ftf;
        my_sslClient_connect_return_list = (const int*)ConnectReturnList_true;

        STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG)).IgnoreArgument(1);
        STRICT_EXPECTED_CALL(sslClient_new());
        STRICT_EXPECTED_CALL(sslClient_setTimeout(SSLClientHandel, 10000));
        STRICT_EXPECTED_CALL(sslClient_hostByName(TEST_CREATE_CONNECTION_HOST_NAME, IGNORED_PTR_ARG)).IgnoreArgument(2);
        STRICT_EXPECTED_CALL(sslClient_connected(SSLClientHandel));
        STRICT_EXPECTED_CALL(sslClient_connect(SSLClientHandel, SSLCLIENT_IP_ADDRESS, TEST_CREATE_CONNECTION_PORT));
        STRICT_EXPECTED_CALL(sslClient_connected(SSLClientHandel));
        STRICT_EXPECTED_CALL(sslClient_stop(SSLClientHandel));
        STRICT_EXPECTED_CALL(sslClient_connected(SSLClientHandel));

        tlsioHandle = tlsioInterfaces->concrete_io_create((void*)&tlsioConfig);
        ASSERT_IS_NOT_NULL(tlsioHandle);

        result = tlsioInterfaces->concrete_io_open(
            tlsioHandle,
            on_io_open_complete, CallbackContext,
            on_bytes_received, CallbackContext,
            on_io_error, CallbackContext);
        ASSERT_ARE_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(int, (int)TLSIO_ARDUINO_STATE_OPEN, (int)tlsio_arduino_get_state(tlsioHandle));

        result = tlsioInterfaces->concrete_io_close(tlsioHandle, on_io_close_complete, CallbackContext);
        ASSERT_ARE_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(int, (int)TLSIO_ARDUINO_STATE_CLOSED, (int)tlsio_arduino_get_state(tlsioHandle));

        ///act
        result = tlsioInterfaces->concrete_io_close(tlsioHandle, on_io_close_complete, CallbackContext);

        ///assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_ARE_NOT_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(int, 1, currentmalloc_call);
        ASSERT_CALLBACK_COUNTERS(1, 1, 1, 0, 0);
        ASSERT_ARE_EQUAL(int, (int)TLSIO_ARDUINO_STATE_ERROR, (int)tlsio_arduino_get_state(tlsioHandle));

        ///cleanup
        tlsioInterfaces->concrete_io_destroy(tlsioHandle);
    }

    /* Tests_SRS_TLSIO_ARDUINO_21_048: [ If the tlsio state is TLSIO_ARDUINO_STATE_ERROR, TLSIO_ARDUINO_STATE_OPENING, TLSIO_ARDUINO_STATE_CLOSING, or TLSIO_ARDUINO_STATE_CLOSED, the tlsio_arduino_close shall set the tlsio state as TLSIO_ARDUINO_STATE_ERROR, call the on_io_error, and return _LINE_. ]*/
    TEST_FUNCTION(tlsio_arduino_close__on_TLSIO_ARDUINO_STATE_CLOSED_no_error_callback__failed)
    {
        ///arrange
        CONCRETE_IO_HANDLE tlsioHandle;
        int result;
        const IO_INTERFACE_DESCRIPTION* tlsioInterfaces = tlsio_arduino_get_interface_description();
        ASSERT_IS_NOT_NULL(tlsioInterfaces);

        my_sslClient_new_return = SSLClientHandel;
        my_sslClient_hostByName_return = SSLCLIENT_IP_ADDRESS_SUCCEED;
        my_sslClient_connected_return_list = (const bool*)ConnectedReturnList_ftf;
        my_sslClient_connect_return_list = (const int*)ConnectReturnList_true;

        STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG)).IgnoreArgument(1);
        STRICT_EXPECTED_CALL(sslClient_new());
        STRICT_EXPECTED_CALL(sslClient_setTimeout(SSLClientHandel, 10000));
        STRICT_EXPECTED_CALL(sslClient_hostByName(TEST_CREATE_CONNECTION_HOST_NAME, IGNORED_PTR_ARG)).IgnoreArgument(2);

        tlsioHandle = tlsioInterfaces->concrete_io_create((void*)&tlsioConfig);
        ASSERT_IS_NOT_NULL(tlsioHandle);
        ASSERT_ARE_EQUAL(int, (int)TLSIO_ARDUINO_STATE_CLOSED, (int)tlsio_arduino_get_state(tlsioHandle));

        ///act
        result = tlsioInterfaces->concrete_io_close(tlsioHandle, on_io_close_complete, CallbackContext);

        ///assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_ARE_NOT_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(int, 1, currentmalloc_call);
        ASSERT_CALLBACK_COUNTERS(0, 0, 0, 0, 0);
        ASSERT_ARE_EQUAL(int, (int)TLSIO_ARDUINO_STATE_ERROR, (int)tlsio_arduino_get_state(tlsioHandle));

        ///cleanup
        tlsioInterfaces->concrete_io_destroy(tlsioHandle);
    }

    /* Tests_SRS_TLSIO_ARDUINO_21_044: [ The tlsio_arduino_close shall set the tlsio to try to close the connection for 10 times before assuming that close connection failed. ]*/
    TEST_FUNCTION(tlsio_arduino_close__retry_without_success_and_without_callback__failed)
    {
        ///arrange
        CONCRETE_IO_HANDLE tlsioHandle;
        int result;
        const IO_INTERFACE_DESCRIPTION* tlsioInterfaces = tlsio_arduino_get_interface_description();
        ASSERT_IS_NOT_NULL(tlsioInterfaces);

        my_sslClient_new_return = SSLClientHandel;
        my_sslClient_hostByName_return = SSLCLIENT_IP_ADDRESS_SUCCEED;
        my_sslClient_connected_return_list = (const bool*)ConnectedReturnList_f12t;
        my_sslClient_connect_return_list = (const int*)ConnectReturnList_true;

        STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG)).IgnoreArgument(1);
        STRICT_EXPECTED_CALL(sslClient_new());
        STRICT_EXPECTED_CALL(sslClient_setTimeout(SSLClientHandel, 10000));
        STRICT_EXPECTED_CALL(sslClient_hostByName(TEST_CREATE_CONNECTION_HOST_NAME, IGNORED_PTR_ARG)).IgnoreArgument(2);
        STRICT_EXPECTED_CALL(sslClient_connected(SSLClientHandel));
        STRICT_EXPECTED_CALL(sslClient_connect(SSLClientHandel, SSLCLIENT_IP_ADDRESS, TEST_CREATE_CONNECTION_PORT));
        STRICT_EXPECTED_CALL(sslClient_connected(SSLClientHandel));
        STRICT_EXPECTED_CALL(sslClient_stop(SSLClientHandel));
        for (size_t i = 0; i < 11; i++)
        {
            STRICT_EXPECTED_CALL(sslClient_connected(SSLClientHandel));
        }

        tlsioHandle = tlsioInterfaces->concrete_io_create((void*)&tlsioConfig);
        ASSERT_IS_NOT_NULL(tlsioHandle);

        result = tlsioInterfaces->concrete_io_open(
            tlsioHandle,
            NULL, NULL,
            NULL, NULL,
            NULL, NULL);
        ASSERT_ARE_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(int, (int)TLSIO_ARDUINO_STATE_OPEN, (int)tlsio_arduino_get_state(tlsioHandle));

        result = tlsioInterfaces->concrete_io_close(tlsioHandle, NULL, NULL);
        ASSERT_ARE_EQUAL(int, 0, result);

        ///act
        for (size_t i = 0; i < 10; i++)
        {
            ASSERT_ARE_EQUAL(int, (int)TLSIO_ARDUINO_STATE_CLOSING, (int)tlsio_arduino_get_state(tlsioHandle));
            tlsioInterfaces->concrete_io_dowork(tlsioHandle);
        }

        ///assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_ARE_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(int, 1, currentmalloc_call);
        ASSERT_CALLBACK_COUNTERS(0, 0, 0, 0, 0);
        ASSERT_ARE_EQUAL(int, (int)TLSIO_ARDUINO_STATE_ERROR, (int)tlsio_arduino_get_state(tlsioHandle));

        ///cleanup
        tlsioInterfaces->concrete_io_destroy(tlsioHandle);
    }

    /* Tests_SRS_TLSIO_ARDUINO_21_044: [ The tlsio_arduino_close shall set the tlsio to try to close the connection for 10 times before assuming that close connection failed. ]*/
    /* Tests_SRS_TLSIO_ARDUINO_21_051: [ If the tlsio_arduino_close retry to close more than 10 times without success, it shall call the on_io_error. ]*/
    /* Tests_SRS_TLSIO_ARDUINO_21_067: [ If the tlsio state is TLSIO_ARDUINO_STATE_CLOSING, and ssl client is connected, the tlsio_arduino_dowork shall decrement the counter of trys for closing. ]*/
    /* Tests_SRS_TLSIO_ARDUINO_21_068: [ If the tlsio state is TLSIO_ARDUINO_STATE_CLOSING, ssl client is connected, and the counter to try becomes 0, the tlsio_arduino_dowork shall change the tlsio state to TLSIO_ARDUINO_STATE_ERROR, call on_io_error. ]*/
    TEST_FUNCTION(tlsio_arduino_close__retry_without_success__failed)
    {
        ///arrange
        CONCRETE_IO_HANDLE tlsioHandle;
        int result;
        const IO_INTERFACE_DESCRIPTION* tlsioInterfaces = tlsio_arduino_get_interface_description();
        ASSERT_IS_NOT_NULL(tlsioInterfaces);

        my_sslClient_new_return = SSLClientHandel;
        my_sslClient_hostByName_return = SSLCLIENT_IP_ADDRESS_SUCCEED;
        my_sslClient_connected_return_list = (const bool*)ConnectedReturnList_f12t;
        my_sslClient_connect_return_list = (const int*)ConnectReturnList_true;

        STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG)).IgnoreArgument(1);
        STRICT_EXPECTED_CALL(sslClient_new());
        STRICT_EXPECTED_CALL(sslClient_setTimeout(SSLClientHandel, 10000));
        STRICT_EXPECTED_CALL(sslClient_hostByName(TEST_CREATE_CONNECTION_HOST_NAME, IGNORED_PTR_ARG)).IgnoreArgument(2);
        STRICT_EXPECTED_CALL(sslClient_connected(SSLClientHandel));
        STRICT_EXPECTED_CALL(sslClient_connect(SSLClientHandel, SSLCLIENT_IP_ADDRESS, TEST_CREATE_CONNECTION_PORT));
        STRICT_EXPECTED_CALL(sslClient_connected(SSLClientHandel));
        STRICT_EXPECTED_CALL(sslClient_stop(SSLClientHandel));
        for (size_t i = 0; i < 11; i++)
        {
            STRICT_EXPECTED_CALL(sslClient_connected(SSLClientHandel));
        }

        tlsioHandle = tlsioInterfaces->concrete_io_create((void*)&tlsioConfig);
        ASSERT_IS_NOT_NULL(tlsioHandle);

        result = tlsioInterfaces->concrete_io_open(
            tlsioHandle,
            on_io_open_complete, CallbackContext,
            on_bytes_received, CallbackContext,
            on_io_error, CallbackContext);
        ASSERT_ARE_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(int, (int)TLSIO_ARDUINO_STATE_OPEN, (int)tlsio_arduino_get_state(tlsioHandle));

        result = tlsioInterfaces->concrete_io_close(tlsioHandle, on_io_close_complete, CallbackContext);
        ASSERT_ARE_EQUAL(int, 0, result);

        ///act
        for (size_t i = 0; i < 10; i++)
        {
            ASSERT_ARE_EQUAL(int, (int)TLSIO_ARDUINO_STATE_CLOSING, (int)tlsio_arduino_get_state(tlsioHandle));
            tlsioInterfaces->concrete_io_dowork(tlsioHandle);
        }

        ///assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_ARE_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(int, 1, currentmalloc_call);
        ASSERT_CALLBACK_COUNTERS(1, 1, 0, 0, 0);
        ASSERT_ARE_EQUAL(int, (int)TLSIO_ARDUINO_STATE_ERROR, (int)tlsio_arduino_get_state(tlsioHandle));

        ///cleanup
        tlsioInterfaces->concrete_io_destroy(tlsioHandle);
    }

    /* Tests_SRS_TLSIO_ARDUINO_21_044: [ The tlsio_arduino_close shall set the tlsio to try to close the connection for 10 times before assuming that close connection failed. ]*/
    /* Tests_SRS_TLSIO_ARDUINO_21_066: [ If the tlsio state is TLSIO_ARDUINO_STATE_CLOSING, and ssl client is not connected, the tlsio_arduino_dowork shall change the tlsio state to TLSIO_ARDUINO_STATE_CLOSE, and call the on_io_close_complete. ]*/
    TEST_FUNCTION(tlsio_arduino_close__retry_10_times__succeed)
    {
        ///arrange
        CONCRETE_IO_HANDLE tlsioHandle;
        int result;
        const IO_INTERFACE_DESCRIPTION* tlsioInterfaces = tlsio_arduino_get_interface_description();
        ASSERT_IS_NOT_NULL(tlsioInterfaces);

        my_sslClient_new_return = SSLClientHandel;
        my_sslClient_hostByName_return = SSLCLIENT_IP_ADDRESS_SUCCEED;
        my_sslClient_connected_return_list = (const bool*)ConnectedReturnList_f11tf;
        my_sslClient_connect_return_list = (const int*)ConnectReturnList_true;

        STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG)).IgnoreArgument(1);
        STRICT_EXPECTED_CALL(sslClient_new());
        STRICT_EXPECTED_CALL(sslClient_setTimeout(SSLClientHandel, 10000));
        STRICT_EXPECTED_CALL(sslClient_hostByName(TEST_CREATE_CONNECTION_HOST_NAME, IGNORED_PTR_ARG)).IgnoreArgument(2);
        STRICT_EXPECTED_CALL(sslClient_connected(SSLClientHandel));
        STRICT_EXPECTED_CALL(sslClient_connect(SSLClientHandel, SSLCLIENT_IP_ADDRESS, TEST_CREATE_CONNECTION_PORT));
        STRICT_EXPECTED_CALL(sslClient_connected(SSLClientHandel));
        STRICT_EXPECTED_CALL(sslClient_stop(SSLClientHandel));
        for (size_t i = 0; i < 11; i++)
        {
            STRICT_EXPECTED_CALL(sslClient_connected(SSLClientHandel));
        }

        tlsioHandle = tlsioInterfaces->concrete_io_create((void*)&tlsioConfig);
        ASSERT_IS_NOT_NULL(tlsioHandle);

        result = tlsioInterfaces->concrete_io_open(
            tlsioHandle,
            on_io_open_complete, CallbackContext,
            on_bytes_received, CallbackContext,
            on_io_error, CallbackContext);
        ASSERT_ARE_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(int, (int)TLSIO_ARDUINO_STATE_OPEN, (int)tlsio_arduino_get_state(tlsioHandle));

        result = tlsioInterfaces->concrete_io_close(tlsioHandle, on_io_close_complete, CallbackContext);
        ASSERT_ARE_EQUAL(int, 0, result);

        ///act
        for (size_t i = 0; i < 10; i++)
        {
            ASSERT_ARE_EQUAL(int, (int)TLSIO_ARDUINO_STATE_CLOSING, (int)tlsio_arduino_get_state(tlsioHandle));
            tlsioInterfaces->concrete_io_dowork(tlsioHandle);
        }

        ///assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_ARE_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(int, 1, currentmalloc_call);
        ASSERT_CALLBACK_COUNTERS(0, 1, 1, 0, 0);
        ASSERT_ARE_EQUAL(void_ptr, CallbackContext, on_io_close_complete_context);
        ASSERT_ARE_EQUAL(int, (int)TLSIO_ARDUINO_STATE_CLOSED, (int)tlsio_arduino_get_state(tlsioHandle));

        ///cleanup
        tlsioInterfaces->concrete_io_destroy(tlsioHandle);
    }

    /* Tests_SRS_TLSIO_ARDUINO_21_043: [ The tlsio_arduino_close shall start the process to close the ssl connection. ]*/
    /* Tests_SRS_TLSIO_ARDUINO_21_045: [ The tlsio_arduino_close shall store the provided on_io_close_complete callback function address. ]*/
    /* Tests_SRS_TLSIO_ARDUINO_21_046: [ The tlsio_arduino_close shall store the provided on_io_close_complete_context handle. ]*/
    /* Tests_SRS_TLSIO_ARDUINO_21_006: [ The tlsio_arduino shall return the status of all async operations using the callbacks. ]*/
    /* Tests_SRS_TLSIO_ARDUINO_21_007: [ If the callback function is set as NULL. The tlsio_arduino shall not call anything. ]*/
    TEST_FUNCTION(tlsio_arduino_close__without_callback__succeed)
    {
        ///arrange
        CONCRETE_IO_HANDLE tlsioHandle;
        int result;
        const IO_INTERFACE_DESCRIPTION* tlsioInterfaces = tlsio_arduino_get_interface_description();
        ASSERT_IS_NOT_NULL(tlsioInterfaces);

        my_sslClient_new_return = SSLClientHandel;
        my_sslClient_hostByName_return = SSLCLIENT_IP_ADDRESS_SUCCEED;
        my_sslClient_connected_return_list = (const bool*)ConnectedReturnList_ftf;
        my_sslClient_connect_return_list = (const int*)ConnectReturnList_true;

        STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG)).IgnoreArgument(1);
        STRICT_EXPECTED_CALL(sslClient_new());
        STRICT_EXPECTED_CALL(sslClient_setTimeout(SSLClientHandel, 10000));
        STRICT_EXPECTED_CALL(sslClient_hostByName(TEST_CREATE_CONNECTION_HOST_NAME, IGNORED_PTR_ARG)).IgnoreArgument(2);
        STRICT_EXPECTED_CALL(sslClient_connected(SSLClientHandel));
        STRICT_EXPECTED_CALL(sslClient_connect(SSLClientHandel, SSLCLIENT_IP_ADDRESS, TEST_CREATE_CONNECTION_PORT));
        STRICT_EXPECTED_CALL(sslClient_connected(SSLClientHandel));
        STRICT_EXPECTED_CALL(sslClient_stop(SSLClientHandel));
        STRICT_EXPECTED_CALL(sslClient_connected(SSLClientHandel));

        tlsioHandle = tlsioInterfaces->concrete_io_create((void*)&tlsioConfig);
        ASSERT_IS_NOT_NULL(tlsioHandle);

        result = tlsioInterfaces->concrete_io_open(
            tlsioHandle,
            NULL, NULL,
            NULL, NULL,
            NULL, NULL);
        ASSERT_ARE_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(int, (int)TLSIO_ARDUINO_STATE_OPEN, (int)tlsio_arduino_get_state(tlsioHandle));

        ///act
        result = tlsioInterfaces->concrete_io_close(tlsioHandle, NULL, NULL);

        ///assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_ARE_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(int, 1, currentmalloc_call);
        ASSERT_CALLBACK_COUNTERS(0, 0, 0, 0, 0);
        ASSERT_ARE_EQUAL(int, (int)TLSIO_ARDUINO_STATE_CLOSED, (int)tlsio_arduino_get_state(tlsioHandle));

        ///cleanup
        tlsioInterfaces->concrete_io_destroy(tlsioHandle);
    }

    /* Tests_SRS_TLSIO_ARDUINO_21_043: [ The tlsio_arduino_close shall start the process to close the ssl connection. ]*/
    /* Tests_SRS_TLSIO_ARDUINO_21_045: [ The tlsio_arduino_close shall store the provided on_io_close_complete callback function address. ]*/
    /* Tests_SRS_TLSIO_ARDUINO_21_046: [ The tlsio_arduino_close shall store the provided on_io_close_complete_context handle. ]*/
    /* Tests_SRS_TLSIO_ARDUINO_21_004: [ The tlsio_arduino shall call the callbacks functions defined in the `xio.h`. ]*/
    /* Tests_SRS_TLSIO_ARDUINO_21_050: [ If the tlsio_arduino_close get success closing the tls connection, it shall call the tlsio_arduino_dowork. ]*/
    TEST_FUNCTION(tlsio_arduino_close__succeed)
    {
        ///arrange
        CONCRETE_IO_HANDLE tlsioHandle;
        int result;
        const IO_INTERFACE_DESCRIPTION* tlsioInterfaces = tlsio_arduino_get_interface_description();
        ASSERT_IS_NOT_NULL(tlsioInterfaces);

        my_sslClient_new_return = SSLClientHandel;
        my_sslClient_hostByName_return = SSLCLIENT_IP_ADDRESS_SUCCEED;
        my_sslClient_connected_return_list = (const bool*)ConnectedReturnList_ftf;
        my_sslClient_connect_return_list = (const int*)ConnectReturnList_true;

        STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG)).IgnoreArgument(1);
        STRICT_EXPECTED_CALL(sslClient_new());
        STRICT_EXPECTED_CALL(sslClient_setTimeout(SSLClientHandel, 10000));
        STRICT_EXPECTED_CALL(sslClient_hostByName(TEST_CREATE_CONNECTION_HOST_NAME, IGNORED_PTR_ARG)).IgnoreArgument(2);
        STRICT_EXPECTED_CALL(sslClient_connected(SSLClientHandel));
        STRICT_EXPECTED_CALL(sslClient_connect(SSLClientHandel, SSLCLIENT_IP_ADDRESS, TEST_CREATE_CONNECTION_PORT));
        STRICT_EXPECTED_CALL(sslClient_connected(SSLClientHandel));
        STRICT_EXPECTED_CALL(sslClient_stop(SSLClientHandel));
        STRICT_EXPECTED_CALL(sslClient_connected(SSLClientHandel));

        tlsioHandle = tlsioInterfaces->concrete_io_create((void*)&tlsioConfig);
        ASSERT_IS_NOT_NULL(tlsioHandle);

        result = tlsioInterfaces->concrete_io_open(
            tlsioHandle,
            on_io_open_complete, CallbackContext,
            on_bytes_received, CallbackContext,
            on_io_error, CallbackContext);
        ASSERT_ARE_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(int, (int)TLSIO_ARDUINO_STATE_OPEN, (int)tlsio_arduino_get_state(tlsioHandle));

        ///act
        result = tlsioInterfaces->concrete_io_close(tlsioHandle, on_io_close_complete, CallbackContext);

        ///assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_ARE_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(int, 1, currentmalloc_call);
        ASSERT_CALLBACK_COUNTERS(0, 1, 1, 0, 0);
        ASSERT_ARE_EQUAL(void_ptr, CallbackContext, on_io_close_complete_context);
        ASSERT_ARE_EQUAL(int, (int)TLSIO_ARDUINO_STATE_CLOSED, (int)tlsio_arduino_get_state(tlsioHandle));

        ///cleanup
        tlsioInterfaces->concrete_io_destroy(tlsioHandle);
    }

    /* Tests_SRS_TLSIO_ARDUINO_21_043: [ The tlsio_arduino_close shall start the process to close the ssl connection. ]*/
    /* Tests_SRS_TLSIO_ARDUINO_21_047: [ If tlsio_arduino_close get success to start the process to close the ssl connection, it shall set the tlsio state as TLSIO_ARDUINO_STATE_CLOSING, and return 0. ]*/
    TEST_FUNCTION(tlsio_arduino_close__stopping_process_on_TLSIO_ARDUINO_STATE_CLOSING__succeed)
    {
        ///arrange
        CONCRETE_IO_HANDLE tlsioHandle;
        int result;
        const IO_INTERFACE_DESCRIPTION* tlsioInterfaces = tlsio_arduino_get_interface_description();
        ASSERT_IS_NOT_NULL(tlsioInterfaces);

        my_sslClient_new_return = SSLClientHandel;
        my_sslClient_hostByName_return = SSLCLIENT_IP_ADDRESS_SUCCEED;
        my_sslClient_connected_return_list = (const bool*)ConnectedReturnList_ftt;
        my_sslClient_connect_return_list = (const int*)ConnectReturnList_true;

        STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG)).IgnoreArgument(1);
        STRICT_EXPECTED_CALL(sslClient_new());
        STRICT_EXPECTED_CALL(sslClient_setTimeout(SSLClientHandel, 10000));
        STRICT_EXPECTED_CALL(sslClient_hostByName(TEST_CREATE_CONNECTION_HOST_NAME, IGNORED_PTR_ARG)).IgnoreArgument(2);
        STRICT_EXPECTED_CALL(sslClient_connected(SSLClientHandel));
        STRICT_EXPECTED_CALL(sslClient_connect(SSLClientHandel, SSLCLIENT_IP_ADDRESS, TEST_CREATE_CONNECTION_PORT));
        STRICT_EXPECTED_CALL(sslClient_connected(SSLClientHandel));
        STRICT_EXPECTED_CALL(sslClient_stop(SSLClientHandel));
        STRICT_EXPECTED_CALL(sslClient_connected(SSLClientHandel));

        tlsioHandle = tlsioInterfaces->concrete_io_create((void*)&tlsioConfig);
        ASSERT_IS_NOT_NULL(tlsioHandle);

        result = tlsioInterfaces->concrete_io_open(
            tlsioHandle,
            on_io_open_complete, CallbackContext,
            on_bytes_received, CallbackContext,
            on_io_error, CallbackContext);
        ASSERT_ARE_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(int, (int)TLSIO_ARDUINO_STATE_OPEN, (int)tlsio_arduino_get_state(tlsioHandle));

        ///act
        result = tlsioInterfaces->concrete_io_close(tlsioHandle, on_io_close_complete, CallbackContext);

        ///assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_ARE_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(int, 1, currentmalloc_call);
        ASSERT_CALLBACK_COUNTERS(0, 1, 0, 0, 0);
        ASSERT_ARE_EQUAL(int, (int)TLSIO_ARDUINO_STATE_CLOSING, (int)tlsio_arduino_get_state(tlsioHandle));

        ///cleanup
        tlsioInterfaces->concrete_io_destroy(tlsioHandle);
    }

    /* Tests_SRS_TLSIO_ARDUINO_21_038: [ If tlsio_arduino_open failed to start the process to open the ssl connection, it shall set the tlsio state as TLSIO_ARDUINO_STATE_ERROR, and return _LINE_. ]*/
    TEST_FUNCTION(tlsio_arduino_open__SSL_connect_without_success__failed)
    {
        ///arrange
        CONCRETE_IO_HANDLE tlsioHandle;
        int result;
        const IO_INTERFACE_DESCRIPTION* tlsioInterfaces = tlsio_arduino_get_interface_description();
        ASSERT_IS_NOT_NULL(tlsioInterfaces);

        my_sslClient_new_return = SSLClientHandel;
        my_sslClient_hostByName_return = SSLCLIENT_IP_ADDRESS_SUCCEED;
        my_sslClient_connected_return_list = (const bool*)ConnectedReturnList_f;
        my_sslClient_connect_return_list = (const int*)ConnectReturnList_false;

        STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG)).IgnoreArgument(1);
        STRICT_EXPECTED_CALL(sslClient_new());
        STRICT_EXPECTED_CALL(sslClient_setTimeout(SSLClientHandel, 10000));
        STRICT_EXPECTED_CALL(sslClient_hostByName(TEST_CREATE_CONNECTION_HOST_NAME, IGNORED_PTR_ARG)).IgnoreArgument(2);
        STRICT_EXPECTED_CALL(sslClient_connected(SSLClientHandel));
        STRICT_EXPECTED_CALL(sslClient_connect(SSLClientHandel, SSLCLIENT_IP_ADDRESS, TEST_CREATE_CONNECTION_PORT));

        tlsioHandle = tlsioInterfaces->concrete_io_create((void*)&tlsioConfig);
        ASSERT_IS_NOT_NULL(tlsioHandle);

        ///act
        result = tlsioInterfaces->concrete_io_open(
            tlsioHandle,
            on_io_open_complete, CallbackContext,
            on_bytes_received, CallbackContext,
            on_io_error, CallbackContext);

        ///assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_ARE_NOT_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(int, 1, currentmalloc_call);
        ASSERT_CALLBACK_COUNTERS(1, 1, 0, 0, 0);
        ASSERT_ARE_EQUAL(int, (int)IO_OPEN_ERROR, (int)on_io_open_complete_result);
        ASSERT_ARE_EQUAL(int, (int)TLSIO_ARDUINO_STATE_ERROR, (int)tlsio_arduino_get_state(tlsioHandle));

        ///cleanup
        tlsioInterfaces->concrete_io_destroy(tlsioHandle);
    }

    /* Tests_SRS_TLSIO_ARDUINO_21_037: [ If the ssl client is connected, the tlsio_arduino_open shall change the state to TLSIO_ARDUINO_STATE_ERROR, log the error, and return _LINE_. ]*/
    TEST_FUNCTION(tlsio_arduino_open_with_SSL_connected__failed)
    {
        ///arrange
        CONCRETE_IO_HANDLE tlsioHandle;
        int result;
        const IO_INTERFACE_DESCRIPTION* tlsioInterfaces = tlsio_arduino_get_interface_description();
        ASSERT_IS_NOT_NULL(tlsioInterfaces);

        my_sslClient_new_return = SSLClientHandel;
        my_sslClient_hostByName_return = SSLCLIENT_IP_ADDRESS_SUCCEED;
        my_sslClient_connected_return_list = (const bool*)ConnectedReturnList_t;
        my_sslClient_connect_return_list = (const int*)ConnectReturnList_true;

        STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG)).IgnoreArgument(1);
        STRICT_EXPECTED_CALL(sslClient_new());
        STRICT_EXPECTED_CALL(sslClient_setTimeout(SSLClientHandel, 10000));
        STRICT_EXPECTED_CALL(sslClient_hostByName(TEST_CREATE_CONNECTION_HOST_NAME, IGNORED_PTR_ARG)).IgnoreArgument(2);
        STRICT_EXPECTED_CALL(sslClient_connected(SSLClientHandel));

        tlsioHandle = tlsioInterfaces->concrete_io_create((void*)&tlsioConfig);
        ASSERT_IS_NOT_NULL(tlsioHandle);

        ///act
        result = tlsioInterfaces->concrete_io_open(
            tlsioHandle,
            on_io_open_complete, CallbackContext,
            on_bytes_received, CallbackContext,
            on_io_error, CallbackContext);

        ///assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_ARE_NOT_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(int, 1, currentmalloc_call);
        ASSERT_CALLBACK_COUNTERS(1, 1, 0, 0, 0);
        ASSERT_ARE_EQUAL(int, (int)IO_OPEN_ERROR, (int)on_io_open_complete_result);
        ASSERT_ARE_EQUAL(int, (int)TLSIO_ARDUINO_STATE_ERROR, (int)tlsio_arduino_get_state(tlsioHandle));

        ///cleanup
        tlsioInterfaces->concrete_io_destroy(tlsioHandle);
    }

    /* Tests_SRS_TLSIO_ARDUINO_21_036: [ If the tlsio_handle is NULL, the tlsio_arduino_open shall not do anything, and return _LINE_. ]*/
    TEST_FUNCTION(tlsio_arduino_open__NULL_tlsio_handle__failed)
    {
        ///arrange
        int result;

        const IO_INTERFACE_DESCRIPTION* tlsioInterfaces = tlsio_arduino_get_interface_description();
        ASSERT_IS_NOT_NULL(tlsioInterfaces);

        ///act
        result = tlsioInterfaces->concrete_io_open(
            NULL,
            on_io_open_complete, CallbackContext,
            on_bytes_received, CallbackContext,
            on_io_error, CallbackContext);

        ///assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_ARE_NOT_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(int, 0, currentmalloc_call);
        ASSERT_CALLBACK_COUNTERS(1, 1, 0, 0, 0);
        ASSERT_ARE_EQUAL(int, (int)IO_OPEN_ERROR, (int)on_io_open_complete_result);

        ///cleanup
    }

    /* Tests_SRS_TLSIO_ARDUINO_21_035: [ If the tlsio state is TLSIO_ARDUINO_STATE_ERROR, TLSIO_ARDUINO_STATE_OPENING, TLSIO_ARDUINO_STATE_OPEN, or TLSIO_ARDUINO_STATE_CLOSING, the tlsio_arduino_open shall set the tlsio state as TLSIO_ARDUINO_STATE_ERROR, and return _LINE_. ]*/
    /* Tests_SRS_TLSIO_ARDUINO_21_039: [ If the tlsio_arduino_open failed to open the tls connection, and the on_io_open_complete callback was provided, it shall call the on_io_open_complete with IO_OPEN_ERROR. ]*/
    /* Tests_SRS_TLSIO_ARDUINO_21_040: [ If the tlsio_arduino_open failed to open the tls connection, and the on_io_error callback was provided, it shall call the on_io_error. ]*/
    TEST_FUNCTION(tlsio_arduino_open_on_TLSIO_ARDUINO_STATE_ERROR__failed)
    {
        ///arrange
        CONCRETE_IO_HANDLE tlsioHandle;
        int result;
        const IO_INTERFACE_DESCRIPTION* tlsioInterfaces = tlsio_arduino_get_interface_description();
        ASSERT_IS_NOT_NULL(tlsioInterfaces);

        my_sslClient_new_return = SSLClientHandel;
        my_sslClient_hostByName_return = SSLCLIENT_IP_ADDRESS_SUCCEED;
        my_sslClient_connected_return_list = (const bool*)ConnectedReturnList_ft;
        my_sslClient_connect_return_list = (const int*)ConnectReturnList_true;

        STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG)).IgnoreArgument(1);
        STRICT_EXPECTED_CALL(sslClient_new());
        STRICT_EXPECTED_CALL(sslClient_setTimeout(SSLClientHandel, 10000));
        STRICT_EXPECTED_CALL(sslClient_hostByName(TEST_CREATE_CONNECTION_HOST_NAME, IGNORED_PTR_ARG)).IgnoreArgument(2);
        STRICT_EXPECTED_CALL(sslClient_connected(SSLClientHandel));
        STRICT_EXPECTED_CALL(sslClient_connect(SSLClientHandel, SSLCLIENT_IP_ADDRESS, TEST_CREATE_CONNECTION_PORT));
        STRICT_EXPECTED_CALL(sslClient_connected(SSLClientHandel));

        tlsioHandle = tlsioInterfaces->concrete_io_create((void*)&tlsioConfig);
        ASSERT_IS_NOT_NULL(tlsioHandle);
        result = tlsioInterfaces->concrete_io_open(
            tlsioHandle,
            on_io_open_complete, CallbackContext,
            on_bytes_received, CallbackContext,
            on_io_error, CallbackContext);
        result = tlsioInterfaces->concrete_io_open(
            tlsioHandle,
            on_io_open_complete, CallbackContext,
            on_bytes_received, CallbackContext,
            on_io_error, CallbackContext);
        ASSERT_ARE_NOT_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(int, (int)TLSIO_ARDUINO_STATE_ERROR, (int)tlsio_arduino_get_state(tlsioHandle));

        ///act
        result = tlsioInterfaces->concrete_io_open(
            tlsioHandle,
            on_io_open_complete, CallbackContext,
            on_bytes_received, CallbackContext,
            on_io_error, CallbackContext);

        ///assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_ARE_NOT_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(int, 1, currentmalloc_call);
        ASSERT_CALLBACK_COUNTERS(2, 3, 0, 0, 0);
        ASSERT_ARE_EQUAL(int, (int)IO_OPEN_ERROR, (int)on_io_open_complete_result);
        ASSERT_ARE_EQUAL(int, (int)TLSIO_ARDUINO_STATE_ERROR, (int)tlsio_arduino_get_state(tlsioHandle));

        ///cleanup
        tlsioInterfaces->concrete_io_destroy(tlsioHandle);
    }

    /* Tests_SRS_TLSIO_ARDUINO_21_035: [ If the tlsio state is TLSIO_ARDUINO_STATE_ERROR, TLSIO_ARDUINO_STATE_OPENING, TLSIO_ARDUINO_STATE_OPEN, or TLSIO_ARDUINO_STATE_CLOSING, the tlsio_arduino_open shall set the tlsio state as TLSIO_ARDUINO_STATE_ERROR, and return _LINE_. ]*/
    /* Tests_SRS_TLSIO_ARDUINO_21_039: [ If the tlsio_arduino_open failed to open the tls connection, and the on_io_open_complete callback was provided, it shall call the on_io_open_complete with IO_OPEN_ERROR. ]*/
    /* Tests_SRS_TLSIO_ARDUINO_21_040: [ If the tlsio_arduino_open failed to open the tls connection, and the on_io_error callback was provided, it shall call the on_io_error. ]*/
    TEST_FUNCTION(tlsio_arduino_open_on_TLSIO_ARDUINO_STATE_OPEN__failed)
    {
        ///arrange
        CONCRETE_IO_HANDLE tlsioHandle;
        int result;
        const IO_INTERFACE_DESCRIPTION* tlsioInterfaces = tlsio_arduino_get_interface_description();
        ASSERT_IS_NOT_NULL(tlsioInterfaces);

        my_sslClient_new_return = SSLClientHandel;
        my_sslClient_hostByName_return = SSLCLIENT_IP_ADDRESS_SUCCEED;
        my_sslClient_connected_return_list = (const bool*)ConnectedReturnList_ft;
        my_sslClient_connect_return_list = (const int*)ConnectReturnList_true;

        STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG)).IgnoreArgument(1);
        STRICT_EXPECTED_CALL(sslClient_new());
        STRICT_EXPECTED_CALL(sslClient_setTimeout(SSLClientHandel, 10000));
        STRICT_EXPECTED_CALL(sslClient_hostByName(TEST_CREATE_CONNECTION_HOST_NAME, IGNORED_PTR_ARG)).IgnoreArgument(2);
        STRICT_EXPECTED_CALL(sslClient_connected(SSLClientHandel));
        STRICT_EXPECTED_CALL(sslClient_connect(SSLClientHandel, SSLCLIENT_IP_ADDRESS, TEST_CREATE_CONNECTION_PORT));
        STRICT_EXPECTED_CALL(sslClient_connected(SSLClientHandel));

        tlsioHandle = tlsioInterfaces->concrete_io_create((void*)&tlsioConfig);
        ASSERT_IS_NOT_NULL(tlsioHandle);
        result = tlsioInterfaces->concrete_io_open(
            tlsioHandle,
            on_io_open_complete, CallbackContext,
            on_bytes_received, CallbackContext,
            on_io_error, CallbackContext);
        ASSERT_ARE_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(int, (int)TLSIO_ARDUINO_STATE_OPEN, (int)tlsio_arduino_get_state(tlsioHandle));

        ///act
        result = tlsioInterfaces->concrete_io_open(
            tlsioHandle,
            on_io_open_complete, CallbackContext,
            on_bytes_received, CallbackContext,
            on_io_error, CallbackContext);

        ///assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_ARE_NOT_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(int, 1, currentmalloc_call);
        ASSERT_CALLBACK_COUNTERS(1, 2, 0, 0, 0);
        ASSERT_ARE_EQUAL(int, (int)IO_OPEN_ERROR, (int)on_io_open_complete_result);
        ASSERT_ARE_EQUAL(int, (int)TLSIO_ARDUINO_STATE_ERROR, (int)tlsio_arduino_get_state(tlsioHandle));

        ///cleanup
        tlsioInterfaces->concrete_io_destroy(tlsioHandle);
    }

    /* Tests_SRS_TLSIO_ARDUINO_21_002: [ The tlsio_arduino shall report the open operation status using the IO_OPEN_RESULT enumerator defined in the `xio.h`.]*/
    /* Tests_SRS_TLSIO_ARDUINO_21_035: [ If the tlsio state is TLSIO_ARDUINO_STATE_ERROR, TLSIO_ARDUINO_STATE_OPENING, TLSIO_ARDUINO_STATE_OPEN, or TLSIO_ARDUINO_STATE_CLOSING, the tlsio_arduino_open shall set the tlsio state as TLSIO_ARDUINO_STATE_ERROR, and return _LINE_. ]*/
    /* Tests_SRS_TLSIO_ARDUINO_21_039: [ If the tlsio_arduino_open failed to open the tls connection, and the on_io_open_complete callback was provided, it shall call the on_io_open_complete with IO_OPEN_ERROR. ]*/
    /* Tests_SRS_TLSIO_ARDUINO_21_040: [ If the tlsio_arduino_open failed to open the tls connection, and the on_io_error callback was provided, it shall call the on_io_error. ]*/
    TEST_FUNCTION(tlsio_arduino_open_on_TLSIO_ARDUINO_STATE_OPENING__failed)
    {
        ///arrange
        CONCRETE_IO_HANDLE tlsioHandle;
        int result;
        const IO_INTERFACE_DESCRIPTION* tlsioInterfaces = tlsio_arduino_get_interface_description();
        ASSERT_IS_NOT_NULL(tlsioInterfaces);

        my_sslClient_new_return = SSLClientHandel;
        my_sslClient_hostByName_return = SSLCLIENT_IP_ADDRESS_SUCCEED;
        my_sslClient_connected_return_list = (const bool*)ConnectedReturnList_ff;
        my_sslClient_connect_return_list = (const int*)ConnectReturnList_true;

        STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG)).IgnoreArgument(1);
        STRICT_EXPECTED_CALL(sslClient_new());
        STRICT_EXPECTED_CALL(sslClient_setTimeout(SSLClientHandel, 10000));
        STRICT_EXPECTED_CALL(sslClient_hostByName(TEST_CREATE_CONNECTION_HOST_NAME, IGNORED_PTR_ARG)).IgnoreArgument(2);
        STRICT_EXPECTED_CALL(sslClient_connected(SSLClientHandel));
        STRICT_EXPECTED_CALL(sslClient_connect(SSLClientHandel, SSLCLIENT_IP_ADDRESS, TEST_CREATE_CONNECTION_PORT));
        STRICT_EXPECTED_CALL(sslClient_connected(SSLClientHandel));

        tlsioHandle = tlsioInterfaces->concrete_io_create((void*)&tlsioConfig);
        ASSERT_IS_NOT_NULL(tlsioHandle);
        result = tlsioInterfaces->concrete_io_open(
            tlsioHandle,
            on_io_open_complete, CallbackContext,
            on_bytes_received, CallbackContext,
            on_io_error, CallbackContext);
        ASSERT_ARE_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(int, (int)TLSIO_ARDUINO_STATE_OPENING, (int)tlsio_arduino_get_state(tlsioHandle));

        ///act
        result = tlsioInterfaces->concrete_io_open(
            tlsioHandle,
            on_io_open_complete, CallbackContext,
            on_bytes_received, CallbackContext,
            on_io_error, CallbackContext);

        ///assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_ARE_NOT_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(int, 1, currentmalloc_call);
        ASSERT_CALLBACK_COUNTERS(1, 1, 0, 0, 0);
        ASSERT_ARE_EQUAL(int, (int)IO_OPEN_ERROR, (int)on_io_open_complete_result);
        ASSERT_ARE_EQUAL(int, (int)TLSIO_ARDUINO_STATE_ERROR, (int)tlsio_arduino_get_state(tlsioHandle));

        ///cleanup
        tlsioInterfaces->concrete_io_destroy(tlsioHandle);
    }

    /* Tests_SRS_TLSIO_ARDUINO_21_032: [ The tlsio_arduino_open shall store the provided on_io_error callback function address. ]*/
    /* Tests_SRS_TLSIO_ARDUINO_21_033: [ The tlsio_arduino_open shall store the provided on_io_error_context handle. ]*/
    /* Tests_SRS_TLSIO_ARDUINO_21_004: [ The tlsio_arduino shall call the callbacks functions defined in the `xio.h`. ]*/
    /* Tests_SRS_TLSIO_ARDUINO_21_006: [ The tlsio_arduino shall return the status of all async operations using the callbacks. ]*/
    /* Tests_SRS_TLSIO_ARDUINO_21_007: [ If the callback function is set as NULL. The tlsio_arduino shall not call anything. ]*/
    TEST_FUNCTION(tlsio_arduino_open__retry_without_success_without_callback__failed)
    {
        ///arrange
        CONCRETE_IO_HANDLE tlsioHandle;
        int result;
        const IO_INTERFACE_DESCRIPTION* tlsioInterfaces = tlsio_arduino_get_interface_description();
        ASSERT_IS_NOT_NULL(tlsioInterfaces);

        my_sslClient_new_return = SSLClientHandel;
        my_sslClient_hostByName_return = SSLCLIENT_IP_ADDRESS_SUCCEED;
        my_sslClient_connected_return_list = (const bool*)ConnectedReturnList_12f;
        my_sslClient_connect_return_list = (const int*)ConnectReturnList_true;

        STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG)).IgnoreArgument(1);
        STRICT_EXPECTED_CALL(sslClient_new());
        STRICT_EXPECTED_CALL(sslClient_setTimeout(SSLClientHandel, 10000));
        STRICT_EXPECTED_CALL(sslClient_hostByName(TEST_CREATE_CONNECTION_HOST_NAME, IGNORED_PTR_ARG)).IgnoreArgument(2);
        STRICT_EXPECTED_CALL(sslClient_connected(SSLClientHandel));
        STRICT_EXPECTED_CALL(sslClient_connect(SSLClientHandel, SSLCLIENT_IP_ADDRESS, TEST_CREATE_CONNECTION_PORT));
        for (size_t i = 0; i < 11; i++)
        {
            STRICT_EXPECTED_CALL(sslClient_connected(SSLClientHandel));
        }

        tlsioHandle = tlsioInterfaces->concrete_io_create((void*)&tlsioConfig);
        ASSERT_IS_NOT_NULL(tlsioHandle);

        result = tlsioInterfaces->concrete_io_open(
            tlsioHandle,
            NULL, NULL,
            NULL, NULL,
            NULL, NULL);
        ASSERT_ARE_EQUAL(int, 0, result);

        ///act
        for (size_t i = 0; i < 10; i++)
        {
            char temp[100];
            sprintf(temp, "on failed call %zu", i);
            TLSIO_ARDUINO_STATE state = tlsio_arduino_get_state(tlsioHandle);
            ASSERT_ARE_EQUAL_WITH_MSG(TLSIO_ARDUINO_STATE, TLSIO_ARDUINO_STATE_OPENING, state, temp);
            tlsioInterfaces->concrete_io_dowork(tlsioHandle);
        }

        ///assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_ARE_EQUAL(int, 1, currentmalloc_call);
        ASSERT_CALLBACK_COUNTERS(0, 0, 0, 0, 0);
        TLSIO_ARDUINO_STATE state = tlsio_arduino_get_state(tlsioHandle);
        ASSERT_ARE_EQUAL(TLSIO_ARDUINO_STATE, TLSIO_ARDUINO_STATE_ERROR, state);

        ///cleanup
        tlsioInterfaces->concrete_io_destroy(tlsioHandle);
    }

    /* Tests_SRS_TLSIO_ARDUINO_21_002: [ The tlsio_arduino shall report the open operation status using the IO_OPEN_RESULT enumerator defined in the `xio.h`.]*/
    /* Tests_SRS_TLSIO_ARDUINO_21_027: [ The tlsio_arduino_open shall set the tlsio to try to open the connection for 10 times before assuming that connection failed. ]*/
    /* Tests_SRS_TLSIO_ARDUINO_21_032: [ The tlsio_arduino_open shall store the provided on_io_error callback function address. ]*/
    /* Tests_SRS_TLSIO_ARDUINO_21_033: [ The tlsio_arduino_open shall store the provided on_io_error_context handle. ]*/
    /* Tests_SRS_TLSIO_ARDUINO_21_040: [ If the tlsio_arduino_open failed to open the tls connection, and the on_io_error callback was provided, it shall call the on_io_error. ]*/
    /* Tests_SRS_TLSIO_ARDUINO_21_042: [ If the tlsio_arduino_open retry to open more than 10 times without success, it shall call the on_io_open_complete with IO_OPEN_CANCELED. ]*/
    /* Tests_SRS_TLSIO_ARDUINO_21_064: [ If the tlsio state is TLSIO_ARDUINO_STATE_OPENING, and ssl client is not connected, the tlsio_arduino_dowork shall decrement the counter of trys for opening. ]*/
    /* Tests_SRS_TLSIO_ARDUINO_21_065: [ If the tlsio state is TLSIO_ARDUINO_STATE_OPENING, ssl client is not connected, and the counter to try becomes 0, the tlsio_arduino_dowork shall change the tlsio state to TLSIO_ARDUINO_STATE_ERROR, call on_io_open_complete with IO_OPEN_CANCELLED, call on_io_error. ]*/
    TEST_FUNCTION(tlsio_arduino_open__retry_without_success__failed)
    {
        ///arrange
        CONCRETE_IO_HANDLE tlsioHandle;
        int result;
        const IO_INTERFACE_DESCRIPTION* tlsioInterfaces = tlsio_arduino_get_interface_description();
        ASSERT_IS_NOT_NULL(tlsioInterfaces);

        my_sslClient_new_return = SSLClientHandel;
        my_sslClient_hostByName_return = SSLCLIENT_IP_ADDRESS_SUCCEED;
        my_sslClient_connected_return_list = (const bool*)ConnectedReturnList_12f;
        my_sslClient_connect_return_list = (const int*)ConnectReturnList_true;

        STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG)).IgnoreArgument(1);
        STRICT_EXPECTED_CALL(sslClient_new());
        STRICT_EXPECTED_CALL(sslClient_setTimeout(SSLClientHandel, 10000));
        STRICT_EXPECTED_CALL(sslClient_hostByName(TEST_CREATE_CONNECTION_HOST_NAME, IGNORED_PTR_ARG)).IgnoreArgument(2);
        STRICT_EXPECTED_CALL(sslClient_connected(SSLClientHandel));
        STRICT_EXPECTED_CALL(sslClient_connect(SSLClientHandel, SSLCLIENT_IP_ADDRESS, TEST_CREATE_CONNECTION_PORT));
        for (size_t i = 0; i < 11; i++)
        {
            STRICT_EXPECTED_CALL(sslClient_connected(SSLClientHandel));
        }

        tlsioHandle = tlsioInterfaces->concrete_io_create((void*)&tlsioConfig);
        ASSERT_IS_NOT_NULL(tlsioHandle);

        result = tlsioInterfaces->concrete_io_open(
            tlsioHandle,
            on_io_open_complete, CallbackContext,
            on_bytes_received, CallbackContext,
            on_io_error, CallbackContext);
        ASSERT_ARE_EQUAL(int, 0, result);

        ///act
        for (size_t i = 0; i < 10; i++)
        {
            ASSERT_ARE_EQUAL(int, (int)TLSIO_ARDUINO_STATE_OPENING, (int)tlsio_arduino_get_state(tlsioHandle));
            tlsioInterfaces->concrete_io_dowork(tlsioHandle);
        }

        ///assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_ARE_EQUAL(int, 1, currentmalloc_call);
        ASSERT_CALLBACK_COUNTERS(1, 1, 0, 0, 0);
        ASSERT_ARE_EQUAL(void_ptr, CallbackContext, on_io_error_context);
        ASSERT_ARE_EQUAL(int, (int)IO_OPEN_CANCELLED, (int)on_io_open_complete_result);
        ASSERT_ARE_EQUAL(int, (int)TLSIO_ARDUINO_STATE_ERROR, (int)tlsio_arduino_get_state(tlsioHandle));

        ///cleanup
        tlsioInterfaces->concrete_io_destroy(tlsioHandle);
    }

    /* Tests_SRS_TLSIO_ARDUINO_21_027: [ The tlsio_arduino_open shall set the tlsio to try to open the connection for 10 times before assuming that connection failed. ]*/
    /* Tests_SRS_TLSIO_ARDUINO_21_063: [ If the tlsio state is TLSIO_ARDUINO_STATE_OPENING, and ssl client is connected, the tlsio_arduino_dowork shall change the tlsio state to TLSIO_ARDUINO_STATE_OPEN, and call the on_io_open_complete with IO_OPEN_OK. ]*/
    TEST_FUNCTION(tlsio_arduino_open__retry_10_times__succeed)
    {
        ///arrange
        CONCRETE_IO_HANDLE tlsioHandle;
        int result;
        const IO_INTERFACE_DESCRIPTION* tlsioInterfaces = tlsio_arduino_get_interface_description();
        ASSERT_IS_NOT_NULL(tlsioInterfaces);

        my_sslClient_new_return = SSLClientHandel;
        my_sslClient_hostByName_return = SSLCLIENT_IP_ADDRESS_SUCCEED;
        my_sslClient_connected_return_list = (const bool*)ConnectedReturnList_11ft;
        my_sslClient_connect_return_list = (const int*)ConnectReturnList_true;

        STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG)).IgnoreArgument(1);
        STRICT_EXPECTED_CALL(sslClient_new());
        STRICT_EXPECTED_CALL(sslClient_setTimeout(SSLClientHandel, 10000));
        STRICT_EXPECTED_CALL(sslClient_hostByName(TEST_CREATE_CONNECTION_HOST_NAME, IGNORED_PTR_ARG)).IgnoreArgument(2);
        STRICT_EXPECTED_CALL(sslClient_connected(SSLClientHandel));
        STRICT_EXPECTED_CALL(sslClient_connect(SSLClientHandel, SSLCLIENT_IP_ADDRESS, TEST_CREATE_CONNECTION_PORT));
        for (size_t i = 0; i < 11; i++)
        {
            STRICT_EXPECTED_CALL(sslClient_connected(SSLClientHandel));
        }

        tlsioHandle = tlsioInterfaces->concrete_io_create((void*)&tlsioConfig);
        ASSERT_IS_NOT_NULL(tlsioHandle);

        result = tlsioInterfaces->concrete_io_open(
            tlsioHandle,
            on_io_open_complete, CallbackContext,
            on_bytes_received, CallbackContext,
            on_io_error, CallbackContext);
        ASSERT_ARE_EQUAL(int, 0, result);

        ///act
        for (size_t i = 0; i < 10; i++)
        {
            ASSERT_ARE_EQUAL(int, (int)TLSIO_ARDUINO_STATE_OPENING, (int)tlsio_arduino_get_state(tlsioHandle));
            tlsioInterfaces->concrete_io_dowork(tlsioHandle);
        }

        ///assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_ARE_EQUAL(int, 1, currentmalloc_call);
        ASSERT_CALLBACK_COUNTERS(0, 1, 0, 0, 0);
        ASSERT_ARE_EQUAL(int, (int)IO_OPEN_OK, (int)on_io_open_complete_result);
        ASSERT_ARE_EQUAL(int, (int)TLSIO_ARDUINO_STATE_OPEN, (int)tlsio_arduino_get_state(tlsioHandle));

        ///cleanup
        tlsioInterfaces->concrete_io_destroy(tlsioHandle);
    }

    /* Tests_SRS_TLSIO_ARDUINO_21_028: [ The tlsio_arduino_open shall store the provided on_io_open_complete callback function address. ]*/
    /* Tests_SRS_TLSIO_ARDUINO_21_029: [ The tlsio_arduino_open shall store the provided on_io_open_complete_context handle. ]*/
    /* Tests_SRS_TLSIO_ARDUINO_21_006: [ The tlsio_arduino shall return the status of all async operations using the callbacks. ]*/
    /* Tests_SRS_TLSIO_ARDUINO_21_007: [ If the callback function is set as NULL. The tlsio_arduino shall not call anything. ]*/
    TEST_FUNCTION(tlsio_arduino_open__without_callback__succeed)
    {
        ///arrange
        CONCRETE_IO_HANDLE tlsioHandle;
        int result;
        const IO_INTERFACE_DESCRIPTION* tlsioInterfaces = tlsio_arduino_get_interface_description();
        ASSERT_IS_NOT_NULL(tlsioInterfaces);

        my_sslClient_new_return = SSLClientHandel;
        my_sslClient_hostByName_return = SSLCLIENT_IP_ADDRESS_SUCCEED;
        my_sslClient_connected_return_list = (const bool*)ConnectedReturnList_ft;
        my_sslClient_connect_return_list = (const int*)ConnectReturnList_true;

        STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG)).IgnoreArgument(1);
        STRICT_EXPECTED_CALL(sslClient_new());
        STRICT_EXPECTED_CALL(sslClient_setTimeout(SSLClientHandel, 10000));
        STRICT_EXPECTED_CALL(sslClient_hostByName(TEST_CREATE_CONNECTION_HOST_NAME, IGNORED_PTR_ARG)).IgnoreArgument(2);
        STRICT_EXPECTED_CALL(sslClient_connected(SSLClientHandel));
        STRICT_EXPECTED_CALL(sslClient_connect(SSLClientHandel, SSLCLIENT_IP_ADDRESS, TEST_CREATE_CONNECTION_PORT));
        STRICT_EXPECTED_CALL(sslClient_connected(SSLClientHandel));

        tlsioHandle = tlsioInterfaces->concrete_io_create((void*)&tlsioConfig);
        ASSERT_IS_NOT_NULL(tlsioHandle);

        ///act
        result = tlsioInterfaces->concrete_io_open(
            tlsioHandle,
            NULL, NULL,
            NULL, NULL,
            NULL, NULL);

        ///assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_ARE_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(int, 1, currentmalloc_call);
        ASSERT_CALLBACK_COUNTERS(0, 0, 0, 0, 0);
        ASSERT_ARE_EQUAL(int, (int)IO_OPEN_OK, (int)on_io_open_complete_result);
        ASSERT_ARE_EQUAL(int, (int)TLSIO_ARDUINO_STATE_OPEN, (int)tlsio_arduino_get_state(tlsioHandle));

        ///cleanup
        tlsioInterfaces->concrete_io_destroy(tlsioHandle);
    }

    /* Tests_SRS_TLSIO_ARDUINO_21_002: [ The tlsio_arduino shall report the open operation status using the IO_OPEN_RESULT enumerator defined in the `xio.h`.]*/
    /* Tests_SRS_TLSIO_ARDUINO_21_026: [ The tlsio_arduino_open shall star the process to open the ssl connection with the host provided in the tlsio_arduino_create. ]*/
    /* Tests_SRS_TLSIO_ARDUINO_21_028: [ The tlsio_arduino_open shall store the provided on_io_open_complete callback function address. ]*/
    /* Tests_SRS_TLSIO_ARDUINO_21_029: [ The tlsio_arduino_open shall store the provided on_io_open_complete_context handle. ]*/
    /* Tests_SRS_TLSIO_ARDUINO_21_041: [ If the tlsio_arduino_open get success opening the tls connection, it shall call the tlsio_arduino_dowork. ]*/
    TEST_FUNCTION(tlsio_arduino_open__succeed)
    {
        ///arrange
        CONCRETE_IO_HANDLE tlsioHandle;
        int result;
        const IO_INTERFACE_DESCRIPTION* tlsioInterfaces = tlsio_arduino_get_interface_description();
        ASSERT_IS_NOT_NULL(tlsioInterfaces);

        my_sslClient_new_return = SSLClientHandel;
        my_sslClient_hostByName_return = SSLCLIENT_IP_ADDRESS_SUCCEED;
        my_sslClient_connected_return_list = (const bool*)ConnectedReturnList_ft;
        my_sslClient_connect_return_list = (const int*)ConnectReturnList_true;

        STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG)).IgnoreArgument(1);
        STRICT_EXPECTED_CALL(sslClient_new());
        STRICT_EXPECTED_CALL(sslClient_setTimeout(SSLClientHandel, 10000));
        STRICT_EXPECTED_CALL(sslClient_hostByName(TEST_CREATE_CONNECTION_HOST_NAME, IGNORED_PTR_ARG)).IgnoreArgument(2);
        STRICT_EXPECTED_CALL(sslClient_connected(SSLClientHandel));
        STRICT_EXPECTED_CALL(sslClient_connect(SSLClientHandel, SSLCLIENT_IP_ADDRESS, TEST_CREATE_CONNECTION_PORT));
        STRICT_EXPECTED_CALL(sslClient_connected(SSLClientHandel));

        tlsioHandle = tlsioInterfaces->concrete_io_create((void*)&tlsioConfig);
        ASSERT_IS_NOT_NULL(tlsioHandle);

        ///act
        result = tlsioInterfaces->concrete_io_open(
            tlsioHandle,
            on_io_open_complete, CallbackContext,
            on_bytes_received, CallbackContext,
            on_io_error, CallbackContext);

        ///assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_ARE_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(int, 1, currentmalloc_call);
        ASSERT_CALLBACK_COUNTERS(0, 1, 0, 0, 0);
        ASSERT_ARE_EQUAL(void_ptr, CallbackContext, on_io_open_complete_context);
        ASSERT_ARE_EQUAL(int, (int)IO_OPEN_OK, (int)on_io_open_complete_result);
        ASSERT_ARE_EQUAL(int, (int)TLSIO_ARDUINO_STATE_OPEN, (int)tlsio_arduino_get_state(tlsioHandle));
        
        ///cleanup
        tlsioInterfaces->concrete_io_destroy(tlsioHandle);
    }

    /* Tests_SRS_TLSIO_ARDUINO_21_026: [ The tlsio_arduino_open shall star the process to open the ssl connection with the host provided in the tlsio_arduino_create. ]*/
    /* Tests_SRS_TLSIO_ARDUINO_21_034: [ If tlsio_arduino_open get success to start the process to open the ssl connection, it shall set the tlsio state as TLSIO_ARDUINO_STATE_OPENING, and return 0. ]*/
    /* Tests_SRS_TLSIO_ARDUINO_21_041: [ If the tlsio_arduino_open get success opening the tls connection, it shall call the tlsio_arduino_dowork. ]*/
    TEST_FUNCTION(tlsio_arduino_open__stopping_on_TLSIO_ARDUINO_STATE_OPENING__succeed)
    {
        ///arrange
        CONCRETE_IO_HANDLE tlsioHandle;
        int result;
        const IO_INTERFACE_DESCRIPTION* tlsioInterfaces = tlsio_arduino_get_interface_description();
        ASSERT_IS_NOT_NULL(tlsioInterfaces);

        my_sslClient_new_return = SSLClientHandel;
        my_sslClient_hostByName_return = SSLCLIENT_IP_ADDRESS_SUCCEED;
        my_sslClient_connected_return_list = (const bool*)ConnectedReturnList_ff;
        my_sslClient_connect_return_list = (const int*)ConnectReturnList_true;

        STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG)).IgnoreArgument(1);
        STRICT_EXPECTED_CALL(sslClient_new());
        STRICT_EXPECTED_CALL(sslClient_setTimeout(SSLClientHandel, 10000));
        STRICT_EXPECTED_CALL(sslClient_hostByName(TEST_CREATE_CONNECTION_HOST_NAME, IGNORED_PTR_ARG)).IgnoreArgument(2);
        STRICT_EXPECTED_CALL(sslClient_connected(SSLClientHandel));
        STRICT_EXPECTED_CALL(sslClient_connect(SSLClientHandel, SSLCLIENT_IP_ADDRESS, TEST_CREATE_CONNECTION_PORT));
        STRICT_EXPECTED_CALL(sslClient_connected(SSLClientHandel));

        tlsioHandle = tlsioInterfaces->concrete_io_create((void*)&tlsioConfig);
        ASSERT_IS_NOT_NULL(tlsioHandle);

        ///act
        result = tlsioInterfaces->concrete_io_open(
            tlsioHandle, 
            on_io_open_complete, CallbackContext,
            on_bytes_received, CallbackContext,
            on_io_error, CallbackContext);

        ///assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_ARE_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(int, 1, currentmalloc_call);
        ASSERT_CALLBACK_COUNTERS(0, 0, 0, 0, 0);
        ASSERT_ARE_EQUAL(int, (int)TLSIO_ARDUINO_STATE_OPENING, (int)tlsio_arduino_get_state(tlsioHandle));

        ///cleanup
        tlsioInterfaces->concrete_io_destroy(tlsioHandle);
    }

    /* Tests_SRS_TLSIO_ARDUINO_21_025: [ If the tlsio state is TLSIO_ARDUINO_STATE_OPENING, TLSIO_ARDUINO_STATE_OPEN, or TLSIO_ARDUINO_STATE_CLOSING, the tlsio_arduino_destroy shall close destroy the tlsio, but log an error. ]*/
    TEST_FUNCTION(tlsio_arduino_destroy_on_TLSIO_ARDUINO_STATE_OPEN__failed)
    {
        ///arrange
        CONCRETE_IO_HANDLE tlsioHandle;
        int result;
        const IO_INTERFACE_DESCRIPTION* tlsioInterfaces = tlsio_arduino_get_interface_description();
        ASSERT_IS_NOT_NULL(tlsioInterfaces);

        my_sslClient_new_return = SSLClientHandel;
        my_sslClient_hostByName_return = SSLCLIENT_IP_ADDRESS_SUCCEED;
        my_sslClient_connected_return_list = (const bool*)ConnectedReturnList_ft;
        my_sslClient_connect_return_list = (const int*)ConnectReturnList_true;

        STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG)).IgnoreArgument(1);
        STRICT_EXPECTED_CALL(sslClient_new());
        STRICT_EXPECTED_CALL(sslClient_setTimeout(SSLClientHandel, 10000));
        STRICT_EXPECTED_CALL(sslClient_hostByName(TEST_CREATE_CONNECTION_HOST_NAME, IGNORED_PTR_ARG)).IgnoreArgument(2);
        STRICT_EXPECTED_CALL(sslClient_connected(SSLClientHandel));
        STRICT_EXPECTED_CALL(sslClient_connect(SSLClientHandel, SSLCLIENT_IP_ADDRESS, TEST_CREATE_CONNECTION_PORT));
        STRICT_EXPECTED_CALL(sslClient_connected(SSLClientHandel));
        STRICT_EXPECTED_CALL(sslClient_delete(IGNORED_PTR_ARG)).IgnoreArgument(1);
        STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG)).IgnoreArgument(1);

        tlsioHandle = tlsioInterfaces->concrete_io_create((void*)&tlsioConfig);
        ASSERT_IS_NOT_NULL(tlsioHandle);

        result = tlsioInterfaces->concrete_io_open(
            tlsioHandle,
            on_io_open_complete, CallbackContext,
            on_bytes_received, CallbackContext,
            on_io_error, CallbackContext);
        ASSERT_ARE_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(int, (int)TLSIO_ARDUINO_STATE_OPEN, (int)tlsio_arduino_get_state(tlsioHandle));

        ///act
        tlsioInterfaces->concrete_io_destroy(tlsioHandle);

        ///assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_ARE_EQUAL(int, 0, currentmalloc_call);
        ASSERT_CALLBACK_COUNTERS(0, 1, 0, 0, 0);

        ///cleanup
    }

    /* Tests_SRS_TLSIO_ARDUINO_21_025: [ If the tlsio state is TLSIO_ARDUINO_STATE_OPENING, TLSIO_ARDUINO_STATE_OPEN, or TLSIO_ARDUINO_STATE_CLOSING, the tlsio_arduino_destroy shall close destroy the tlsio, but log an error. ]*/
    TEST_FUNCTION(tlsio_arduino_destroy_on_TLSIO_ARDUINO_STATE_OPENING__failed)
    {
        ///arrange
        CONCRETE_IO_HANDLE tlsioHandle;
        int result;
        const IO_INTERFACE_DESCRIPTION* tlsioInterfaces = tlsio_arduino_get_interface_description();
        ASSERT_IS_NOT_NULL(tlsioInterfaces);

        my_sslClient_new_return = SSLClientHandel;
        my_sslClient_hostByName_return = SSLCLIENT_IP_ADDRESS_SUCCEED;
        my_sslClient_connected_return_list = (const bool*)ConnectedReturnList_ff;
        my_sslClient_connect_return_list = (const int*)ConnectReturnList_true;

        STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG)).IgnoreArgument(1);
        STRICT_EXPECTED_CALL(sslClient_new());
        STRICT_EXPECTED_CALL(sslClient_setTimeout(SSLClientHandel, 10000));
        STRICT_EXPECTED_CALL(sslClient_hostByName(TEST_CREATE_CONNECTION_HOST_NAME, IGNORED_PTR_ARG)).IgnoreArgument(2);
        STRICT_EXPECTED_CALL(sslClient_connected(SSLClientHandel));
        STRICT_EXPECTED_CALL(sslClient_connect(SSLClientHandel, SSLCLIENT_IP_ADDRESS, TEST_CREATE_CONNECTION_PORT));
        STRICT_EXPECTED_CALL(sslClient_connected(SSLClientHandel));
        STRICT_EXPECTED_CALL(sslClient_delete(IGNORED_PTR_ARG)).IgnoreArgument(1);
        STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG)).IgnoreArgument(1);

        tlsioHandle = tlsioInterfaces->concrete_io_create((void*)&tlsioConfig);
        ASSERT_IS_NOT_NULL(tlsioHandle);

        result = tlsioInterfaces->concrete_io_open(
            tlsioHandle,
            on_io_open_complete, CallbackContext,
            on_bytes_received, CallbackContext,
            on_io_error, CallbackContext);
        ASSERT_ARE_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(int, (int)TLSIO_ARDUINO_STATE_OPENING, (int)tlsio_arduino_get_state(tlsioHandle));

        ///act
        tlsioInterfaces->concrete_io_destroy(tlsioHandle);

        ///assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_ARE_EQUAL(int, 0, currentmalloc_call);
        ASSERT_CALLBACK_COUNTERS(0, 0, 0, 0, 0);

        ///cleanup
    }


    /* Tests_SRS_TLSIO_ARDUINO_21_024: [ If the tlsio_handle is NULL, the tlsio_arduino_destroy shall not do anything. ]*/
    TEST_FUNCTION(tlsio_arduino_destroy__NULL_tlsio_handle__failed)
    {
        ///arrange
        const IO_INTERFACE_DESCRIPTION* tlsioInterfaces = tlsio_arduino_get_interface_description();
        ASSERT_IS_NOT_NULL(tlsioInterfaces);

        ///act
        tlsioInterfaces->concrete_io_destroy(NULL);

        ///assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_ARE_EQUAL(int, 0, currentmalloc_call);
        ASSERT_CALLBACK_COUNTERS(0, 0, 0, 0, 0);

        ///cleanup
    }

    /* Tests_SRS_TLSIO_ARDUINO_21_021: [ The tlsio_arduino_destroy shall destroy a created instance of the tlsio for Arduino identified by the CONCRETE_IO_HANDLE. ]*/
    /* Tests_SRS_TLSIO_ARDUINO_21_022: [ The tlsio_arduino_destroy shall free the memory allocated for tlsio_instance. ]*/
    /* Tests_SRS_TLSIO_ARDUINO_21_023: [ If there is an instance of sslClient, the tlsio_arduino_destroy shall destroy the instance of the sslClient. ]*/
    TEST_FUNCTION(tlsio_arduino_destroy__succeed)
    {
        ///arrange
        CONCRETE_IO_HANDLE tlsioHandle;
        const IO_INTERFACE_DESCRIPTION* tlsioInterfaces = tlsio_arduino_get_interface_description();
        ASSERT_IS_NOT_NULL(tlsioInterfaces);

        my_sslClient_new_return = SSLClientHandel;
        my_sslClient_hostByName_return = SSLCLIENT_IP_ADDRESS_SUCCEED;

        STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG)).IgnoreArgument(1);
        STRICT_EXPECTED_CALL(sslClient_new());
        STRICT_EXPECTED_CALL(sslClient_setTimeout(SSLClientHandel, 10000));
        STRICT_EXPECTED_CALL(sslClient_hostByName(TEST_CREATE_CONNECTION_HOST_NAME, IGNORED_PTR_ARG)).IgnoreArgument(2);
        STRICT_EXPECTED_CALL(sslClient_delete(IGNORED_PTR_ARG)).IgnoreArgument(1);
        STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG)).IgnoreArgument(1);

        tlsioHandle = tlsioInterfaces->concrete_io_create((void*)&tlsioConfig);
        ASSERT_IS_NOT_NULL(tlsioHandle);

        ///act
        tlsioInterfaces->concrete_io_destroy(tlsioHandle);

        ///assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_ARE_EQUAL(int, 0, currentmalloc_call);
        ASSERT_CALLBACK_COUNTERS(0, 0, 0, 0, 0);

        ///cleanup
    }

    /* Tests_SRS_TLSIO_ARDUINO_21_009: [ The tlsio_arduino_create shall create a new instance of the tlsio for Arduino. ]*/
    /* Tests_SRS_TLSIO_ARDUINO_21_010: [ The tlsio_arduino_create shall return a non-NULL handle on success. ]*/
    /* Tests_SRS_TLSIO_ARDUINO_21_011: [ The tlsio_arduino_create shall allocate memory to control the tlsio instance. ]*/
    /* Tests_SRS_TLSIO_ARDUINO_21_013: [ The tlsio_arduino_create shall create a new instance of the sslClient. ]*/
    /* Tests_SRS_TLSIO_ARDUINO_21_015: [ The tlsio_arduino_create shall set 10 seconds for the sslClient timeout. ]*/
    /* Tests_SRS_TLSIO_ARDUINO_21_017: [ The tlsio_arduino_create shall receive the connection configuration (TLSIO_CONFIG). ]*/
    /* Tests_SRS_TLSIO_ARDUINO_21_018: [ The tlsio_arduino_create shall convert the provide hostName to an IP address. ]*/
    /* Tests_SRS_TLSIO_ARDUINO_21_016: [ The tlsio_arduino_create shall initialize all callback pointers as NULL. ]*/
    /* Tests_SRS_TLSIO_ARDUINO_21_020: [ If tlsio_arduino_create get success to create the tlsio instance, it shall set the tlsio state as TLSIO_ARDUINO_STATE_CLOSED. ]*/
    TEST_FUNCTION(tlsio_arduino_create__succeed)
    {
        ///arrange
        CONCRETE_IO_HANDLE tlsioHandle;
        const IO_INTERFACE_DESCRIPTION* tlsioInterfaces = tlsio_arduino_get_interface_description();
        ASSERT_IS_NOT_NULL(tlsioInterfaces);

        my_sslClient_new_return = SSLClientHandel;
        my_sslClient_hostByName_return = SSLCLIENT_IP_ADDRESS_SUCCEED;

        STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG)).IgnoreArgument(1);
        STRICT_EXPECTED_CALL(sslClient_new());
        STRICT_EXPECTED_CALL(sslClient_setTimeout(SSLClientHandel, 10000));
        STRICT_EXPECTED_CALL(sslClient_hostByName(TEST_CREATE_CONNECTION_HOST_NAME, IGNORED_PTR_ARG)).IgnoreArgument(2);
        
        ///act
        tlsioHandle = tlsioInterfaces->concrete_io_create((void*)&tlsioConfig);

        ///assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_ARE_EQUAL(int, 1, currentmalloc_call);
        ASSERT_IS_NOT_NULL(tlsioHandle);
        ASSERT_ARE_EQUAL(int, (int)TLSIO_ARDUINO_STATE_CLOSED, (int)tlsio_arduino_get_state(tlsioHandle));
        ASSERT_CALLBACK_COUNTERS(0, 0, 0, 0, 0);

        ///cleanup
        tlsioInterfaces->concrete_io_destroy(tlsioHandle);
    }

    /* Tests_SRS_TLSIO_ARDUINO_21_019: [ If the WiFi cannot find the IP for the hostName, the tlsio_arduino_create shall destroy the sslClient and tlsio instances and return NULL as the handle. ]*/
    TEST_FUNCTION(tlsio_arduino_create__wifi_failed_to_convert_hostName_to_IP__failed)
    {
        ///arrange
        CONCRETE_IO_HANDLE tlsioHandle;
        const IO_INTERFACE_DESCRIPTION* tlsioInterfaces = tlsio_arduino_get_interface_description();
        ASSERT_IS_NOT_NULL(tlsioInterfaces);

        my_sslClient_new_return = SSLClientHandel;
        my_sslClient_hostByName_return = SSLCLIENT_IP_ADDRESS_FAILED;

        STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG)).IgnoreArgument(1);
        STRICT_EXPECTED_CALL(sslClient_new());
        STRICT_EXPECTED_CALL(sslClient_setTimeout(SSLClientHandel, 10000));
        STRICT_EXPECTED_CALL(sslClient_hostByName(TEST_CREATE_CONNECTION_HOST_NAME, IGNORED_PTR_ARG)).IgnoreArgument(2);
        STRICT_EXPECTED_CALL(sslClient_delete(IGNORED_PTR_ARG)).IgnoreArgument(1);
        STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG)).IgnoreArgument(1);
        
        ///act
        tlsioHandle = tlsioInterfaces->concrete_io_create((void*)&tlsioConfig);

        ///assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_ARE_EQUAL(int, 0, currentmalloc_call);
        ASSERT_IS_NULL(tlsioHandle);
        ASSERT_CALLBACK_COUNTERS(0, 0, 0, 0, 0);
        ASSERT_ARE_EQUAL(int, (int)TLSIO_ARDUINO_STATE_NULL, (int)tlsio_arduino_get_state(tlsioHandle));

        ///cleanup
    }

    /* Tests_SRS_TLSIO_ARDUINO_21_014: [ If the tlsio_arduino_create failed to create a new instance of the sslClient, it shall return NULL as the handle. ]*/
    TEST_FUNCTION(tlsio_arduino_create__failed_to_create_sslClient_instace__failed)
    {
        ///arrange
        CONCRETE_IO_HANDLE tlsioHandle;
        const IO_INTERFACE_DESCRIPTION* tlsioInterfaces = tlsio_arduino_get_interface_description();
        ASSERT_IS_NOT_NULL(tlsioInterfaces);

        my_sslClient_new_return = SSLCLIENT_HANDEL_NULL;

        STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG)).IgnoreArgument(1);
        STRICT_EXPECTED_CALL(sslClient_new());
        STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG)).IgnoreArgument(1);
        
        ///act
        tlsioHandle = tlsioInterfaces->concrete_io_create((void*)&tlsioConfig);

        ///assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_ARE_EQUAL(int, 0, currentmalloc_call);
        ASSERT_CALLBACK_COUNTERS(0, 0, 0, 0, 0);
        ASSERT_IS_NULL(tlsioHandle);

        ///cleanup
    }

    /* Tests_SRS_TLSIO_ARDUINO_21_012: [ If there is not enough memory to control the tlsio, the tlsio_arduino_create shall return NULL as the handle. ]*/
    /* Tests_SRS_TLSIO_ARDUINO_21_005: [ The tlsio_arduino shall received the connection information using the TLSIO_CONFIG structure defined in `tlsio.h`. ]*/
    TEST_FUNCTION(tlsio_arduino_create__not_enough_memory_to_control_tlsio__failed)
    {
        ///arrange
        CONCRETE_IO_HANDLE tlsioHandle;
        const IO_INTERFACE_DESCRIPTION* tlsioInterfaces = tlsio_arduino_get_interface_description();
        ASSERT_IS_NOT_NULL(tlsioInterfaces);

        my_sslClient_new_return = SSLClientHandel;
        whenShallmalloc_fail = 1;

        STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG)).IgnoreArgument(1);

        ///act
        tlsioHandle = tlsioInterfaces->concrete_io_create((void*)&tlsioConfig);

        ///assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_ARE_EQUAL(int, 0, currentmalloc_call);
        ASSERT_IS_NULL(tlsioHandle);
        ASSERT_CALLBACK_COUNTERS(0, 0, 0, 0, 0);

        ///cleanup
    }

    /* Tests_SRS_TLSIO_ARDUINO_21_001: [ The tlsio_arduino shall implement and export all the Concrete functions in the VTable IO_INTERFACE_DESCRIPTION defined in the `xio.h`. ]*/
    /* Tests_SRS_TLSIO_ARDUINO_21_008: [ The tlsio_arduino_get_interface_description shall return the VTable IO_INTERFACE_DESCRIPTION. ]*/
    TEST_FUNCTION(tlsio_arduino_get_interface_description__succeed)
    {
        ///arrange
		const IO_INTERFACE_DESCRIPTION* tlsioInterfaces;

        ///act
        tlsioInterfaces = tlsio_arduino_get_interface_description();

        ///assert
        ASSERT_IS_NOT_NULL(tlsioInterfaces);

        ///cleanup
    }


END_TEST_SUITE(tlsioarduino_ut)
