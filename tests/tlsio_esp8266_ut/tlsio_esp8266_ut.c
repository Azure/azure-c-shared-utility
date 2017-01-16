// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.


#include <stdlib.h>
#include <stdio.h>
#ifdef _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

void* my_gballoc_malloc(size_t size)
{
	return malloc(size);
}

void* my_gballoc_realloc(void* ptr, size_t size)
{
	return realloc(ptr, size);
}

void my_gballoc_free(void* ptr)
{
	free(ptr);
}

/**
 * Include the C standards here.
 */
#include <stddef.h>
#include <time.h>

/**
 * Include the test tools.
 */
#include "testrunnerswitcher.h"
#include "umock_c.h"
#include "umocktypes_charptr.h"
#include "umock_c_negative_tests.h"
#include "azure_c_shared_utility/macro_utils.h"

/**
 * Include the mockable headers here.
 */
#define ENABLE_MOCKS
#include "../../adapters/esp8266_mock.h"
#undef ENABLE_MOCKS

/**
 * Include the target header after the ENABLE_MOCKS session.
 */
#include "azure_c_shared_utility/xio.h"
#include "azure_c_shared_utility/tlsio.h"


static int g_ssl_write_success = 1;
static int g_ssl_read_returns_data = 1;
static int g_on_bytes_received_buffer_size = 0;


typedef enum TLSIO_STATE_TAG
{
    TLSIO_STATE_NOT_OPEN,
    TLSIO_STATE_OPENING,
    TLSIO_STATE_IN_HANDSHAKE,
    TLSIO_STATE_OPEN,
    TLSIO_STATE_CLOSING,
    TLSIO_STATE_ERROR
} TLSIO_STATE;

typedef struct TLS_IO_INSTANCE_TAG
{
    ON_BYTES_RECEIVED on_bytes_received;
    ON_IO_OPEN_COMPLETE on_io_open_complete;
    ON_IO_CLOSE_COMPLETE on_io_close_complete;
    ON_IO_ERROR on_io_error;
    void* on_bytes_received_context;
    void* on_io_open_complete_context;
    void* on_io_close_complete_context;
    void* on_io_error_context;
    SSL* ssl;
    SSL_CTX* ssl_context;
    TLSIO_STATE tlsio_state;
    char* hostname;
    int port;
    char* certificate;
    const char* x509certificate;
    const char* x509privatekey;
} TLS_IO_INSTANCE;

int real_mallocAndStrcpy_s(char** destination, const char* source);
const IO_INTERFACE_DESCRIPTION* tlsio_openssl_get_interface_description(void);


int my_lwip_select(int maxfdp1, fd_set *readset, fd_set *writeset, fd_set *exceptset,
               struct timeval *timeout){
    return 1;
}

int my_SSL_write(SSL *ssl, const void *buffer, int len){
    if (g_ssl_write_success){
        return len;
    }else{
        return 0;
    }
}

void my_SSL_CTX_free(SSL_CTX *ctx){

}

void my_SSL_free(SSL *ssl){

}

int my_SSL_read(SSL *ssl, void *buffer, int len){
    if (g_ssl_read_returns_data){
        return len;
    }else{
        return 0;
    }
}

int my_SSL_connect(SSL *ssl){
    return 0;
}

int my_SSL_set_fd(SSL *ssl, int fd){
    return 0;
}

SSL* my_SSL_new(SSL_CTX *ssl_ctx){
    SSL* ssl = (SSL*)malloc(sizeof(SSL));
    return ssl;
}

int my_SSL_set_fragment(SSL_CTX *ssl_ctx, unsigned int frag_size){
    return 0;
}

SSL_CTX* my_SSL_CTX_new(SSL_METHOD *method){
    return (SSL_CTX*)malloc(sizeof(SSL_CTX));
}

err_t my_netconn_gethostbyname(const char *name, ip_addr_t *addr){
    return 0;
}

SSL_METHOD* my_TLSv1_client_method(void){
    return NULL;
}

int my_bind(int s, const struct sockaddr* name, socklen_t namelen){
    return 0;
}

int my_connect(int s, const struct sockaddr *name, socklen_t namelen){
    return 0;
}


static void test_on_io_open_complete(void* context, IO_OPEN_RESULT open_result)
{
    (void)context;
    (void)open_result;
}

static void on_bytes_received(void* context, const unsigned char* buffer, size_t size)
{
    g_on_bytes_received_buffer_size = size;
}

#define ENABLE_MOCKS
#include "azure_c_shared_utility/gballoc.h"
#include "azure_c_shared_utility/crt_abstractions.h"
#include "azure_c_shared_utility/optionhandler.h"
#undef ENABLE_MOCKS

 /**
  * You can create some global variables that your test will need in some way.
  */
static void* g_GenericPointer;

/**
  * Umock error will helps you to identify errors in the test suite or in the way that you are 
  *    using it, just keep it as is.
  */
DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    char temp_str[256];
    (void)snprintf(temp_str, sizeof(temp_str), "umock_c reported error :%s", ENUM_TO_STRING(UMOCK_C_ERROR_CODE, error_code));
    ASSERT_FAIL(temp_str);
}

/**
 * This is necessary for the test suite, just keep as is.
 */
static TEST_MUTEX_HANDLE g_testByTest;
static TEST_MUTEX_HANDLE g_dllByDll;

/**
 * Tests begin here. 
 *   
 *   RUN_TEST_SUITE(tlsio_esp8266_ut, failedTestCount);
 *
 */
BEGIN_TEST_SUITE(tlsio_esp8266_ut)

    /**
     * This is the place where we initialize the test system. Replace the test name to associate the test 
     *   suite with your test cases.
     * It is called once, before start the tests.
     */
    TEST_SUITE_INITIALIZE(a)
    {
        int result;
        TEST_INITIALIZE_MEMORY_DEBUG(g_dllByDll);
        g_testByTest = TEST_MUTEX_CREATE();
        ASSERT_IS_NOT_NULL(g_testByTest);

        (void)umock_c_init(on_umock_c_error);

        result = umocktypes_charptr_register_types();
		ASSERT_ARE_EQUAL(int, 0, result);

        /**
         * It is necessary to identify the types defined on your target. With it, the test system will 
         *    know how to use it. 
         *
         * On the target.h example, there is the type TARGET_HANDLE that is a void*
         */
        //REGISTER_UMOCK_ALIAS_TYPE(CALLEE_HANDLE, void*);


        /**
         * Or you can combine, for example, in the success case malloc will call my_gballoc_malloc, and for
         *    the failed cases, it will return NULL.
         */
        REGISTER_GLOBAL_MOCK_HOOK(gballoc_malloc, my_gballoc_malloc);
        REGISTER_GLOBAL_MOCK_FAIL_RETURN(gballoc_malloc, NULL);
        REGISTER_GLOBAL_MOCK_HOOK(gballoc_realloc, my_gballoc_realloc);
        REGISTER_GLOBAL_MOCK_FAIL_RETURN(gballoc_realloc, NULL);
        REGISTER_GLOBAL_MOCK_HOOK(gballoc_free, my_gballoc_free);


        //REGISTER_GLOBAL_MOCK_HOOK(OptionHandler_Create, my_OptionHandler_Create);
        REGISTER_GLOBAL_MOCK_HOOK(mallocAndStrcpy_s, real_mallocAndStrcpy_s);
        REGISTER_GLOBAL_MOCK_HOOK(lwip_select, my_lwip_select);
        REGISTER_GLOBAL_MOCK_HOOK(SSL_write, my_SSL_write);
        REGISTER_GLOBAL_MOCK_HOOK(SSL_CTX_free, my_SSL_CTX_free);
        REGISTER_GLOBAL_MOCK_HOOK(SSL_free, my_SSL_free);
        REGISTER_GLOBAL_MOCK_HOOK(SSL_read, my_SSL_read);
        REGISTER_GLOBAL_MOCK_HOOK(SSL_connect, my_SSL_connect);
        REGISTER_GLOBAL_MOCK_HOOK(SSL_set_fd, my_SSL_set_fd);
        REGISTER_GLOBAL_MOCK_HOOK(SSL_new, my_SSL_new);
        REGISTER_GLOBAL_MOCK_HOOK(SSL_set_fragment, my_SSL_set_fragment);
        REGISTER_GLOBAL_MOCK_HOOK(SSL_CTX_new, my_SSL_CTX_new);
        REGISTER_GLOBAL_MOCK_HOOK(netconn_gethostbyname, my_netconn_gethostbyname);
        REGISTER_GLOBAL_MOCK_HOOK(TLSv1_client_method, my_TLSv1_client_method);
        REGISTER_GLOBAL_MOCK_HOOK(bind, my_bind);
        REGISTER_GLOBAL_MOCK_HOOK(connect, my_connect);

        /**
         * You can initialize other global variables here, for instance image that you have a standard void* that will be converted
         *   any pointer that your test needs.
         */
        g_GenericPointer = malloc(1);
        ASSERT_IS_NOT_NULL(g_GenericPointer);
    }

    /**
     * The test suite will call this function to cleanup your machine.
     * It is called only once, after all tests is done.
     */
    TEST_SUITE_CLEANUP(TestClassCleanup)
    {
        free(g_GenericPointer);

        umock_c_deinit();

        TEST_MUTEX_DESTROY(g_testByTest);
        TEST_DEINITIALIZE_MEMORY_DEBUG(g_dllByDll);
    }

    /**
     * The test suite will call this function to prepare the machine for the new test.
     * It is called before execute each test.
     */
    TEST_FUNCTION_INITIALIZE(initialize)
    {
        if (TEST_MUTEX_ACQUIRE(g_testByTest))
        {
            ASSERT_FAIL("Could not acquire test serialization mutex.");
        }

        umock_c_reset_all_calls();
    }

    /**
     * The test suite will call this function to cleanup your machine for the next test.
     * It is called after execute each test.
     */
    TEST_FUNCTION_CLEANUP(cleans)
    {
        TEST_MUTEX_RELEASE(g_testByTest);
    }

    /* Tests_SRS_TLSIO_SSL_ESP8266_99_019: [ The tlsio_openssl_dowrok succeed]*/
    TEST_FUNCTION(tlsio_openssl_dowork__succeed_with_data)
    {
        ///arrange
        int result = 0;
        g_ssl_read_returns_data = 1;
        TLS_IO_INSTANCE instance;
        instance.on_bytes_received = on_bytes_received;

        const IO_INTERFACE_DESCRIPTION* tlsioInterfaces = tlsio_openssl_get_interface_description();
        ASSERT_IS_NOT_NULL(tlsioInterfaces);

        umock_c_reset_all_calls();
        EXPECTED_CALL(SSL_read(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG));

        ///act
        tlsioInterfaces->concrete_io_dowork(&instance);
        
        ///assert
        ASSERT_ARE_EQUAL(int, result, 0);
        ASSERT_ARE_EQUAL(int, g_on_bytes_received_buffer_size, 64);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ///cleanup
    }

    /* Tests_SRS_TLSIO_SSL_ESP8266_99_019: [ The tlsio_openssl_dowrok succeed]*/
    TEST_FUNCTION(tlsio_openssl_dowork__succeed_without_data)
    {
        ///arrange
        int result = 0;
        g_ssl_read_returns_data = 0;
        TLS_IO_INSTANCE instance;
        instance.on_bytes_received = on_bytes_received;

        const IO_INTERFACE_DESCRIPTION* tlsioInterfaces = tlsio_openssl_get_interface_description();
        ASSERT_IS_NOT_NULL(tlsioInterfaces);

        umock_c_reset_all_calls();
        EXPECTED_CALL(SSL_read(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG));

        ///act
        tlsioInterfaces->concrete_io_dowork(&instance);
        
        ///assert
        ASSERT_ARE_EQUAL(int, result, 0);
        ASSERT_ARE_EQUAL(int, g_on_bytes_received_buffer_size, 0);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ///cleanup
    }


    /* Tests_SRS_TLSIO_SSL_ESP8266_99_018: [ The tlsio_openssl_dowork NULL parameter. No crash when passing NULL]*/
    TEST_FUNCTION(tlsio_openssl_dowork__failed)
    {
        ///arrange
        const IO_INTERFACE_DESCRIPTION* tlsioInterfaces = tlsio_openssl_get_interface_description();
        ASSERT_IS_NOT_NULL(tlsioInterfaces);

        ///act
        tlsioInterfaces->concrete_io_dowork(NULL);

        ///assert
        ///cleanup
    }


    /* Tests_SRS_TLSIO_SSL_ESP8266_99_017: [ The tlsio_openssl_send SSL_write succeed]*/
    TEST_FUNCTION(tlsio_openssl_send__SSL_write_succeed)
    {
        ///arrange
        int result = 0;
        TLS_IO_INSTANCE instance;
        instance.tlsio_state = TLSIO_STATE_OPEN;
        g_ssl_write_success = 1;

        const IO_INTERFACE_DESCRIPTION* tlsioInterfaces = tlsio_openssl_get_interface_description();
        ASSERT_IS_NOT_NULL(tlsioInterfaces);

        ///act
        result = tlsioInterfaces->concrete_io_send(&instance, NULL, 10, NULL, NULL);
        
        ///assert
        ASSERT_ARE_EQUAL(int, result, 0);
        ///cleanup
     }

    /* Tests_SRS_TLSIO_SSL_ESP8266_99_016: [ The tlsio_openssl_send SSL_write failed]*/
    TEST_FUNCTION(tlsio_openssl_send__SSL_write_failed)
    {
        ///arrange
        int result = 0;
        TLS_IO_INSTANCE instance;
        instance.tlsio_state = TLSIO_STATE_OPEN;
        g_ssl_write_success = 0;

        const IO_INTERFACE_DESCRIPTION* tlsioInterfaces = tlsio_openssl_get_interface_description();
        ASSERT_IS_NOT_NULL(tlsioInterfaces);

        ///act
        result = tlsioInterfaces->concrete_io_send(&instance, NULL, 10, NULL, NULL);
        
        ///assert
        ASSERT_ARE_NOT_EQUAL(int, result, 0);
        ///cleanup
    }

    /* Tests_SRS_TLSIO_SSL_ESP8266_99_015: [ The tlsio_openssl_send wrong state.]*/
    TEST_FUNCTION(tlsio_openssl_send_wrong_state__failed)
    {
        ///arrange
        int result;
        const IO_INTERFACE_DESCRIPTION* tlsioInterfaces = tlsio_openssl_get_interface_description();
        ASSERT_IS_NOT_NULL(tlsioInterfaces);

        TLS_IO_INSTANCE instance;
        instance.tlsio_state = TLSIO_STATE_NOT_OPEN;
        ///act
        result = tlsioInterfaces->concrete_io_send(&instance, NULL, 0,  NULL, NULL);

        ///assert
        ASSERT_ARE_NOT_EQUAL(int, 0, result);

        ///cleanup
    }


    /* Tests_SRS_TLSIO_SSL_ESP8266_99_014: [ The tlsio_openssl_send NULL instance.]*/
    TEST_FUNCTION(tlsio_openssl_send__failed)
    {
        ///arrange
        int result;
        const IO_INTERFACE_DESCRIPTION* tlsioInterfaces = tlsio_openssl_get_interface_description();
        ASSERT_IS_NOT_NULL(tlsioInterfaces);

        ///act
        result = tlsioInterfaces->concrete_io_send(NULL, NULL, 0, NULL, NULL);

        ///assert
        ASSERT_ARE_NOT_EQUAL(int, 0, result);

        ///cleanup
    }

    /* Tests_SRS_TLSIO_SSL_ESP8266_99_013: [ The tlsio_openssl_close succeed.]*/
    TEST_FUNCTION(tlsio_openssl_close__succeed)
    {
        ///arrange
        TLS_IO_INSTANCE instance;
        instance.tlsio_state = TLSIO_STATE_OPEN;

        const IO_INTERFACE_DESCRIPTION* tlsioInterfaces = tlsio_openssl_get_interface_description();
        ASSERT_IS_NOT_NULL(tlsioInterfaces);

        umock_c_reset_all_calls();
        STRICT_EXPECTED_CALL(SSL_free(IGNORED_PTR_ARG)).IgnoreArgument(1);
        STRICT_EXPECTED_CALL(SSL_CTX_free(IGNORED_PTR_ARG)).IgnoreArgument(1);

        ///act
        tlsioInterfaces->concrete_io_close(&instance, NULL, NULL);
        
        ///assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ///cleanup
    }


    /* Tests_SRS_TLSIO_SSL_ESP8266_99_012: [ The tlsio_openssl_close wrong state.]*/
    TEST_FUNCTION(tlsio_openssl_close_wrong_state__failed)
    {
        ///arrange
        int result;
        const IO_INTERFACE_DESCRIPTION* tlsioInterfaces = tlsio_openssl_get_interface_description();
        ASSERT_IS_NOT_NULL(tlsioInterfaces);

        TLS_IO_INSTANCE instance;
        instance.tlsio_state = TLSIO_STATE_NOT_OPEN;
        ///act
        result = tlsioInterfaces->concrete_io_close(&instance, NULL, NULL);

        ///assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_ARE_NOT_EQUAL(int, 0, result);

        instance.tlsio_state = TLSIO_STATE_CLOSING;
        ///act
        result = tlsioInterfaces->concrete_io_close(&instance, NULL, NULL);

        ///assert
        ASSERT_ARE_NOT_EQUAL(int, 0, result);
        ///cleanup
    }

    /* Tests_SRS_TLSIO_SSL_ESP8266_99_011: [ The tlsio_openssl_close NULL parameter.]*/
    TEST_FUNCTION(tlsio_openssl_close__failed)
    {
        ///arrange
        int result;
        const IO_INTERFACE_DESCRIPTION* tlsioInterfaces = tlsio_openssl_get_interface_description();
        ASSERT_IS_NOT_NULL(tlsioInterfaces);

        ///act
        result = tlsioInterfaces->concrete_io_close(NULL, NULL, NULL);

        ///assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_ARE_NOT_EQUAL(int, 0, result);

        ///cleanup
    }

    /* Tests_SRS_TLSIO_SSL_ESP8266_99_010: [ The tlsio_openssl_destroy succeed ]*/
    TEST_FUNCTION(tlsio_openssl_destroy__succeed)
    {
        ///arrange
        TLS_IO_INSTANCE* instance = malloc(sizeof(TLS_IO_INSTANCE));
        memset(instance, 0, sizeof(instance));

        const IO_INTERFACE_DESCRIPTION* tlsioInterfaces = tlsio_openssl_get_interface_description();
        ASSERT_IS_NOT_NULL(tlsioInterfaces);

        umock_c_reset_all_calls();
        STRICT_EXPECTED_CALL(free(instance));

        ///act
        tlsioInterfaces->concrete_io_destroy(instance);
        
        ///assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ///cleanup
    }

    /* Tests_SRS_TLSIO_SSL_ESP8266_99_009: [ The tlsio_openssl_destroy NULL parameter. make sure there is no crash ]*/
    TEST_FUNCTION(tlsio_openssl_destroy__failed)
    {
        ///arrange
        const IO_INTERFACE_DESCRIPTION* tlsioInterfaces = tlsio_openssl_get_interface_description();
        ASSERT_IS_NOT_NULL(tlsioInterfaces);

        ///act
        tlsioInterfaces->concrete_io_destroy(NULL);

        ///assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ///cleanup
    }

    /* Tests_SRS_TLSIO_SSL_ESP8266_99_008: [ The tlsio_openssl_open succeed ]*/
    TEST_FUNCTION(tlsio_openssl_open_succeed)
    {
        ///arrange
        TLS_IO_INSTANCE tls_io_instance;
        tls_io_instance.tlsio_state = TLSIO_STATE_NOT_OPEN;
        ip_addr_t addr;

        int result;
        const IO_INTERFACE_DESCRIPTION* tlsioInterfaces = tlsio_openssl_get_interface_description();
        ASSERT_IS_NOT_NULL(tlsioInterfaces);

        umock_c_reset_all_calls();
        
        STRICT_EXPECTED_CALL(netconn_gethostbyname(IGNORED_PTR_ARG,IGNORED_PTR_ARG)).IgnoreArgument(1).IgnoreArgument(2);   
        STRICT_EXPECTED_CALL(bind(IGNORED_NUM_ARG,IGNORED_PTR_ARG, IGNORED_NUM_ARG)).IgnoreArgument(1).IgnoreArgument(2).IgnoreArgument(3);   
        STRICT_EXPECTED_CALL(connect(IGNORED_NUM_ARG,IGNORED_PTR_ARG, IGNORED_NUM_ARG)).IgnoreArgument(1).IgnoreArgument(2).IgnoreArgument(3);
        STRICT_EXPECTED_CALL(lwip_select(IGNORED_NUM_ARG, IGNORED_PTR_ARG, 
             IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG)).IgnoreArgument(1).IgnoreArgument(2).IgnoreArgument(3).IgnoreArgument(4).IgnoreArgument(5);
        STRICT_EXPECTED_CALL(TLSv1_client_method());
        STRICT_EXPECTED_CALL(SSL_CTX_new(IGNORED_PTR_ARG)).IgnoreArgument(1);
        STRICT_EXPECTED_CALL(SSL_set_fragment(IGNORED_PTR_ARG,IGNORED_NUM_ARG)).IgnoreArgument(1).IgnoreArgument(2);
        STRICT_EXPECTED_CALL(SSL_new(IGNORED_PTR_ARG)).IgnoreArgument(1);
        STRICT_EXPECTED_CALL(SSL_set_fd(IGNORED_PTR_ARG, IGNORED_NUM_ARG)).IgnoreArgument(1).IgnoreArgument(2);
        STRICT_EXPECTED_CALL(SSL_connect(IGNORED_PTR_ARG)).IgnoreArgument(1);
        

        ///act
        result = tlsioInterfaces->concrete_io_open(&tls_io_instance, test_on_io_open_complete, NULL, NULL, NULL, NULL, NULL);

        ///assert
        ASSERT_ARE_EQUAL(int, 0, result);
        /**
         * The follow assert will compare the expected calls with the actual calls. If it is different, 
         *    it will show the serialized strings with the differences in the log.
         */
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
    }


    /* Codes_SRS_TLSIO_SSL_ESP8266_99_007: [ The tlsio_openssl_open invalid state. ]*/
    TEST_FUNCTION(tlsio_openssl_open_invalid_state__failed)
    {
        ///arrange
        int result;
        const IO_INTERFACE_DESCRIPTION* tlsioInterfaces = tlsio_openssl_get_interface_description();
        ASSERT_IS_NOT_NULL(tlsioInterfaces);

        TLS_IO_INSTANCE tls_io_instance;
        tls_io_instance.tlsio_state = TLSIO_STATE_OPENING;
        ///act
        result = tlsioInterfaces->concrete_io_open(&tls_io_instance, NULL, NULL, NULL, NULL, NULL, NULL);

        ///assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_ARE_NOT_EQUAL(int, 0, result);
        ///cleanup
    }


    /* Codes_SRS_TLSIO_SSL_ESP8266_99_006: [ The tlsio_openssl_open failed when tls_io is NULL. ]*/
    TEST_FUNCTION(tlsio_openssl_open__failed)
    {
        ///arrange
        int result;
        const IO_INTERFACE_DESCRIPTION* tlsioInterfaces = tlsio_openssl_get_interface_description();
        ASSERT_IS_NOT_NULL(tlsioInterfaces);

        ///act
        result = tlsioInterfaces->concrete_io_open(NULL, NULL, NULL, NULL, NULL, NULL, NULL);

        ///assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_ARE_NOT_EQUAL(int, 0, result);

        ///cleanup
    }

    /* Test_SRS_TLSIO_SSL_ESP8266_99_004: [ The tlsio_openssl_create shall return NULL when malloc fails. ]*/
    TEST_FUNCTION(tlsio_openssl_create_unhappy_paths)
    {
        ///arrange
        int negativeTestsInitResult = umock_c_negative_tests_init();
        ASSERT_ARE_EQUAL(int, 0, negativeTestsInitResult);

        OPTIONHANDLER_HANDLE result;
        const IO_INTERFACE_DESCRIPTION* tlsioInterfaces = tlsio_openssl_get_interface_description();
        ASSERT_IS_NOT_NULL(tlsioInterfaces);

        umock_c_reset_all_calls();

        STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG)).IgnoreArgument(1);
        
        umock_c_negative_tests_snapshot();


        for (size_t i = 0; i < umock_c_negative_tests_call_count(); i++)
        {
            umock_c_negative_tests_reset();
            umock_c_negative_tests_fail_call(i);

            TLSIO_CONFIG tlsio_config;
            char temp_str[128];
            (void)sprintf(temp_str, "On failed call %zu", i);

            printf("i is %d\n", (int)i);
            ///act
            result = tlsioInterfaces->concrete_io_create(&tlsio_config);

            ///assert
            ASSERT_IS_NULL_WITH_MSG(result, temp_str);
        }

        ///cleanup
        umock_c_negative_tests_deinit();   
    }


    /* Tests_SRS_TLSIO_SSL_ESP8266_99_005: [ The tlsio_openssl_create succeed. ]*/
    TEST_FUNCTION(tlsio_openssl_create_succeed)
    {
        ///arrange
        TLSIO_CONFIG tlsio_config;

        OPTIONHANDLER_HANDLE result;
        const IO_INTERFACE_DESCRIPTION* tlsioInterfaces = tlsio_openssl_get_interface_description();
        ASSERT_IS_NOT_NULL(tlsioInterfaces);

        umock_c_reset_all_calls();
        
        //STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG)).IgnoreArgument(1);    
        STRICT_EXPECTED_CALL(gballoc_malloc(sizeof(TLS_IO_INSTANCE)));   
        STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, IGNORED_PTR_ARG)).IgnoreArgument(1).IgnoreArgument(2);  

        ///act
        result = tlsioInterfaces->concrete_io_create(&tlsio_config);

        ///assert
        ASSERT_IS_NOT_NULL(result);
        /**
         * The follow assert will compare the expected calls with the actual calls. If it is different, 
         *    it will show the serialized strings with the differences in the log.
         */
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
    }


    /* Tests_SRS_TLSIO_SSL_ESP8266_99_003: [ The tlsio_openssl_create shall return NULL when io_create_parameters is NULL. ]*/
    TEST_FUNCTION(tlsio_openssl_create__failed)
    {
        ///arrange
        OPTIONHANDLER_HANDLE result;
        const IO_INTERFACE_DESCRIPTION* tlsioInterfaces = tlsio_openssl_get_interface_description();
        ASSERT_IS_NOT_NULL(tlsioInterfaces);

        ///act
        result = tlsioInterfaces->concrete_io_create(NULL);

        ///assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_IS_NULL(result);

        ///cleanup
    }


    /* Tests_SRS_TLSIO_SSL_ESP8266_99_001: [ The tlsio_openssl_retrieveoptions shall not do anything, and return NULL. ]*/
    TEST_FUNCTION(tlsio_openssl_retrieveoptions__succeed)
    {
        ///arrange
        OPTIONHANDLER_HANDLE result;
        const IO_INTERFACE_DESCRIPTION* tlsioInterfaces = tlsio_openssl_get_interface_description();
        ASSERT_IS_NOT_NULL(tlsioInterfaces);

        umock_c_reset_all_calls();

        ///act
        result = tlsioInterfaces->concrete_io_retrieveoptions(NULL);

        ///assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_IS_NULL(result);

        ///cleanup
    }

    /* Tests_SRS_TLSIO_SSL_ESP8266_99_002: [ The tlsio_openssl_setoption shall not do anything, and return 0. ]*/
    TEST_FUNCTION(tlsio_openssl_setoption__succeed)
    {
        ///arrange
        int result;
        const IO_INTERFACE_DESCRIPTION* tlsioInterfaces = tlsio_openssl_get_interface_description();
        ASSERT_IS_NOT_NULL(tlsioInterfaces);

        umock_c_reset_all_calls();

        ///act
        result = tlsioInterfaces->concrete_io_setoption(NULL, NULL, NULL);

        ///assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_ARE_EQUAL(int, 0, result);

        ///cleanup
    }



END_TEST_SUITE(tlsio_esp8266_ut)
