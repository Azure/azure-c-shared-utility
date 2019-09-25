// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.
#ifdef __cplusplus
#include <cstdlib>
#include <cstddef>
#else
#include <stdlib.h>
#include <stddef.h>
#endif

static void* my_gballoc_malloc(size_t size)
{
    return malloc(size);
}

static void* my_gballoc_realloc(void* ptr, size_t size)
{
    return realloc(ptr, size);
}

static void my_gballoc_free(void* ptr)
{
    free(ptr);
}

#include "testrunnerswitcher.h"
#include "umock_c/umock_c.h"
#include "umock_c/umocktypes_charptr.h"
#include "umock_c/umocktypes_stdint.h"
#include "umock_c/umock_c_negative_tests.h"
#include "azure_macro_utils/macro_utils.h"

#include "azure_c_shared_utility/tlsio.h"
#include "azure_c_shared_utility/socketio.h"
#include "azure_c_shared_utility/xio.h"
#include "azure_c_shared_utility/platform.h"

#include "tlsio_threadx.h"
#include "nx_secure_tls_api.h"
#include "nx_secure_tls.h"

#define ENABLE_MOCKS
#include "azure_c_shared_utility/gballoc.h"
#include "umock_c/umock_c_prod.h"

#include "azure_c_shared_utility/crt_abstractions.h"
#include "azure_c_shared_utility/optimize_size.h"
#include "azure_c_shared_utility/xlogging.h"
#include "azure_c_shared_utility/shared_util_options.h"


MOCKABLE_FUNCTION(, void, on_bytes_recv, void*, context, const unsigned char*, buffer, size_t, size);
MOCKABLE_FUNCTION(, void, on_error, void*, context);
MOCKABLE_FUNCTION(, void, on_close_complete, void*, context);


//MOCKABLE_FUNCTION(, UINT, nx_secure_tls_session_delete, NX_SECURE_TLS_SESSION*, tls_session);

#undef ENABLE_MOCKS


static TEST_MUTEX_HANDLE g_testByTest;
static TEST_MUTEX_HANDLE g_dllByDll;

#define BUFFER_LEN          10
#define TEST_DEVICE_ID      11
#define THREADX_READ_LIMIT  5

static const IO_INTERFACE_DESCRIPTION* TEST_SOCKETIO_INTERFACE_DESCRIPTION = (const IO_INTERFACE_DESCRIPTION*)0x0014;
static XIO_HANDLE TEST_IO_HANDLE = (XIO_HANDLE)0x0015;
static const unsigned char TEST_BUFFER[] = { 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xA };
static const size_t TEST_BUFFER_LEN = BUFFER_LEN;
static const char* TEST_TRUSTED_CERT = "test_trusted_cert";


//static HandShakeDoneCb g_handshake_done_cb = NULL;
static void* g_handshake_done_ctx = NULL;

static ON_BYTES_RECEIVED g_on_bytes_received;
static void* g_on_bytes_received_context;
static ON_IO_ERROR g_on_io_error;
static void* g_on_io_error_context;
//static CallbackIORecv g_threadx_cb_rcv;
static void* g_threadx_rcv_ctx;

MU_DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

static int my_mallocAndStrcpy_s(char** destination, const char* source)
{
    *destination = (char*)malloc(strlen(source) + 1);
    (void)strcpy(*destination, source);
    return 0;
}

static void execute_threadx_open(ON_IO_OPEN_COMPLETE on_io_open_complete, void* on_io_open_complete_context)
{
    on_io_open_complete(on_io_open_complete_context, IO_OPEN_OK);
}

static void on_io_open_complete(void* context, IO_OPEN_RESULT open_result)
{
    (void)context;
    (void)open_result;
}

static void on_send_complete(void* context, IO_SEND_RESULT send_result)
{
    (void)context;
    (void)send_result;
}

static int my_xio_open(XIO_HANDLE xio, ON_IO_OPEN_COMPLETE on_io_open_complete, void* on_io_open_complete_context, ON_BYTES_RECEIVED on_bytes_received, void* on_bytes_received_context, ON_IO_ERROR on_io_error, void* on_io_error_context)
{
    (void)xio;
    g_on_io_error = on_io_error;
    g_on_io_error_context = on_io_error_context;
    g_on_bytes_received = on_bytes_received;
    g_on_bytes_received_context = on_bytes_received_context;

    //execute_wolfssl_open(on_io_open_complete, on_io_open_complete_context);

    return 0;
}

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    ASSERT_FAIL("umock_c reported error :%s", MU_ENUM_TO_STRING(UMOCK_C_ERROR_CODE, error_code));
}

BEGIN_TEST_SUITE(tlsio_threadx_ut)

TEST_SUITE_INITIALIZE(suite_init)
{
    g_testByTest = TEST_MUTEX_CREATE();
    ASSERT_IS_NOT_NULL(g_testByTest);

    umock_c_init(on_umock_c_error);

    REGISTER_UMOCK_ALIAS_TYPE(CONCRETE_IO_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(XIO_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(CallbackIORecv, void*);
    REGISTER_UMOCK_ALIAS_TYPE(HandShakeDoneCb, void*);
    REGISTER_UMOCK_ALIAS_TYPE(ON_IO_OPEN_COMPLETE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(ON_BYTES_RECEIVED, void*);
    REGISTER_UMOCK_ALIAS_TYPE(ON_IO_ERROR, void*);
    REGISTER_UMOCK_ALIAS_TYPE(ON_BYTES_RECEIVED, void*);
    REGISTER_UMOCK_ALIAS_TYPE(ON_IO_CLOSE_COMPLETE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(ON_BYTES_RECEIVED, void*);

    REGISTER_GLOBAL_MOCK_HOOK(gballoc_malloc, my_gballoc_malloc);
    REGISTER_GLOBAL_MOCK_FAIL_RETURN(gballoc_malloc, NULL);
    REGISTER_GLOBAL_MOCK_HOOK(gballoc_realloc, my_gballoc_realloc);
    REGISTER_GLOBAL_MOCK_FAIL_RETURN(gballoc_realloc, NULL);
    REGISTER_GLOBAL_MOCK_HOOK(gballoc_free, my_gballoc_free);
}

TEST_SUITE_CLEANUP(suite_cleanup)
{
    umock_c_deinit();

    TEST_MUTEX_DESTROY(g_testByTest);
}

TEST_FUNCTION_INITIALIZE(TestMethodInitialize)
{
    if (TEST_MUTEX_ACQUIRE(g_testByTest))
    {
        ASSERT_FAIL("our mutex is ABANDONED. Failure in test framework");
    }

    g_on_bytes_received = NULL;
    g_on_bytes_received_context = NULL;
    g_on_io_error = NULL;
    g_on_io_error_context = NULL;
    //g_threadx_cb_rcv = NULL;
    //g_handshake_done_cb = NULL;
    g_handshake_done_ctx = NULL;

    umock_c_reset_all_calls();
}

TEST_FUNCTION_CLEANUP(TestMethodCleanup)
{
    TEST_MUTEX_RELEASE(g_testByTest);
}

TEST_FUNCTION(tlsio_threadx_create_succeeds)
{
    //arrange
    TLSIO_CONFIG tls_io_config;
    memset(&tls_io_config, 0, sizeof(tls_io_config));
    tls_io_config.hostname = "global.azure-devices-provisioning.net";
    tls_io_config.port = 8883;
    
    //act
    CONCRETE_IO_HANDLE io_handle = tlsio_threadx_create(&tls_io_config);

    //assert
    ASSERT_IS_NOT_NULL(io_handle);
    ASSERT_ARE_EQUAL(unsigned_long, _nx_secure_tls_created_count, 1);

    //clean
    tlsio_threadx_destroy(io_handle);
}

TEST_FUNCTION(tlsio_threadx_create_config_hostname_null_fail)
{
    //arrange
    TLSIO_CONFIG tls_io_config;
    memset(&tls_io_config, 0, sizeof(tls_io_config));

    //act
    CONCRETE_IO_HANDLE io_handle = tlsio_threadx_create(&tls_io_config);

    //assert
    ASSERT_IS_NULL(io_handle);
    ASSERT_ARE_EQUAL(unsigned_long, _nx_secure_tls_created_count, 0);

    //clean
}

TEST_FUNCTION(tlsio_wolfssl_create_config_NULL_fail)
{
    //arrange

    //act
    CONCRETE_IO_HANDLE io_handle = tlsio_threadx_create(NULL);

    //assert
    ASSERT_IS_NULL(io_handle);
    ASSERT_ARE_EQUAL(unsigned_long, _nx_secure_tls_created_count, 0);

    //clean
}

TEST_FUNCTION(tlsio_threadx_destroy_succeeds)
{
    //arrange
    TLSIO_CONFIG tls_io_config;
    memset(&tls_io_config, 0, sizeof(tls_io_config));
    tls_io_config.hostname = "global.azure-devices-provisioning.net";
    tls_io_config.port = 8883;
    CONCRETE_IO_HANDLE io_handle = tlsio_threadx_create(&tls_io_config);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    //act
    tlsio_threadx_destroy(io_handle);

    //assert
    ASSERT_ARE_EQUAL(unsigned_long, _nx_secure_tls_created_count, 0);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    //clean
}

TEST_FUNCTION(tlsio_threadx_destroy_handle_NULL_succeeds)
{
    //arrange

    //act
    tlsio_threadx_destroy(NULL);

    //assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    //clean
}

TEST_FUNCTION(tlsio_threadx_open_handle_NULL_fail)
{
    //arrange

    //act
    int test_result = tlsio_threadx_open(NULL, on_io_open_complete, NULL, on_bytes_recv, NULL, on_error, NULL);

    //assert
    ASSERT_ARE_NOT_EQUAL(int, 0, test_result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    //clean
}

TEST_FUNCTION(tlsio_threadx_open_succeeds)
{
    //arrange
    TLSIO_CONFIG tls_io_config;
    memset(&tls_io_config, 0, sizeof(tls_io_config));
    tls_io_config.hostname = "global.azure-devices-provisioning.net";
    tls_io_config.port = 8883;
    CONCRETE_IO_HANDLE io_handle = tlsio_threadx_create(&tls_io_config);
    umock_c_reset_all_calls();

    //act
    int test_result = tlsio_threadx_open(io_handle, on_io_open_complete, NULL, on_bytes_recv, NULL, on_error, NULL);

    //assert
    ASSERT_ARE_EQUAL(int, 0, test_result);

    //clean
    (void)tlsio_threadx_close(io_handle, on_close_complete, NULL);
    tlsio_threadx_destroy(io_handle);
}

TEST_FUNCTION(tlsio_threadx_close_handle_NULL_fail)
{
    //arrange

    //act
    int test_result = tlsio_threadx_close(NULL, on_close_complete, NULL);

    //assert
    ASSERT_ARE_NOT_EQUAL(int, 0, test_result);

    //clean
}

TEST_FUNCTION(tlsio_threadx_close_succeeds)
{
    //arrange
    TLSIO_CONFIG tls_io_config;
    memset(&tls_io_config, 0, sizeof(tls_io_config));
    tls_io_config.hostname = "global.azure-devices-provisioning.net";
    tls_io_config.port = 8883;
    CONCRETE_IO_HANDLE io_handle = tlsio_threadx_create(&tls_io_config);
    (void)tlsio_threadx_open(io_handle, on_io_open_complete, NULL, on_bytes_recv, NULL, on_error, NULL);
    umock_c_reset_all_calls();

    //act
    int test_result = tlsio_threadx_close(io_handle, on_close_complete, NULL);

    //assert
    ASSERT_ARE_EQUAL(int, 0, test_result);

    //clean
    tlsio_threadx_destroy(io_handle);
}

TEST_FUNCTION(tlsio_threadx_close_not_open_succeeds)
{
    //arrange
    TLSIO_CONFIG tls_io_config;
    memset(&tls_io_config, 0, sizeof(tls_io_config));
    tls_io_config.hostname = "global.azure-devices-provisioning.net";
    tls_io_config.port = 8883;
    CONCRETE_IO_HANDLE io_handle = tlsio_threadx_create(&tls_io_config);
    umock_c_reset_all_calls();

    //act
    int test_result = tlsio_threadx_close(io_handle, on_close_complete, NULL);

    //assert
    ASSERT_ARE_NOT_EQUAL(int, 0, test_result);

    //clean
    tlsio_threadx_destroy(io_handle);
}

TEST_FUNCTION(tlsio_threadx_send_handle_NULL_fail)
{
    //arrange
    umock_c_reset_all_calls();

    //act
    int test_result = tlsio_threadx_send(NULL, TEST_BUFFER, TEST_BUFFER_LEN, on_send_complete, NULL);

    //assert
    ASSERT_ARE_NOT_EQUAL(int, 0, test_result);

    //clean
}

TEST_FUNCTION(tlsio_threadx_send_buffer_0_fail)
{
    //arrange
    TLSIO_CONFIG tls_io_config;
    memset(&tls_io_config, 0, sizeof(tls_io_config));
    tls_io_config.hostname = "global.azure-devices-provisioning.net";
    tls_io_config.port = 8883;
    CONCRETE_IO_HANDLE io_handle = tlsio_threadx_create(&tls_io_config);
    (void)tlsio_threadx_open(io_handle, on_io_open_complete, NULL, on_bytes_recv, NULL, on_error, NULL);
    umock_c_reset_all_calls();

    //act
    int test_result = tlsio_threadx_send(io_handle, NULL, 0, on_send_complete, NULL);

    //assert
    ASSERT_ARE_NOT_EQUAL(int, 0, test_result);

    //clean
    (void)tlsio_threadx_close(io_handle, on_close_complete, NULL);
    tlsio_threadx_destroy(io_handle);
}

TEST_FUNCTION(tlsio_threadx_send_not_open_fail)
{
    //arrange
    TLSIO_CONFIG tls_io_config;
    memset(&tls_io_config, 0, sizeof(tls_io_config));
    tls_io_config.hostname = "global.azure-devices-provisioning.net";
    tls_io_config.port = 8883;
    CONCRETE_IO_HANDLE io_handle = tlsio_threadx_create(&tls_io_config);
    umock_c_reset_all_calls();

    //act
    int test_result = tlsio_threadx_send(io_handle, TEST_BUFFER, TEST_BUFFER_LEN, on_send_complete, NULL);

    //assert
    ASSERT_ARE_NOT_EQUAL(int, 0, test_result);

    //clean
    tlsio_threadx_destroy(io_handle);
}

TEST_FUNCTION(tlsio_threadx_send_succeeds)
{
    //arrange
    TLSIO_CONFIG tls_io_config;
    memset(&tls_io_config, 0, sizeof(tls_io_config));
    tls_io_config.hostname = "global.azure-devices-provisioning.net";
    tls_io_config.port = 8883;
    CONCRETE_IO_HANDLE io_handle = tlsio_threadx_create(&tls_io_config);
    (void)tlsio_threadx_open(io_handle, on_io_open_complete, NULL, on_bytes_recv, NULL, on_error, NULL);
    umock_c_reset_all_calls();

    //act
    int test_result = tlsio_threadx_send(io_handle, TEST_BUFFER, TEST_BUFFER_LEN, on_send_complete, NULL);

    //assert
    ASSERT_ARE_EQUAL(int, 0, test_result);

    //clean
    (void)tlsio_threadx_close(io_handle, on_close_complete, NULL);
    tlsio_threadx_destroy(io_handle);
}

TEST_FUNCTION(tlsio_threadx_dowork_handle_NULL_succeeds)
{
    //arrange

    //act
    tlsio_threadx_dowork(NULL);

    //assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    //clean
}

TEST_FUNCTION(tlsio_threadx_dowork_NOT_OPEN_succeeds)
{
    //arrange
    TLSIO_CONFIG tls_io_config;
    memset(&tls_io_config, 0, sizeof(tls_io_config));
    tls_io_config.hostname = "global.azure-devices-provisioning.net";
    tls_io_config.port = 8883;
    CONCRETE_IO_HANDLE io_handle = tlsio_threadx_create(&tls_io_config);
    umock_c_reset_all_calls();

    //act
    tlsio_threadx_dowork(NULL);

    //assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    //clean
    tlsio_threadx_destroy(io_handle);
}

TEST_FUNCTION(tlsio_threadx_dowork_succeeds)
{
    //arrange
    TLSIO_CONFIG tls_io_config;
    memset(&tls_io_config, 0, sizeof(tls_io_config));
    tls_io_config.hostname = "global.azure-devices-provisioning.net";
    tls_io_config.port = 8883;
    CONCRETE_IO_HANDLE io_handle = tlsio_threadx_create(&tls_io_config);
    (void)tlsio_threadx_open(io_handle, on_io_open_complete, NULL, on_bytes_recv, NULL, on_error, NULL);
    umock_c_reset_all_calls();

    //act
    tlsio_threadx_dowork(io_handle);

    //assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    //clean
    (void)tlsio_threadx_close(io_handle, on_close_complete, NULL);
    tlsio_threadx_destroy(io_handle);
}

TEST_FUNCTION(tlsio_threadx_get_interface_description_succeed)
{
    //arrange
    umock_c_reset_all_calls();

    //act
    const IO_INTERFACE_DESCRIPTION* interface_desc =  tlsio_threadx_get_interface_description();

    //assert
    ASSERT_IS_NOT_NULL(interface_desc->concrete_io_retrieveoptions);
    ASSERT_IS_NOT_NULL(interface_desc->concrete_io_create);
    ASSERT_IS_NOT_NULL(interface_desc->concrete_io_destroy);
    ASSERT_IS_NOT_NULL(interface_desc->concrete_io_open);
    ASSERT_IS_NOT_NULL(interface_desc->concrete_io_close);
    ASSERT_IS_NOT_NULL(interface_desc->concrete_io_send);
    ASSERT_IS_NOT_NULL(interface_desc->concrete_io_dowork);
    ASSERT_IS_NOT_NULL(interface_desc->concrete_io_setoption);

    //clean
}


TEST_FUNCTION(tlsio_threadx_setoption_tls_io_NULL_Fail)
{
    //arrange
    umock_c_reset_all_calls();

    //act
    int test_result = tlsio_threadx_setoption(NULL, OPTION_TRUSTED_CERT, TEST_TRUSTED_CERT);

    //assert
    ASSERT_ARE_NOT_EQUAL(int, 0, test_result);

    //clean
}

TEST_FUNCTION(tlsio_threadx_setoption_option_name_NULL_Fail)
{
    //arrange
    TLSIO_CONFIG tls_io_config;
    memset(&tls_io_config, 0, sizeof(tls_io_config));
    tls_io_config.hostname = "global.azure-devices-provisioning.net";
    CONCRETE_IO_HANDLE io_handle = tlsio_threadx_create(&tls_io_config);
    umock_c_reset_all_calls();

    //act
    int test_result = tlsio_threadx_setoption(io_handle, NULL, TEST_TRUSTED_CERT);

    //assert
    ASSERT_ARE_NOT_EQUAL(int, 0, test_result);

    //clean
    tlsio_threadx_destroy(io_handle);
}

TEST_FUNCTION(tlsio_threadx_setoption_trusted_cert_succeed)
{
    //arrange
    TLSIO_CONFIG tls_io_config;
    memset(&tls_io_config, 0, sizeof(tls_io_config));
    tls_io_config.hostname = "global.azure-devices-provisioning.net";
    CONCRETE_IO_HANDLE io_handle = tlsio_threadx_create(&tls_io_config);
    umock_c_reset_all_calls();

    //act
    int test_result = tlsio_threadx_setoption(io_handle, OPTION_TRUSTED_CERT, TEST_TRUSTED_CERT);

    //assert
    ASSERT_ARE_EQUAL(int, 0, test_result);

    //clean
    tlsio_threadx_destroy(io_handle);
}

TEST_FUNCTION(tlsio_threadx_setoption_trusted_cert_twice_succeed)
{
    //arrange
    TLSIO_CONFIG tls_io_config;
    memset(&tls_io_config, 0, sizeof(tls_io_config));
    tls_io_config.hostname = "global.azure-devices-provisioning.net";
    CONCRETE_IO_HANDLE io_handle = tlsio_threadx_create(&tls_io_config);
    umock_c_reset_all_calls();

    //act
    int test_result = tlsio_threadx_setoption(io_handle, OPTION_TRUSTED_CERT, TEST_TRUSTED_CERT);
    ASSERT_ARE_EQUAL(int, 0, test_result);

    test_result = tlsio_threadx_setoption(io_handle, OPTION_TRUSTED_CERT, TEST_TRUSTED_CERT);

    //assert
    ASSERT_ARE_EQUAL(int, 0, test_result);

    //clean
    tlsio_threadx_destroy(io_handle);
}

END_TEST_SUITE(tlsio_threadx_ut)
