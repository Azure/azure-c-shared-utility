// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "testrunnerswitcher.h"
#include "micromock.h"
#include "micromockcharstararenullterminatedstrings.h"
#include "azure_c_shared_utility/xio.h"
#include "azure_c_shared_utility/optionhandler.h"
#include "azure_c_shared_utility/wsio.h"
#include "azure_c_shared_utility/list.h"
#include "azure_c_shared_utility/lock.h"
#include "libwebsockets.h"
#include "openssl/ssl.h"

extern "C" int gballoc_init(void);
extern "C" void gballoc_deinit(void);
extern "C" void* gballoc_malloc(size_t size);
extern "C" void* gballoc_calloc(size_t nmemb, size_t size);
extern "C" void* gballoc_realloc(void* ptr, size_t size);
extern "C" void gballoc_free(void* ptr);

namespace BASEIMPLEMENTATION
{
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

static const void** list_items = NULL;
static size_t list_item_count = 0;

static const LIST_HANDLE TEST_LIST_HANDLE = (LIST_HANDLE)0x4242;
static const LIST_ITEM_HANDLE TEST_LIST_ITEM_HANDLE = (LIST_ITEM_HANDLE)0x11;
static struct lws_context* TEST_LIBWEBSOCKET_CONTEXT = (struct lws_context*)0x4243;
static void* TEST_USER_CONTEXT = (void*)0x4244;
static struct lws* TEST_LIBWEBSOCKET = (struct lws*)0x4245;
static struct lws_extension* TEST_INTERNAL_EXTENSIONS = (struct lws_extension*)0x4246;
static BIO* TEST_BIO = (BIO*)0x4247;
static BIO_METHOD* TEST_BIO_METHOD = (BIO_METHOD*)0x4248;
static X509_STORE* TEST_CERT_STORE = (X509_STORE*)0x4249;
static X509* TEST_X509_CERT_1 = (X509*)0x424A;
static X509* TEST_X509_CERT_2 = (X509*)0x424B;
static const SSL_CTX* TEST_SSL_CONTEXT = (const SSL_CTX*)0x424C;
static WSIO_CONFIG default_wsio_config = 
{
    "test_host",
    443,
    "test_ws_protocol",
    "a/b/c",
    false,
    "my_trusted_ca_payload"
};
static callback_function* saved_ws_callback;
static void* saved_ws_callback_context;

#define TEST_LOCK_HANDLE (LOCK_HANDLE)0x4443

template<>
bool operator==<const lws_context_creation_info*>(const CMockValue<const lws_context_creation_info*>& lhs, const CMockValue<const lws_context_creation_info*>& rhs)
{
    bool result;

    if (lhs.GetValue() == rhs.GetValue())
    {
        result = true;
    }
    else
    {
        size_t currentProtocol = 0;

        result = lhs.GetValue()->gid == rhs.GetValue()->gid;
        result = result && (lhs.GetValue()->extensions == rhs.GetValue()->extensions);
        result = result && (lhs.GetValue()->token_limits == rhs.GetValue()->token_limits);
        result = result && (lhs.GetValue()->uid == rhs.GetValue()->uid);
        result = result && (lhs.GetValue()->ka_time == rhs.GetValue()->ka_time);
        if (rhs.GetValue()->ka_time != 0)
        {
            result = result && (lhs.GetValue()->ka_interval == rhs.GetValue()->ka_interval);
            result = result && (lhs.GetValue()->ka_probes == rhs.GetValue()->ka_probes);
        }

        result = result && (lhs.GetValue()->options == rhs.GetValue()->options);
        result = result && (lhs.GetValue()->iface == rhs.GetValue()->iface);
        result = result && (lhs.GetValue()->port == rhs.GetValue()->port);
        result = result && (lhs.GetValue()->provided_client_ssl_ctx == rhs.GetValue()->provided_client_ssl_ctx);
        if (rhs.GetValue()->http_proxy_address == NULL)
        {
            result = result && (lhs.GetValue()->http_proxy_address == rhs.GetValue()->http_proxy_address);
        }
        else
        {
            result = result && (strcmp(lhs.GetValue()->http_proxy_address, rhs.GetValue()->http_proxy_address) == 0);
            result = result && (lhs.GetValue()->http_proxy_port == rhs.GetValue()->http_proxy_port);
        }

        if (rhs.GetValue()->ssl_ca_filepath == NULL)
        {
            result = result && (lhs.GetValue()->ssl_ca_filepath == rhs.GetValue()->ssl_ca_filepath);
        }
        else
        {
            result = result && (strcmp(lhs.GetValue()->ssl_ca_filepath, rhs.GetValue()->ssl_ca_filepath) == 0);
        }

        if (rhs.GetValue()->ssl_cert_filepath == NULL)
        {
            result = result && (lhs.GetValue()->ssl_cert_filepath == rhs.GetValue()->ssl_cert_filepath);
        }
        else
        {
            result = result && (strcmp(lhs.GetValue()->ssl_cert_filepath, rhs.GetValue()->ssl_cert_filepath) == 0);
        }

        if (rhs.GetValue()->ssl_cipher_list == NULL)
        {
            result = result && (lhs.GetValue()->ssl_cipher_list == rhs.GetValue()->ssl_cipher_list);
        }
        else
        {
            result = result && (strcmp(lhs.GetValue()->ssl_cipher_list, rhs.GetValue()->ssl_cipher_list) == 0);
        }

        if (rhs.GetValue()->ssl_private_key_filepath == NULL)
        {
            result = result && (lhs.GetValue()->ssl_private_key_filepath == rhs.GetValue()->ssl_private_key_filepath);
        }
        else
        {
            result = result && (strcmp(lhs.GetValue()->ssl_private_key_filepath, rhs.GetValue()->ssl_private_key_filepath) == 0);
        }

        if (rhs.GetValue()->ssl_private_key_password == NULL)
        {
            result = result && (lhs.GetValue()->ssl_private_key_password == rhs.GetValue()->ssl_private_key_password);
        }
        else
        {
            result = result && (strcmp(lhs.GetValue()->ssl_private_key_password, rhs.GetValue()->ssl_private_key_password) == 0);
        }

        while (lhs.GetValue()->protocols[currentProtocol].name != NULL)
        {
            result = result && (lhs.GetValue()->protocols[currentProtocol].id == rhs.GetValue()->protocols[currentProtocol].id);
            result = result && (strcmp(lhs.GetValue()->protocols[currentProtocol].name, rhs.GetValue()->protocols[currentProtocol].name) == 0);
            result = result && (lhs.GetValue()->protocols[currentProtocol].per_session_data_size == rhs.GetValue()->protocols[currentProtocol].per_session_data_size);
            result = result && (lhs.GetValue()->protocols[currentProtocol].rx_buffer_size == rhs.GetValue()->protocols[currentProtocol].rx_buffer_size);
            result = result && (lhs.GetValue()->protocols[currentProtocol].user == rhs.GetValue()->protocols[currentProtocol].user);

            currentProtocol++;
        }

        if (rhs.GetValue()->protocols[currentProtocol].name != NULL)
        {
            result = false;
        }
    }

    return result;
}

template<>
bool operator==<lws_context_creation_info*>(const CMockValue<lws_context_creation_info*>& lhs, const CMockValue<lws_context_creation_info*>& rhs)
{
    const lws_context_creation_info* lhs_value = lhs.GetValue();
    const lws_context_creation_info* rhs_value = rhs.GetValue();

    return CMockValue<const lws_context_creation_info*>(lhs_value) == CMockValue<const lws_context_creation_info*>(rhs_value);
}

static LIST_ITEM_HANDLE add_to_list(const void* item)
{
    const void** items = (const void**)realloc(list_items, (list_item_count + 1) * sizeof(item));
    if (items != NULL)
    {
        list_items = items;
        list_items[list_item_count++] = item;
    }
    return (LIST_ITEM_HANDLE)list_item_count;
}

TYPED_MOCK_CLASS(wsio_mocks, CGlobalMock)
{
public:
    /* Lock mocks */
    MOCK_STATIC_METHOD_0(, LOCK_HANDLE, Lock_Init);
    MOCK_METHOD_END(LOCK_HANDLE, TEST_LOCK_HANDLE);
    MOCK_STATIC_METHOD_1(, LOCK_RESULT, Lock, LOCK_HANDLE, handle);
    MOCK_METHOD_END(LOCK_RESULT, LOCK_OK);
    MOCK_STATIC_METHOD_1(, LOCK_RESULT, Unlock, LOCK_HANDLE, handle);
    MOCK_METHOD_END(LOCK_RESULT, LOCK_OK);
    MOCK_STATIC_METHOD_1(, LOCK_RESULT, Lock_Deinit, LOCK_HANDLE, handle);
    MOCK_METHOD_END(LOCK_RESULT, LOCK_OK);

    /* gballoc mocks */
    MOCK_STATIC_METHOD_1(, void*, gballoc_malloc, size_t, size)
    MOCK_METHOD_END(void*, BASEIMPLEMENTATION::gballoc_malloc(size));

    MOCK_STATIC_METHOD_2(, void*, gballoc_realloc, void*, ptr, size_t, size)
        MOCK_METHOD_END(void*, BASEIMPLEMENTATION::gballoc_realloc(ptr, size));

    MOCK_STATIC_METHOD_1(, void, gballoc_free, void*, ptr)
        BASEIMPLEMENTATION::gballoc_free(ptr);
    MOCK_VOID_METHOD_END()

    // list mocks
    MOCK_STATIC_METHOD_0(, LIST_HANDLE, list_create)
    MOCK_METHOD_END(LIST_HANDLE, TEST_LIST_HANDLE);
    MOCK_STATIC_METHOD_1(, void, list_destroy, LIST_HANDLE, list)
    MOCK_VOID_METHOD_END();
    MOCK_STATIC_METHOD_2(, int, list_remove, LIST_HANDLE, list, LIST_ITEM_HANDLE, item)
        size_t index = (size_t)item - 1;
        (void)memmove(&list_items[index], &list_items[index + 1], sizeof(const void*) * (list_item_count - index - 1));
        list_item_count--;
    MOCK_METHOD_END(int, 0);

    MOCK_STATIC_METHOD_1(, LIST_ITEM_HANDLE, list_get_head_item, LIST_HANDLE, list)
        LIST_ITEM_HANDLE list_item_handle = NULL;
        if (list_item_count > 0)
        {
            list_item_handle = (LIST_ITEM_HANDLE)1;
        }
        else
        {
            list_item_handle = NULL;
        }
    MOCK_METHOD_END(LIST_ITEM_HANDLE, list_item_handle);

    MOCK_STATIC_METHOD_2(, LIST_ITEM_HANDLE, list_add, LIST_HANDLE, list, const void*, item)
    MOCK_METHOD_END(LIST_ITEM_HANDLE, add_to_list(item));
    MOCK_STATIC_METHOD_1(, const void*, list_item_get_value, LIST_ITEM_HANDLE, item_handle)
    MOCK_METHOD_END(const void*, (const void*)list_items[(size_t)item_handle - 1]);
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

    // libwebsockets mocks
    MOCK_STATIC_METHOD_1(, struct lws_context*, lws_create_context, struct lws_context_creation_info*, info)
        saved_ws_callback = info->protocols[0].callback;
        saved_ws_callback_context = info->user;
    MOCK_METHOD_END(struct lws_context*, TEST_LIBWEBSOCKET_CONTEXT)
    MOCK_STATIC_METHOD_1(, void, lws_context_destroy, struct lws_context*, context)
    MOCK_VOID_METHOD_END()
    MOCK_STATIC_METHOD_2(, int, lws_service, struct lws_context*, context, int, timeout_ms)
    MOCK_METHOD_END(int, 0)
    MOCK_STATIC_METHOD_1(, struct lws_context*, lws_get_context, const struct lws*, wsi)
    MOCK_METHOD_END(struct lws_context*, TEST_LIBWEBSOCKET_CONTEXT)
    MOCK_STATIC_METHOD_1(, void*, lws_context_user, struct lws_context*, context)
    MOCK_METHOD_END(void*, TEST_USER_CONTEXT)
    MOCK_STATIC_METHOD_4(, int, lws_write, struct lws*, wsi, unsigned char*, buf, size_t, len, enum lws_write_protocol, protocol)
    MOCK_METHOD_END(int, 0)
    MOCK_STATIC_METHOD_1(, int, lws_callback_on_writable, struct lws*, wsi)
    MOCK_METHOD_END(int, 0)
    MOCK_STATIC_METHOD_9(, struct lws*, lws_client_connect, struct lws_context*, clients, const char*, address, int, port, int, ssl_connection, const char*, path, const char*, host, const char*, origin, const char*, protocol, int, ietf_version_or_minus_one)
    MOCK_METHOD_END(struct lws*, TEST_LIBWEBSOCKET)
    MOCK_STATIC_METHOD_0(, struct lws_extension*, lws_get_internal_extensions)
    MOCK_METHOD_END(struct lws_extension*, TEST_INTERNAL_EXTENSIONS)

    // openssl mocks
    MOCK_STATIC_METHOD_1(, BIO*, BIO_new, BIO_METHOD*, type)
    MOCK_METHOD_END(BIO*, TEST_BIO)
    MOCK_STATIC_METHOD_1(, int, BIO_free, BIO*, a)
    MOCK_METHOD_END(int, 0)
    MOCK_STATIC_METHOD_2(, int, BIO_puts, BIO*, bp, const char*, buf)
    MOCK_METHOD_END(int, 0)
    MOCK_STATIC_METHOD_0(, BIO_METHOD*, BIO_s_mem);
    MOCK_METHOD_END(BIO_METHOD*, TEST_BIO_METHOD)
    MOCK_STATIC_METHOD_1(, X509_STORE*, SSL_CTX_get_cert_store, const SSL_CTX*, ctx);
    MOCK_METHOD_END(X509_STORE*, TEST_CERT_STORE)
    MOCK_STATIC_METHOD_2(, int, X509_STORE_add_cert, X509_STORE*, ctx, X509*, x);
    MOCK_METHOD_END(int, 1)
    MOCK_STATIC_METHOD_4(, X509*, PEM_read_bio_X509, BIO*, bp, X509**, x, pem_password_cb*, cb, void*, u);
    MOCK_METHOD_END(X509*, TEST_X509_CERT_1)
    MOCK_STATIC_METHOD_1(, void, X509_free, X509*, a);
    MOCK_VOID_METHOD_END()

    // consumer mocks
    MOCK_STATIC_METHOD_2(, void, test_on_io_open_complete, void*, context, IO_OPEN_RESULT, io_open_result);
    MOCK_VOID_METHOD_END()
    MOCK_STATIC_METHOD_3(, void, test_on_bytes_received, void*, context, const unsigned char*, buffer, size_t, size);
    MOCK_VOID_METHOD_END()
    MOCK_STATIC_METHOD_1(, void, test_on_io_error, void*, context);
    MOCK_VOID_METHOD_END()
    MOCK_STATIC_METHOD_1(, void, test_on_io_close_complete, void*, context);
    MOCK_VOID_METHOD_END()
    MOCK_STATIC_METHOD_2(, void, test_on_send_complete, void*, context, IO_SEND_RESULT, send_result)
    MOCK_VOID_METHOD_END()

    MOCK_STATIC_METHOD_3(, OPTIONHANDLER_HANDLE, OptionHandler_Create, pfCloneOption, clone, pfDestroyOption, destroy, pfSetOption, setoption);
		OPTIONHANDLER_HANDLE r = (OPTIONHANDLER_HANDLE) malloc(1);
    MOCK_METHOD_END(OPTIONHANDLER_HANDLE, r)
};

extern "C"
{
    DECLARE_GLOBAL_MOCK_METHOD_0(wsio_mocks, , LIST_HANDLE, list_create);
    DECLARE_GLOBAL_MOCK_METHOD_1(wsio_mocks, , void, list_destroy, LIST_HANDLE, list);
    DECLARE_GLOBAL_MOCK_METHOD_1(wsio_mocks, , LIST_ITEM_HANDLE, list_get_head_item, LIST_HANDLE, list);
    DECLARE_GLOBAL_MOCK_METHOD_2(wsio_mocks, , int, list_remove, LIST_HANDLE, list, LIST_ITEM_HANDLE, item);
    DECLARE_GLOBAL_MOCK_METHOD_2(wsio_mocks, , LIST_ITEM_HANDLE, list_add, LIST_HANDLE, list, const void*, item);
    DECLARE_GLOBAL_MOCK_METHOD_1(wsio_mocks, , const void*, list_item_get_value, LIST_ITEM_HANDLE, item_handle);
    DECLARE_GLOBAL_MOCK_METHOD_3(wsio_mocks, , LIST_ITEM_HANDLE, list_find, LIST_HANDLE, handle, LIST_MATCH_FUNCTION, match_function, const void*, match_context);
    DECLARE_GLOBAL_MOCK_METHOD_3(wsio_mocks, , int, list_remove_matching_item, LIST_HANDLE, handle, LIST_MATCH_FUNCTION, match_function, const void*, match_context);

    DECLARE_GLOBAL_MOCK_METHOD_1(wsio_mocks, , struct lws_context*, lws_create_context, struct lws_context_creation_info*, info);
    DECLARE_GLOBAL_MOCK_METHOD_1(wsio_mocks, , void, lws_context_destroy, struct lws_context*, context);
    DECLARE_GLOBAL_MOCK_METHOD_2(wsio_mocks, , int, lws_service, struct lws_context*, context, int, timeout_ms);
    DECLARE_GLOBAL_MOCK_METHOD_1(wsio_mocks, , struct lws_context*, lws_get_context, const struct lws*, wsi);
    DECLARE_GLOBAL_MOCK_METHOD_1(wsio_mocks, , void*, lws_context_user, struct lws_context*, context);
    DECLARE_GLOBAL_MOCK_METHOD_4(wsio_mocks, , int, lws_write, struct lws*, wsi, unsigned char*, buf, size_t, len, enum lws_write_protocol, protocol);
    DECLARE_GLOBAL_MOCK_METHOD_1(wsio_mocks, , int, lws_callback_on_writable, struct lws*, wsi);
    DECLARE_GLOBAL_MOCK_METHOD_9(wsio_mocks, , struct lws*, lws_client_connect, struct lws_context*, clients, const char*, address, int, port, int, ssl_connection, const char*, path, const char*, host, const char*, origin, const char*, protocol, int, ietf_version_or_minus_one);
    DECLARE_GLOBAL_MOCK_METHOD_0(wsio_mocks, , struct lws_extension*, lws_get_internal_extensions);

    DECLARE_GLOBAL_MOCK_METHOD_1(wsio_mocks, , BIO*, BIO_new, BIO_METHOD*, type);
    DECLARE_GLOBAL_MOCK_METHOD_1(wsio_mocks, , int, BIO_free, BIO*, a);
    DECLARE_GLOBAL_MOCK_METHOD_2(wsio_mocks, , int, BIO_puts, BIO*, bp, const char*, buf);
    DECLARE_GLOBAL_MOCK_METHOD_0(wsio_mocks, , BIO_METHOD*, BIO_s_mem);
    DECLARE_GLOBAL_MOCK_METHOD_1(wsio_mocks, , X509_STORE*, SSL_CTX_get_cert_store, const SSL_CTX*, ctx);
    DECLARE_GLOBAL_MOCK_METHOD_2(wsio_mocks, , int, X509_STORE_add_cert, X509_STORE*, ctx, X509*, x);
    DECLARE_GLOBAL_MOCK_METHOD_4(wsio_mocks, , X509*, PEM_read_bio_X509, BIO*, bp, X509**, x, pem_password_cb*, cb, void*, u);
    DECLARE_GLOBAL_MOCK_METHOD_1(wsio_mocks, , void, X509_free, X509*, a);

    DECLARE_GLOBAL_MOCK_METHOD_2(wsio_mocks, , void, test_on_io_open_complete, void*, context, IO_OPEN_RESULT, io_open_result);
    DECLARE_GLOBAL_MOCK_METHOD_3(wsio_mocks, , void, test_on_bytes_received, void*, context, const unsigned char*, buffer, size_t, size);
    DECLARE_GLOBAL_MOCK_METHOD_1(wsio_mocks, , void, test_on_io_error, void*, context);
    DECLARE_GLOBAL_MOCK_METHOD_1(wsio_mocks, , void, test_on_io_close_complete, void*, context);
    DECLARE_GLOBAL_MOCK_METHOD_2(wsio_mocks, , void, test_on_send_complete, void*, context, IO_SEND_RESULT, send_result);

    DECLARE_GLOBAL_MOCK_METHOD_3(wsio_mocks, , OPTIONHANDLER_HANDLE, OptionHandler_Create, pfCloneOption, clone, pfDestroyOption, destroy, pfSetOption, setoption);

    DECLARE_GLOBAL_MOCK_METHOD_1(wsio_mocks, , void*, gballoc_malloc, size_t, size);
    DECLARE_GLOBAL_MOCK_METHOD_2(wsio_mocks, , void*, gballoc_realloc, void*, ptr, size_t, size);
    DECLARE_GLOBAL_MOCK_METHOD_1(wsio_mocks, , void, gballoc_free, void*, ptr)

    DECLARE_GLOBAL_MOCK_METHOD_0(wsio_mocks, , LOCK_HANDLE, Lock_Init);
    DECLARE_GLOBAL_MOCK_METHOD_1(wsio_mocks, , LOCK_RESULT, Lock, LOCK_HANDLE, handle);
    DECLARE_GLOBAL_MOCK_METHOD_1(wsio_mocks, , LOCK_RESULT, Unlock, LOCK_HANDLE, handle);
    DECLARE_GLOBAL_MOCK_METHOD_1(wsio_mocks, , LOCK_RESULT, Lock_Deinit, LOCK_HANDLE, handle);
}

MICROMOCK_MUTEX_HANDLE test_serialize_mutex;

BEGIN_TEST_SUITE(wsio_ut)

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
}

TEST_FUNCTION_CLEANUP(method_cleanup)
{
    free(list_items);
    list_items = NULL;
    list_item_count = 0;
	if (!MicroMockReleaseMutex(test_serialize_mutex))
	{
		ASSERT_FAIL("Could not release test serialization mutex.");
	}
}

/* wsio_create */

/* Tests_SRS_WSIO_01_001: [wsio_create shall create an instance of a wsio and return a non-NULL handle to it.] */
/* Tests_SRS_WSIO_01_098: [wsio_create shall create a pending IO list that is to be used when sending buffers over the libwebsockets IO by calling list_create.] */
/* Tests_SRS_WSIO_01_003: [io_create_parameters shall be used as a WSIO_CONFIG*.] */
/* Tests_SRS_WSIO_01_006: [The members host, protocol_name, relative_path and trusted_ca shall be copied for later use (they are needed when the IO is opened).] */
TEST_FUNCTION(wsio_create_with_valid_args_succeeds)
{
	// arrange
	wsio_mocks mocks;

	EXPECTED_CALL(mocks, gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(mocks, list_create());
    EXPECTED_CALL(mocks, gballoc_malloc(IGNORED_NUM_ARG));
    EXPECTED_CALL(mocks, gballoc_malloc(IGNORED_NUM_ARG));
    EXPECTED_CALL(mocks, gballoc_malloc(IGNORED_NUM_ARG));
    EXPECTED_CALL(mocks, gballoc_malloc(IGNORED_NUM_ARG));
    EXPECTED_CALL(mocks, gballoc_malloc(IGNORED_NUM_ARG));

	// act
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);

	// assert
	ASSERT_IS_NOT_NULL(wsio);
    mocks.AssertActualAndExpectedCalls();

	// cleanup
	wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_002: [If the argument io_create_parameters is NULL then wsio_create shall return NULL.] */
TEST_FUNCTION(wsio_create_with_NULL_io_create_parameters_fails)
{
    // arrange
    wsio_mocks mocks;

    // act
    CONCRETE_IO_HANDLE wsio = wsio_create(NULL);

    // assert
    ASSERT_IS_NULL(wsio);
}

/* Tests_SRS_WSIO_01_004: [If any of the WSIO_CONFIG fields host, protocol_name or relative_path is NULL then wsio_create shall return NULL.] */
TEST_FUNCTION(wsio_create_with_NULL_hostname_fails)
{
    // arrange
    wsio_mocks mocks;
    static WSIO_CONFIG test_wsio_config =
    {
        NULL,
        443,
        "test_ws_protocol",
        "a/b/c",
        false,
        "my_trusted_ca_payload"
    };

    // act
    CONCRETE_IO_HANDLE wsio = wsio_create(&test_wsio_config);

    // assert
    ASSERT_IS_NULL(wsio);
}

/* Tests_SRS_WSIO_01_004: [If any of the WSIO_CONFIG fields host, protocol_name or relative_path is NULL then wsio_create shall return NULL.] */
TEST_FUNCTION(wsio_create_with_NULL_protocol_name_fails)
{
    // arrange
    wsio_mocks mocks;
    static WSIO_CONFIG test_wsio_config =
    {
        "testhost",
        443,
        NULL,
        "a/b/c",
        false,
        "my_trusted_ca_payload"
    };

    // act
    CONCRETE_IO_HANDLE wsio = wsio_create(&test_wsio_config);

    // assert
    ASSERT_IS_NULL(wsio);
}

/* Tests_SRS_WSIO_01_004: [If any of the WSIO_CONFIG fields host, protocol_name or relative_path is NULL then wsio_create shall return NULL.] */
TEST_FUNCTION(wsio_create_with_NULL_relative_path_fails)
{
    // arrange
    wsio_mocks mocks;
    static WSIO_CONFIG test_wsio_config =
    {
        "testhost",
        443,
        "test_ws_protocol",
        NULL,
        false,
        "my_trusted_ca_payload"
    };

    // act
    CONCRETE_IO_HANDLE wsio = wsio_create(&test_wsio_config);

    // assert
    ASSERT_IS_NULL(wsio);
}

/* Tests_SRS_WSIO_01_100: [The trusted_ca member shall be optional (it can be NULL).] */
/* Tests_SRS_WSIO_01_006: [The members host, protocol_name, relative_path and trusted_ca shall be copied for later use (they are needed when the IO is opened).] */
TEST_FUNCTION(wsio_create_with_NULL_trusted_ca_and_the_Rest_of_the_args_valid_succeeds)
{
    // arrange
    wsio_mocks mocks;
    static WSIO_CONFIG test_wsio_config =
    {
        "testhost",
        443,
        "test_ws_protocol",
        "a/b/c",
        false,
        NULL
    };

    EXPECTED_CALL(mocks, gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(mocks, list_create());
    EXPECTED_CALL(mocks, gballoc_malloc(IGNORED_NUM_ARG));
    EXPECTED_CALL(mocks, gballoc_malloc(IGNORED_NUM_ARG));
    EXPECTED_CALL(mocks, gballoc_malloc(IGNORED_NUM_ARG));
    EXPECTED_CALL(mocks, gballoc_malloc(IGNORED_NUM_ARG));

    // act
    CONCRETE_IO_HANDLE wsio = wsio_create(&test_wsio_config);

    // assert
    ASSERT_IS_NOT_NULL(wsio);
    mocks.AssertActualAndExpectedCalls();

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_005: [If allocating memory for the new wsio instance fails then wsio_create shall return NULL.] */
TEST_FUNCTION(when_allocating_memory_for_the_instance_fails_wsio_create_fails)
{
    // arrange
    wsio_mocks mocks;

    EXPECTED_CALL(mocks, gballoc_malloc(IGNORED_NUM_ARG))
        .SetReturn((void*)NULL);

    // act
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);

    // assert
    ASSERT_IS_NULL(wsio);
}

/* Tests_SRS_WSIO_01_099: [If list_create fails then wsio_create shall fail and return NULL.] */
TEST_FUNCTION(when_creating_the_pending_io_list_fails_wsio_create_fails)
{
    // arrange
    wsio_mocks mocks;

    EXPECTED_CALL(mocks, gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(mocks, list_create())
        .SetReturn((LIST_HANDLE)NULL);
    EXPECTED_CALL(mocks, gballoc_free(IGNORED_PTR_ARG));

    // act
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);

    // assert
    ASSERT_IS_NULL(wsio);
}

/* Tests_SRS_WSIO_01_005: [If allocating memory for the new wsio instance fails then wsio_create shall return NULL.] */
TEST_FUNCTION(when_allocating_memory_for_the_host_name_fails_wsio_create_fails)
{
    // arrange
    wsio_mocks mocks;

    EXPECTED_CALL(mocks, gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(mocks, list_create());
    EXPECTED_CALL(mocks, gballoc_malloc(IGNORED_NUM_ARG))
        .SetReturn((LIST_HANDLE)NULL);
    STRICT_EXPECTED_CALL(mocks, list_destroy(TEST_LIST_HANDLE));
    EXPECTED_CALL(mocks, gballoc_free(IGNORED_PTR_ARG));

    // act
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);

    // assert
    ASSERT_IS_NULL(wsio);
}

/* Tests_SRS_WSIO_01_005: [If allocating memory for the new wsio instance fails then wsio_create shall return NULL.] */
TEST_FUNCTION(when_allocating_memory_for_the_protocol_name_fails_wsio_create_fails)
{
    // arrange
    wsio_mocks mocks;

    EXPECTED_CALL(mocks, gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(mocks, list_create());
    EXPECTED_CALL(mocks, gballoc_malloc(IGNORED_NUM_ARG));
    EXPECTED_CALL(mocks, gballoc_malloc(IGNORED_NUM_ARG))
        .SetReturn((LIST_HANDLE)NULL);
    STRICT_EXPECTED_CALL(mocks, list_destroy(TEST_LIST_HANDLE));
    EXPECTED_CALL(mocks, gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(mocks, gballoc_free(IGNORED_PTR_ARG));

    // act
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);

    // assert
    ASSERT_IS_NULL(wsio);
}

/* Tests_SRS_WSIO_01_005: [If allocating memory for the new wsio instance fails then wsio_create shall return NULL.] */
TEST_FUNCTION(when_allocating_memory_for_the_relative_path_fails_wsio_create_fails)
{
    // arrange
    wsio_mocks mocks;

    EXPECTED_CALL(mocks, gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(mocks, list_create());
    EXPECTED_CALL(mocks, gballoc_malloc(IGNORED_NUM_ARG));
    EXPECTED_CALL(mocks, gballoc_malloc(IGNORED_NUM_ARG));
    EXPECTED_CALL(mocks, gballoc_malloc(IGNORED_NUM_ARG))
        .SetReturn((LIST_HANDLE)NULL);
    STRICT_EXPECTED_CALL(mocks, list_destroy(TEST_LIST_HANDLE));
    EXPECTED_CALL(mocks, gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(mocks, gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(mocks, gballoc_free(IGNORED_PTR_ARG));

    // act
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);

    // assert
    ASSERT_IS_NULL(wsio);
}

/* Tests_SRS_WSIO_01_005: [If allocating memory for the new wsio instance fails then wsio_create shall return NULL.] */
TEST_FUNCTION(when_allocating_memory_for_the_protocols_fails_wsio_create_fails)
{
    // arrange
    wsio_mocks mocks;

    EXPECTED_CALL(mocks, gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(mocks, list_create());
    EXPECTED_CALL(mocks, gballoc_malloc(IGNORED_NUM_ARG));
    EXPECTED_CALL(mocks, gballoc_malloc(IGNORED_NUM_ARG));
    EXPECTED_CALL(mocks, gballoc_malloc(IGNORED_NUM_ARG));
    EXPECTED_CALL(mocks, gballoc_malloc(IGNORED_NUM_ARG))
        .SetReturn((LIST_HANDLE)NULL);
    STRICT_EXPECTED_CALL(mocks, list_destroy(TEST_LIST_HANDLE));
    EXPECTED_CALL(mocks, gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(mocks, gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(mocks, gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(mocks, gballoc_free(IGNORED_PTR_ARG));

    // act
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);

    // assert
    ASSERT_IS_NULL(wsio);
}

/* Tests_SRS_WSIO_01_005: [If allocating memory for the new wsio instance fails then wsio_create shall return NULL.] */
TEST_FUNCTION(when_allocating_memory_for_the_trusted_ca_fails_wsio_create_fails)
{
    // arrange
    wsio_mocks mocks;

    EXPECTED_CALL(mocks, gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(mocks, list_create());
    EXPECTED_CALL(mocks, gballoc_malloc(IGNORED_NUM_ARG));
    EXPECTED_CALL(mocks, gballoc_malloc(IGNORED_NUM_ARG));
    EXPECTED_CALL(mocks, gballoc_malloc(IGNORED_NUM_ARG));
    EXPECTED_CALL(mocks, gballoc_malloc(IGNORED_NUM_ARG));
    EXPECTED_CALL(mocks, gballoc_malloc(IGNORED_NUM_ARG))
        .SetReturn((LIST_HANDLE)NULL);
    STRICT_EXPECTED_CALL(mocks, list_destroy(TEST_LIST_HANDLE));
    EXPECTED_CALL(mocks, gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(mocks, gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(mocks, gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(mocks, gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(mocks, gballoc_free(IGNORED_PTR_ARG));

    // act
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);

    // assert
    ASSERT_IS_NULL(wsio);
}

/* wsio_destroy */

/* Tests_SRS_WSIO_01_007: [wsio_destroy shall free all resources associated with the wsio instance.] */
TEST_FUNCTION(when_wsio_destroy_is_called_all_resources_are_freed)
{
    // arrange
    wsio_mocks mocks;
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    mocks.ResetAllCalls();

    STRICT_EXPECTED_CALL(mocks, list_destroy(TEST_LIST_HANDLE));
    EXPECTED_CALL(mocks, gballoc_free(IGNORED_PTR_ARG)); // host_name
    EXPECTED_CALL(mocks, gballoc_free(IGNORED_PTR_ARG)); // relative_path
    EXPECTED_CALL(mocks, gballoc_free(IGNORED_PTR_ARG)); // protocol_name
    EXPECTED_CALL(mocks, gballoc_free(IGNORED_PTR_ARG)); // protocols
    EXPECTED_CALL(mocks, gballoc_free(IGNORED_PTR_ARG)); // trusted_ca
    EXPECTED_CALL(mocks, gballoc_free(IGNORED_PTR_ARG)); // instance

    // act
    wsio_destroy(wsio);

    // assert
    // no explicit assert, uMock checks the calls
}

/* Tests_SRS_WSIO_01_008: [If ws_io is NULL, wsio_destroy shall do nothing.] */
TEST_FUNCTION(wsio_destroy_with_NULL_handle_does_nothing)
{
    // arrange
    wsio_mocks mocks;

    // act
    wsio_destroy(NULL);

    // assert
    // no explicit assert, uMock checks the calls
}

/* Tests_SRS_WSIO_01_009: [wsio_destroy shall execute a close action if the IO has already been open or an open action is already pending.] */
TEST_FUNCTION(wsio_destroy_closes_the_underlying_lws_before_destroying_all_resources)
{
    // arrange
    wsio_mocks mocks;
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    mocks.ResetAllCalls();

    STRICT_EXPECTED_CALL(mocks, test_on_io_open_complete((void*)0x4242, IO_OPEN_CANCELLED));

    STRICT_EXPECTED_CALL(mocks, lws_context_destroy(TEST_LIBWEBSOCKET_CONTEXT));
    STRICT_EXPECTED_CALL(mocks, list_destroy(TEST_LIST_HANDLE));
    EXPECTED_CALL(mocks, gballoc_free(IGNORED_PTR_ARG)); // host_name
    EXPECTED_CALL(mocks, gballoc_free(IGNORED_PTR_ARG)); // relative_path
    EXPECTED_CALL(mocks, gballoc_free(IGNORED_PTR_ARG)); // protocol_name
    EXPECTED_CALL(mocks, gballoc_free(IGNORED_PTR_ARG)); // protocols
    EXPECTED_CALL(mocks, gballoc_free(IGNORED_PTR_ARG)); // trusted_ca
    EXPECTED_CALL(mocks, gballoc_free(IGNORED_PTR_ARG)); // instance

    // act
    wsio_destroy(wsio);

    // assert
    // no explicit assert, uMock checks the calls
}

/* wsio_open */

/* Tests_SRS_WSIO_01_010: [wsio_open shall create a context for the libwebsockets connection by calling lws_create_context.] */
/* Tests_SRS_WSIO_01_011: [The port member of the info argument shall be set to CONTEXT_PORT_NO_LISTEN.] */
/* Tests_SRS_WSIO_01_091: [The extensions field shall be set to the internal extensions obtained by calling lws_get_internal_extensions.] */
/* Tests_SRS_WSIO_01_092: [gid and uid shall be set to -1.] */
/* Tests_SRS_WSIO_01_093: [The members iface, token_limits, ssl_cert_filepath, ssl_private_key_filepath, ssl_private_key_password, ssl_ca_filepath, ssl_cipher_list and provided_client_ssl_ctx shall be set to NULL.] */
/* Tests_SRS_WSIO_01_094: [No proxy support shall be implemented, thus setting http_proxy_address to NULL.] */
/* Tests_SRS_WSIO_01_095: [The member options shall be set to 0.] */
/* Tests_SRS_WSIO_01_096: [The member user shall be set to a user context that will be later passed by the libwebsockets callbacks.] */
/* Tests_SRS_WSIO_01_097: [Keep alive shall not be supported, thus ka_time shall be set to 0.] */
/* Tests_SRS_WSIO_01_012: [The protocols member shall be populated with 2 protocol entries, one containing the actual protocol to be used and one empty (fields shall be NULL or 0).] */
/* Tests_SRS_WSIO_01_013: [callback shall be set to a callback used by the wsio module to listen to libwebsockets events.] */
/* Tests_SRS_WSIO_01_014: [id shall be set to 0] */
/* Tests_SRS_WSIO_01_015: [name shall be set to protocol_name as passed to wsio_create] */
/* Tests_SRS_WSIO_01_016: [per_session_data_size shall be set to 0] */
/* Tests_SRS_WSIO_01_017: [rx_buffer_size shall be set to 0, as there is no need for atomic frames] */
/* Tests_SRS_WSIO_01_019: [user shall be set to NULL] */
/* Tests_SRS_WSIO_01_023: [wsio_open shall trigger the lws connect by calling lws_client_connect and passing to it the following arguments] */
/* Tests_SRS_WSIO_01_024: [clients shall be the context created earlier in wsio_open] */
/* Tests_SRS_WSIO_01_025: [address shall be the hostname passed to wsio_create] */
/* Tests_SRS_WSIO_01_026: [port shall be the port passed to wsio_create] */
/* Tests_SRS_WSIO_01_103: [otherwise it shall be 0.] */
/* Tests_SRS_WSIO_01_028: [path shall be the relative_path passed in wsio_create] */
/* Tests_SRS_WSIO_01_029: [host shall be the host passed to wsio_create] */
/* Tests_SRS_WSIO_01_030: [origin shall be the host passed to wsio_create] */
/* Tests_SRS_WSIO_01_031: [protocol shall be the protocol_name passed to wsio_create] */
/* Tests_SRS_WSIO_01_032: [ietf_version_or_minus_one shall be -1] */
/* Tests_SRS_WSIO_01_104: [On success, wsio_open shall return 0.] */
TEST_FUNCTION(wsio_open_with_proper_arguments_succeeds)
{
    // arrange
    wsio_mocks mocks;
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    mocks.ResetAllCalls();

    lws_context_creation_info lws_context_info;
    lws_protocols protocols[] = 
    {
        { default_wsio_config.protocol_name, NULL, 0, 0, 0, NULL },
        { NULL, NULL, 0, 0, 0, NULL }
    };

    lws_context_info.port = CONTEXT_PORT_NO_LISTEN;
    lws_context_info.extensions = TEST_INTERNAL_EXTENSIONS;
    lws_context_info.gid = -1;
    lws_context_info.uid = -1;
    lws_context_info.iface = NULL;
    lws_context_info.token_limits = NULL;
    lws_context_info.ssl_cert_filepath = NULL;
    lws_context_info.ssl_private_key_filepath = NULL;
    lws_context_info.ssl_private_key_password = NULL;
    lws_context_info.ssl_ca_filepath = NULL;
    lws_context_info.ssl_cipher_list = NULL;
    lws_context_info.provided_client_ssl_ctx = NULL;
    lws_context_info.http_proxy_address = NULL;
    lws_context_info.options = 0;
    lws_context_info.ka_time = 0;

    lws_context_info.protocols = protocols;

    STRICT_EXPECTED_CALL(mocks, lws_get_internal_extensions());
    STRICT_EXPECTED_CALL(mocks, lws_create_context(&lws_context_info));
    STRICT_EXPECTED_CALL(mocks, lws_client_connect(TEST_LIBWEBSOCKET_CONTEXT, default_wsio_config.host, default_wsio_config.port, 0, default_wsio_config.relative_path, default_wsio_config.host, default_wsio_config.host, default_wsio_config.protocol_name, -1));

    // act
    int result = wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    mocks.AssertActualAndExpectedCalls();
    
    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_015: [name shall be set to protocol_name as passed to wsio_create] */
/* Tests_SRS_WSIO_01_025: [address shall be the hostname passed to wsio_create] */
/* Tests_SRS_WSIO_01_026: [port shall be the port passed to wsio_create] */
/* Tests_SRS_WSIO_01_027: [if use_ssl passed in wsio_create is true, the use_ssl argument shall be 1] */
/* Tests_SRS_WSIO_01_028: [path shall be the relative_path passed in wsio_create] */
/* Tests_SRS_WSIO_01_029: [host shall be the host passed to wsio_create] */
/* Tests_SRS_WSIO_01_030: [origin shall be the host passed to wsio_create] */
/* Tests_SRS_WSIO_01_031: [protocol shall be the protocol_name passed to wsio_create] */
/* Tests_SRS_WSIO_01_091: [The extensions field shall be set to the internal extensions obtained by calling lws_get_internal_extensions.] */
/* Tests_SRS_WSIO_01_104: [On success, wsio_open shall return 0.] */
TEST_FUNCTION(wsio_open_with_different_config_succeeds)
{
    // arrange
    wsio_mocks mocks;
    static WSIO_CONFIG wsio_config =
    {
        "hagauaga",
        1234,
        "another_proto",
        "d1/e2/f3",
        true,
        "booohoo"
    };
    CONCRETE_IO_HANDLE wsio = wsio_create(&wsio_config);
    mocks.ResetAllCalls();

    lws_context_creation_info lws_context_info;
    lws_protocols protocols[] =
    {
        { wsio_config.protocol_name, NULL, 0, 0, 0, NULL },
        { NULL, NULL, 0, 0, 0, NULL }
    };

    lws_context_info.port = CONTEXT_PORT_NO_LISTEN;
    lws_context_info.extensions = (lws_extension*)NULL;
    lws_context_info.gid = -1;
    lws_context_info.uid = -1;
    lws_context_info.iface = NULL;
    lws_context_info.token_limits = NULL;
    lws_context_info.ssl_cert_filepath = NULL;
    lws_context_info.ssl_private_key_filepath = NULL;
    lws_context_info.ssl_private_key_password = NULL;
    lws_context_info.ssl_ca_filepath = NULL;
    lws_context_info.ssl_cipher_list = NULL;
    lws_context_info.provided_client_ssl_ctx = NULL;
    lws_context_info.http_proxy_address = NULL;
    lws_context_info.options = 0;
    lws_context_info.ka_time = 0;

    lws_context_info.protocols = protocols;

    STRICT_EXPECTED_CALL(mocks, lws_get_internal_extensions())
        .SetReturn((lws_extension*)NULL);
    STRICT_EXPECTED_CALL(mocks, lws_create_context(&lws_context_info));
    STRICT_EXPECTED_CALL(mocks, lws_client_connect(TEST_LIBWEBSOCKET_CONTEXT, wsio_config.host, wsio_config.port, 1, wsio_config.relative_path, wsio_config.host, wsio_config.host, wsio_config.protocol_name, -1));

    // act
    int result = wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    mocks.AssertActualAndExpectedCalls();

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_022: [If creating the context fails then wsio_open shall fail and return a non-zero value.] */
TEST_FUNCTION(when_creating_the_libwebsockets_context_fails_then_wsio_open_fails)
{
    // arrange
    wsio_mocks mocks;
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    mocks.ResetAllCalls();

    lws_context_creation_info lws_context_info;
    lws_protocols protocols[] =
    {
        { default_wsio_config.protocol_name, NULL, 0, 0, 0, NULL },
        { NULL, NULL, 0, 0, 0, NULL }
    };

    lws_context_info.port = CONTEXT_PORT_NO_LISTEN;
    lws_context_info.extensions = TEST_INTERNAL_EXTENSIONS;
    lws_context_info.gid = -1;
    lws_context_info.uid = -1;
    lws_context_info.iface = NULL;
    lws_context_info.token_limits = NULL;
    lws_context_info.ssl_cert_filepath = NULL;
    lws_context_info.ssl_private_key_filepath = NULL;
    lws_context_info.ssl_private_key_password = NULL;
    lws_context_info.ssl_ca_filepath = NULL;
    lws_context_info.ssl_cipher_list = NULL;
    lws_context_info.provided_client_ssl_ctx = NULL;
    lws_context_info.http_proxy_address = NULL;
    lws_context_info.options = 0;
    lws_context_info.ka_time = 0;

    lws_context_info.protocols = protocols;

    STRICT_EXPECTED_CALL(mocks, lws_get_internal_extensions());
    STRICT_EXPECTED_CALL(mocks, lws_create_context(&lws_context_info))
        .SetReturn((lws_context*)NULL);

    // act
    int result = wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    mocks.AssertActualAndExpectedCalls();

    ///cleanup 
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_033: [If lws_client_connect fails then wsio_open shall fail and return a non-zero value.] */
TEST_FUNCTION(when_lws_client_connect_fails_then_wsio_open_fails)
{
    // arrange
    wsio_mocks mocks;
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    mocks.ResetAllCalls();

    lws_context_creation_info lws_context_info;
    lws_protocols protocols[] =
    {
        { default_wsio_config.protocol_name, NULL, 0, 0, 0, NULL },
        { NULL, NULL, 0, 0, 0, NULL }
    };

    lws_context_info.port = CONTEXT_PORT_NO_LISTEN;
    lws_context_info.extensions = TEST_INTERNAL_EXTENSIONS;
    lws_context_info.gid = -1;
    lws_context_info.uid = -1;
    lws_context_info.iface = NULL;
    lws_context_info.token_limits = NULL;
    lws_context_info.ssl_cert_filepath = NULL;
    lws_context_info.ssl_private_key_filepath = NULL;
    lws_context_info.ssl_private_key_password = NULL;
    lws_context_info.ssl_ca_filepath = NULL;
    lws_context_info.ssl_cipher_list = NULL;
    lws_context_info.provided_client_ssl_ctx = NULL;
    lws_context_info.http_proxy_address = NULL;
    lws_context_info.options = 0;
    lws_context_info.ka_time = 0;

    lws_context_info.protocols = protocols;

    STRICT_EXPECTED_CALL(mocks, lws_get_internal_extensions());
    STRICT_EXPECTED_CALL(mocks, lws_create_context(&lws_context_info));
    STRICT_EXPECTED_CALL(mocks, lws_client_connect(TEST_LIBWEBSOCKET_CONTEXT, default_wsio_config.host, default_wsio_config.port, 0, default_wsio_config.relative_path, default_wsio_config.host, default_wsio_config.host, default_wsio_config.protocol_name, -1))
        .SetReturn((lws*)NULL);
    STRICT_EXPECTED_CALL(mocks, lws_context_destroy(TEST_LIBWEBSOCKET_CONTEXT));

    // act
    int result = wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    mocks.AssertActualAndExpectedCalls();

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);

    ///cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_034: [If another open is in progress or has completed successfully (the IO is open), wsio_open shall fail and return a non-zero value without performing any connection related activities.] */
TEST_FUNCTION(a_second_wsio_open_while_opening_fails)
{
    // arrange
    wsio_mocks mocks;
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    mocks.ResetAllCalls();

    // act
    int result = wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    mocks.AssertActualAndExpectedCalls();

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_036: [The callback on_io_open_complete shall be called with io_open_result being set to IO_OPEN_OK when the open action is succesfull.] */
/* Tests_SRS_WSIO_01_039: [The callback_context argument shall be passed to on_io_open_complete as is.] */
TEST_FUNCTION(when_ws_callback_indicates_a_connect_complete_then_the_on_open_complete_callback_is_triggered)
{
    // arrange
    wsio_mocks mocks;
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    mocks.ResetAllCalls();

    STRICT_EXPECTED_CALL(mocks, lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(mocks, lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    STRICT_EXPECTED_CALL(mocks, test_on_io_open_complete((void*)0x4242, IO_OPEN_OK));

    // act
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_ESTABLISHED, NULL, NULL, 0);

    // assert
    mocks.AssertActualAndExpectedCalls();

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_037: [If any error occurs while the open action is in progress, the callback on_io_open_complete shall be called with io_open_result being set to IO_OPEN_ERROR.] */
/* Tests_SRS_WSIO_01_039: [The callback_context argument shall be passed to on_io_open_complete as is.] */
TEST_FUNCTION(when_ws_callback_indicates_a_connection_error_then_the_on_open_complete_callback_is_triggered_with_error)
{
    // arrange
    wsio_mocks mocks;
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    mocks.ResetAllCalls();

    STRICT_EXPECTED_CALL(mocks, lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(mocks, lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    STRICT_EXPECTED_CALL(mocks, test_on_io_open_complete((void*)0x4242, IO_OPEN_ERROR));
    STRICT_EXPECTED_CALL(mocks, lws_context_destroy(TEST_LIBWEBSOCKET_CONTEXT));

    // act
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_CONNECTION_ERROR, NULL, NULL, 0);

    // assert
    mocks.AssertActualAndExpectedCalls();

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_038: [If wsio_close is called while the open action is in progress, the callback on_io_open_complete shall be called with io_open_result being set to IO_OPEN_CANCELLED and then the wsio_close shall proceed to close the IO.] */
/* Tests_SRS_WSIO_01_039: [The callback_context argument shall be passed to on_io_open_complete as is.] */
TEST_FUNCTION(when_wsio_close_is_called_while_an_open_action_is_in_progress_the_on_io_open_complete_callback_is_triggered_with_cancelled)
{
    // arrange
    wsio_mocks mocks;
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    mocks.ResetAllCalls();

    STRICT_EXPECTED_CALL(mocks, test_on_io_open_complete((void*)0x4242, IO_OPEN_CANCELLED));
    STRICT_EXPECTED_CALL(mocks, lws_context_destroy(TEST_LIBWEBSOCKET_CONTEXT));

    // act
    wsio_close(wsio, NULL, NULL);

    // assert
    mocks.AssertActualAndExpectedCalls();

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_039: [The callback_context argument shall be passed to on_io_open_complete as is.] */
TEST_FUNCTION(when_ws_callback_indicates_a_connection_error_then_the_on_open_complete_callback_is_triggered_with_error_and_NULL_callback_context)
{
    // arrange
    wsio_mocks mocks;
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, NULL, test_on_bytes_received, NULL, test_on_io_error, NULL);
    mocks.ResetAllCalls();

    STRICT_EXPECTED_CALL(mocks, lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(mocks, lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    STRICT_EXPECTED_CALL(mocks, test_on_io_open_complete(NULL, IO_OPEN_ERROR));
    STRICT_EXPECTED_CALL(mocks, lws_context_destroy(TEST_LIBWEBSOCKET_CONTEXT));

    // act
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_CONNECTION_ERROR, NULL, NULL, 0);

    // assert
    mocks.AssertActualAndExpectedCalls();

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_039: [The callback_context argument shall be passed to on_io_open_complete as is.] */
TEST_FUNCTION(when_ws_callback_indicates_a_connect_complete_then_the_on_open_complete_callback_is_triggered_with_NULL_callback_context)
{
    // arrange
    wsio_mocks mocks;
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, NULL, test_on_bytes_received, NULL, test_on_io_error, NULL);
    mocks.ResetAllCalls();

    STRICT_EXPECTED_CALL(mocks, lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(mocks, lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    STRICT_EXPECTED_CALL(mocks, test_on_io_open_complete(NULL, IO_OPEN_OK));

    // act
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_ESTABLISHED, NULL, NULL, 0);

    // assert
    mocks.AssertActualAndExpectedCalls();

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_040: [The argument on_io_open_complete shall be optional, if NULL is passed by the caller then no open complete callback shall be triggered.] */
TEST_FUNCTION(when_ws_callback_indicates_a_connect_complete_and_no_on_open_complete_callback_was_supplied_no_callback_is_triggered)
{
    // arrange
    wsio_mocks mocks;
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, NULL, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    mocks.ResetAllCalls();

    STRICT_EXPECTED_CALL(mocks, lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(mocks, lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);

    // act
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_ESTABLISHED, NULL, NULL, 0);

    // assert
    mocks.AssertActualAndExpectedCalls();

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_040: [The argument on_io_open_complete shall be optional, if NULL is passed by the caller then no open complete callback shall be triggered.] */
TEST_FUNCTION(when_ws_callback_indicates_a_connect_error_and_no_on_open_complete_callback_was_supplied_no_callback_is_triggered)
{
    // arrange
    wsio_mocks mocks;
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, NULL, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    mocks.ResetAllCalls();

    STRICT_EXPECTED_CALL(mocks, lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(mocks, lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    STRICT_EXPECTED_CALL(mocks, lws_context_destroy(TEST_LIBWEBSOCKET_CONTEXT));

    // act
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_CONNECTION_ERROR, NULL, NULL, 0);

    // assert
    mocks.AssertActualAndExpectedCalls();

    // cleanup
    wsio_destroy(wsio);
}

/* wsio_close */

/* Tests_SRS_WSIO_01_041: [wsio_close shall close the websockets IO if an open action is either pending or has completed successfully (if the IO is open).] */
/* Tests_SRS_WSIO_01_049: [The argument on_io_close_complete shall be optional, if NULL is passed by the caller then no close complete callback shall be triggered.] */
/* Tests_SRS_WSIO_01_043: [wsio_close shall close the connection by calling lws_context_destroy.] */
/* Tests_SRS_WSIO_01_044: [On success wsio_close shall return 0.] */
TEST_FUNCTION(wsio_close_destroys_the_ws_context)
{
    // arrange
    wsio_mocks mocks;
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    mocks.ResetAllCalls();

    STRICT_EXPECTED_CALL(mocks, test_on_io_open_complete((void*)0x4242, IO_OPEN_CANCELLED));
    STRICT_EXPECTED_CALL(mocks, lws_context_destroy(TEST_LIBWEBSOCKET_CONTEXT));

    // act
    int result = wsio_close(wsio, NULL, NULL);

    // assert
    mocks.AssertActualAndExpectedCalls();
    ASSERT_ARE_EQUAL(int, 0, result);

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_047: [The callback on_io_close_complete shall be called after the close action has been completed in the context of wsio_close (wsio_close is effectively blocking).] */
/* Tests_SRS_WSIO_01_048: [The callback_context argument shall be passed to on_io_close_complete as is.] */
TEST_FUNCTION(wsio_close_destroys_the_ws_context_and_calls_the_io_close_complete_callback)
{
    // arrange
    wsio_mocks mocks;
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    mocks.ResetAllCalls();

    STRICT_EXPECTED_CALL(mocks, test_on_io_open_complete((void*)0x4242, IO_OPEN_CANCELLED));
    STRICT_EXPECTED_CALL(mocks, lws_context_destroy(TEST_LIBWEBSOCKET_CONTEXT));
    STRICT_EXPECTED_CALL(mocks, test_on_io_close_complete((void*)0x4243));

    // act
    int result = wsio_close(wsio, test_on_io_close_complete, (void*)0x4243);

    // assert
    mocks.AssertActualAndExpectedCalls();
    ASSERT_ARE_EQUAL(int, 0, result);

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_047: [The callback on_io_close_complete shall be called after the close action has been completed in the context of wsio_close (wsio_close is effectively blocking).] */
/* Tests_SRS_WSIO_01_048: [The callback_context argument shall be passed to on_io_close_complete as is.] */
/* Tests_SRS_WSIO_01_108: [wsio_close shall obtain all the pending IO items by repetitively querying for the head of the pending IO list and freeing that head item.] */
/* Tests_SRS_WSIO_01_111: [Obtaining the head of the pending IO list shall be done by calling list_get_head_item.] */
TEST_FUNCTION(wsio_close_after_ws_connected_calls_the_io_close_complete_callback)
{
    // arrange
    wsio_mocks mocks;
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    STRICT_EXPECTED_CALL(mocks, lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_ESTABLISHED, saved_ws_callback_context, NULL, 0);
    mocks.ResetAllCalls();

    STRICT_EXPECTED_CALL(mocks, list_get_head_item(TEST_LIST_HANDLE));
    STRICT_EXPECTED_CALL(mocks, lws_context_destroy(TEST_LIBWEBSOCKET_CONTEXT));
    STRICT_EXPECTED_CALL(mocks, test_on_io_close_complete((void*)0x4243));

    // act
    int result = wsio_close(wsio, test_on_io_close_complete, (void*)0x4243);

    // assert
    mocks.AssertActualAndExpectedCalls();
    ASSERT_ARE_EQUAL(int, 0, result);

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_042: [if ws_io is NULL, wsio_close shall return a non-zero value.] */
TEST_FUNCTION(wsio_close_with_NULL_handle_fails)
{
    // arrange
    wsio_mocks mocks;

    // act
    int result = wsio_close(NULL, test_on_io_close_complete, (void*)0x4242);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
}

/* Tests_SRS_WSIO_01_045: [wsio_close when no open action has been issued shall fail and return a non-zero value.] */
TEST_FUNCTION(wsio_close_when_not_open_fails)
{
    // arrange
    wsio_mocks mocks;
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    mocks.ResetAllCalls();

    // act
    int result = wsio_close(wsio, test_on_io_close_complete, (void*)0x4242);

    // assert
    mocks.AssertActualAndExpectedCalls();
    ASSERT_ARE_NOT_EQUAL(int, 0, result);

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_046: [wsio_close after a wsio_close shall fail and return a non-zero value.]  */
TEST_FUNCTION(wsio_close_when_already_closed_fails)
{
    // arrange
    wsio_mocks mocks;
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    (void)wsio_close(wsio, test_on_io_close_complete, (void*)0x4242);
    mocks.ResetAllCalls();

    // act
    int result = wsio_close(wsio, test_on_io_close_complete, (void*)0x4242);

    // assert
    mocks.AssertActualAndExpectedCalls();
    ASSERT_ARE_NOT_EQUAL(int, 0, result);

    // cleanup
    wsio_destroy(wsio);
}

/* wsio_send */

/* Tests_SRS_WSIO_01_050: [wsio_send shall send the buffer bytes through the websockets connection.] */
/* Tests_SRS_WSIO_01_054: [wsio_send shall queue the buffer and size until the libwebsockets callback is invoked with the event LWS_CALLBACK_CLIENT_WRITEABLE.] */
/* Tests_SRS_WSIO_01_105: [The data and callback shall be queued by calling list_add on the list created in wsio_create.] */
/* Tests_SRS_WSIO_01_056: [After queueing the data, wsio_send shall call lws_callback_on_writable, while passing as arguments the websockets instance previously obtained in wsio_open from lws_client_connect.] */
/* Tests_SRS_WSIO_01_107: [On success, wsio_send shall return 0.] */
TEST_FUNCTION(wsio_send_adds_the_bytes_to_the_list_and_triggers_on_writable_when_the_io_is_open)
{
    // arrange
    wsio_mocks mocks;
    unsigned char test_buffer[] = { 0x42 };
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    STRICT_EXPECTED_CALL(mocks, lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_ESTABLISHED, saved_ws_callback_context, NULL, 0);
    mocks.ResetAllCalls();

    EXPECTED_CALL(mocks, gballoc_malloc(IGNORED_NUM_ARG));
    EXPECTED_CALL(mocks, gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(mocks, list_add(TEST_LIST_HANDLE, IGNORED_PTR_ARG))
        .IgnoreArgument(2);
    STRICT_EXPECTED_CALL(mocks, lws_callback_on_writable(TEST_LIBWEBSOCKET));

    // act
    int result = wsio_send(wsio, test_buffer, sizeof(test_buffer), test_on_send_complete, (void*)0x4243);

    // assert
    mocks.AssertActualAndExpectedCalls();
    ASSERT_ARE_EQUAL(int, 0, result);

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_055: [If queueing the data fails (i.e. due to insufficient memory), wsio_send shall fail and return a non-zero value.] */
TEST_FUNCTION(when_allocating_memory_for_the_list_item_fails_wsio_send_fails)
{
    // arrange
    wsio_mocks mocks;
    unsigned char test_buffer[] = { 0x42 };
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    STRICT_EXPECTED_CALL(mocks, lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_ESTABLISHED, saved_ws_callback_context, NULL, 0);
    mocks.ResetAllCalls();

    EXPECTED_CALL(mocks, gballoc_malloc(IGNORED_NUM_ARG))
        .SetReturn((void*)NULL);

    // act
    int result = wsio_send(wsio, test_buffer, sizeof(test_buffer), test_on_send_complete, (void*)0x4243);

    // assert
    mocks.AssertActualAndExpectedCalls();
    ASSERT_ARE_NOT_EQUAL(int, 0, result);

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_055: [If queueing the data fails (i.e. due to insufficient memory), wsio_send shall fail and return a non-zero value.] */
TEST_FUNCTION(when_allocating_memory_for_the_pending_bytes_fails_wsio_send_fails)
{
    // arrange
    wsio_mocks mocks;
    unsigned char test_buffer[] = { 0x42 };
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    STRICT_EXPECTED_CALL(mocks, lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_ESTABLISHED, saved_ws_callback_context, NULL, 0);
    mocks.ResetAllCalls();

    EXPECTED_CALL(mocks, gballoc_malloc(IGNORED_NUM_ARG));
    EXPECTED_CALL(mocks, gballoc_malloc(IGNORED_NUM_ARG))
        .SetReturn((void*)NULL);
    EXPECTED_CALL(mocks, gballoc_free(IGNORED_PTR_ARG));

    // act
    int result = wsio_send(wsio, test_buffer, sizeof(test_buffer), test_on_send_complete, (void*)0x4243);

    // assert
    mocks.AssertActualAndExpectedCalls();
    ASSERT_ARE_NOT_EQUAL(int, 0, result);

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_055: [If queueing the data fails (i.e. due to insufficient memory), wsio_send shall fail and return a non-zero value.] */
TEST_FUNCTION(when_list_add_fails_wsio_send_fails)
{
    // arrange
    wsio_mocks mocks;
    unsigned char test_buffer[] = { 0x42 };
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    STRICT_EXPECTED_CALL(mocks, lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_ESTABLISHED, saved_ws_callback_context, NULL, 0);
    mocks.ResetAllCalls();

    EXPECTED_CALL(mocks, gballoc_malloc(IGNORED_NUM_ARG));
    EXPECTED_CALL(mocks, gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(mocks, list_add(TEST_LIST_HANDLE, IGNORED_PTR_ARG))
        .IgnoreArgument(2)
        .SetReturn((LIST_ITEM_HANDLE)NULL);
    EXPECTED_CALL(mocks, gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(mocks, gballoc_free(IGNORED_PTR_ARG));

    // act
    int result = wsio_send(wsio, test_buffer, sizeof(test_buffer), test_on_send_complete, (void*)0x4243);

    // assert
    mocks.AssertActualAndExpectedCalls();
    ASSERT_ARE_NOT_EQUAL(int, 0, result);

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_106: [If lws_callback_on_writable returns a negative value, wsio_send shall fail and return a non-zero value.] */
TEST_FUNCTION(when_callback_on_writable_fails_then_wsio_send_fails)
{
    // arrange
    wsio_mocks mocks;
    unsigned char test_buffer[] = { 0x42 };
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    STRICT_EXPECTED_CALL(mocks, lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_ESTABLISHED, saved_ws_callback_context, NULL, 0);
    mocks.ResetAllCalls();

    EXPECTED_CALL(mocks, gballoc_malloc(IGNORED_NUM_ARG));
    EXPECTED_CALL(mocks, gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(mocks, list_add(TEST_LIST_HANDLE, IGNORED_PTR_ARG))
        .IgnoreArgument(2);
    STRICT_EXPECTED_CALL(mocks, lws_callback_on_writable(TEST_LIBWEBSOCKET))
        .SetReturn(-1);

    // act
    int result = wsio_send(wsio, test_buffer, sizeof(test_buffer), test_on_send_complete, (void*)0x4243);

    // assert
    mocks.AssertActualAndExpectedCalls();
    ASSERT_ARE_NOT_EQUAL(int, 0, result);

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_050: [wsio_send shall send the buffer bytes through the websockets connection.] */
/* Tests_SRS_WSIO_01_054: [wsio_send shall queue the buffer and size until the libwebsockets callback is invoked with the event LWS_CALLBACK_CLIENT_WRITEABLE.] */
/* Tests_SRS_WSIO_01_057: [The callback on_send_complete shall be called with SEND_RESULT_OK when the send is indicated as complete.] */
TEST_FUNCTION(when_lws_wants_to_send_bytes_they_are_pushed_to_lws)
{
    // arrange
    wsio_mocks mocks;
    unsigned char test_buffer[] = { 0x42 };
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    STRICT_EXPECTED_CALL(mocks, lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_ESTABLISHED, saved_ws_callback_context, NULL, 0);
    (void)wsio_send(wsio, test_buffer, sizeof(test_buffer), test_on_send_complete, (void*)0x4243);
    mocks.ResetAllCalls();

    STRICT_EXPECTED_CALL(mocks, lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(mocks, lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    STRICT_EXPECTED_CALL(mocks, list_get_head_item(TEST_LIST_HANDLE));
    EXPECTED_CALL(mocks, list_item_get_value(IGNORED_PTR_ARG));
    EXPECTED_CALL(mocks, gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(mocks, lws_write(TEST_LIBWEBSOCKET, IGNORED_PTR_ARG, sizeof(test_buffer), LWS_WRITE_BINARY))
        .ValidateArgumentBuffer(2, test_buffer, sizeof(test_buffer))
        .SetReturn((int)sizeof(test_buffer));
    EXPECTED_CALL(mocks, gballoc_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(mocks, test_on_send_complete((void*)0x4243, IO_SEND_OK));
    EXPECTED_CALL(mocks, gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(mocks, list_remove(TEST_LIST_HANDLE, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(mocks, list_get_head_item(TEST_LIST_HANDLE));
    EXPECTED_CALL(mocks, gballoc_free(IGNORED_PTR_ARG));

    // act
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_WRITEABLE, saved_ws_callback_context, NULL, 0);

    // assert
    mocks.AssertActualAndExpectedCalls();

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_060: [The argument on_send_complete shall be optional, if NULL is passed by the caller then no send complete callback shall be triggered.] */
TEST_FUNCTION(when_lws_wants_to_send_bytes_they_are_pushed_to_lws_but_no_callback_is_called_if_none_was_specified)
{
    // arrange
    wsio_mocks mocks;
    unsigned char test_buffer[] = { 0x42 };
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    STRICT_EXPECTED_CALL(mocks, lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_ESTABLISHED, saved_ws_callback_context, NULL, 0);
    (void)wsio_send(wsio, test_buffer, sizeof(test_buffer), NULL, (void*)0x4243);
    mocks.ResetAllCalls();

    STRICT_EXPECTED_CALL(mocks, lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(mocks, lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    STRICT_EXPECTED_CALL(mocks, list_get_head_item(TEST_LIST_HANDLE));
    EXPECTED_CALL(mocks, list_item_get_value(IGNORED_PTR_ARG));
    EXPECTED_CALL(mocks, gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(mocks, lws_write(TEST_LIBWEBSOCKET, IGNORED_PTR_ARG, sizeof(test_buffer), LWS_WRITE_BINARY))
        .ValidateArgumentBuffer(2, test_buffer, sizeof(test_buffer))
        .SetReturn((int)sizeof(test_buffer));
    EXPECTED_CALL(mocks, gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(mocks, gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(mocks, list_remove(TEST_LIST_HANDLE, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(mocks, list_get_head_item(TEST_LIST_HANDLE));
    EXPECTED_CALL(mocks, gballoc_free(IGNORED_PTR_ARG));

    // act
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_WRITEABLE, saved_ws_callback_context, NULL, 0);

    // assert
    mocks.AssertActualAndExpectedCalls();

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_051: [If the wsio is not OPEN (open has not been called or is still in progress) then wsio_send shall fail and return a non-zero value.] */
TEST_FUNCTION(when_ws_io_is_not_opened_yet_wsio_send_fails)
{
    // arrange
    wsio_mocks mocks;
    unsigned char test_buffer[] = { 0x42 };
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    mocks.ResetAllCalls();

    // act
    int result = wsio_send(wsio, test_buffer, sizeof(test_buffer), test_on_send_complete, (void*)0x4243);

    // assert
    mocks.AssertActualAndExpectedCalls();
    ASSERT_ARE_NOT_EQUAL(int, 0, result);

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_051: [If the wsio is not OPEN (open has not been called or is still in progress) then wsio_send shall fail and return a non-zero value.] */
TEST_FUNCTION(when_ws_io_is_opening_wsio_send_fails)
{
    // arrange
    wsio_mocks mocks;
    unsigned char test_buffer[] = { 0x42 };
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    mocks.ResetAllCalls();

    // act
    int result = wsio_send(wsio, test_buffer, sizeof(test_buffer), test_on_send_complete, (void*)0x4243);

    // assert
    mocks.AssertActualAndExpectedCalls();
    ASSERT_ARE_NOT_EQUAL(int, 0, result);

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_052: [If any of the arguments ws_io or buffer are NULL, wsio_send shall fail and return a non-zero value.] */
TEST_FUNCTION(when_wsio_is_NULL_wsio_send_fails)
{
    // arrange
    wsio_mocks mocks;
    unsigned char test_buffer[] = { 0x42 };
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    STRICT_EXPECTED_CALL(mocks, lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_ESTABLISHED, saved_ws_callback_context, NULL, 0);
    mocks.ResetAllCalls();

    // act
    int result = wsio_send(NULL, test_buffer, sizeof(test_buffer), test_on_send_complete, (void*)0x4243);

    // assert
    mocks.AssertActualAndExpectedCalls();
    ASSERT_ARE_NOT_EQUAL(int, 0, result);

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_052: [If any of the arguments ws_io or buffer are NULL, wsio_send shall fail and return a non-zero value.] */
TEST_FUNCTION(when_buffer_is_NULL_wsio_send_fails)
{
    // arrange
    wsio_mocks mocks;
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    STRICT_EXPECTED_CALL(mocks, lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_ESTABLISHED, saved_ws_callback_context, NULL, 0);
    mocks.ResetAllCalls();

    // act
    int result = wsio_send(wsio, NULL, 1, test_on_send_complete, (void*)0x4243);

    // assert
    mocks.AssertActualAndExpectedCalls();
    ASSERT_ARE_NOT_EQUAL(int, 0, result);

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_053: [If size is zero then wsio_send shall fail and return a non-zero value.] */
TEST_FUNCTION(when_size_is_zero_wsio_send_fails)
{
    // arrange
    wsio_mocks mocks;
    unsigned char test_buffer[] = { 0x42 };
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    STRICT_EXPECTED_CALL(mocks, lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_ESTABLISHED, saved_ws_callback_context, NULL, 0);
    mocks.ResetAllCalls();

    // act
    int result = wsio_send(wsio, test_buffer, 0, test_on_send_complete, (void*)0x4243);

    // assert
    mocks.AssertActualAndExpectedCalls();
    ASSERT_ARE_NOT_EQUAL(int, 0, result);

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_108: [wsio_close shall obtain all the pending IO items by repetitively querying for the head of the pending IO list and freeing that head item.] */
/* Tests_SRS_WSIO_01_111: [Obtaining the head of the pending IO list shall be done by calling list_get_head_item.] */
/* Tests_SRS_WSIO_01_109: [For each pending item the send complete callback shall be called with IO_SEND_CANCELLED.] */
/* Tests_SRS_WSIO_01_110: [The callback context passed to the on_send_complete callback shall be the context given to wsio_send.] */
TEST_FUNCTION(wsio_close_with_a_pending_send_cancels_the_send)
{
    // arrange
    wsio_mocks mocks;
    unsigned char test_buffer[] = { 0x42 };
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    STRICT_EXPECTED_CALL(mocks, lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_ESTABLISHED, saved_ws_callback_context, NULL, 0);
    (void)wsio_send(wsio, test_buffer, sizeof(test_buffer), test_on_send_complete, (void*)0x4243);
    mocks.ResetAllCalls();

    STRICT_EXPECTED_CALL(mocks, list_get_head_item(TEST_LIST_HANDLE));
    EXPECTED_CALL(mocks, list_item_get_value(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(mocks, test_on_send_complete((void*)0x4243, IO_SEND_CANCELLED));
    EXPECTED_CALL(mocks, gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(mocks, gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(mocks, list_remove(TEST_LIST_HANDLE, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(mocks, list_get_head_item(TEST_LIST_HANDLE));
    STRICT_EXPECTED_CALL(mocks, lws_context_destroy(TEST_LIBWEBSOCKET_CONTEXT));

    // act
    (void)wsio_close(wsio, NULL, NULL);

    // assert
    mocks.AssertActualAndExpectedCalls();

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_110: [The callback context passed to the on_send_complete callback shall be the context given to wsio_send.] */
/* Tests_SRS_WSIO_01_059: [The callback_context argument shall be passed to on_send_complete as is.] */
TEST_FUNCTION(wsio_close_with_a_pending_send_cancels_the_send_and_passes_the_appropriate_callback_context)
{
    // arrange
    wsio_mocks mocks;
    unsigned char test_buffer[] = { 0x42 };
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    STRICT_EXPECTED_CALL(mocks, lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_ESTABLISHED, saved_ws_callback_context, NULL, 0);
    (void)wsio_send(wsio, test_buffer, sizeof(test_buffer), test_on_send_complete, (void*)0x4243);
    (void)wsio_send(wsio, test_buffer, sizeof(test_buffer), test_on_send_complete, (void*)0x4244);
    mocks.ResetAllCalls();

    STRICT_EXPECTED_CALL(mocks, list_get_head_item(TEST_LIST_HANDLE));
    EXPECTED_CALL(mocks, list_item_get_value(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(mocks, test_on_send_complete((void*)0x4243, IO_SEND_CANCELLED));
    EXPECTED_CALL(mocks, gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(mocks, gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(mocks, list_remove(TEST_LIST_HANDLE, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(mocks, list_get_head_item(TEST_LIST_HANDLE));
    EXPECTED_CALL(mocks, list_item_get_value(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(mocks, test_on_send_complete((void*)0x4244, IO_SEND_CANCELLED));
    EXPECTED_CALL(mocks, gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(mocks, gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(mocks, list_remove(TEST_LIST_HANDLE, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(mocks, list_get_head_item(TEST_LIST_HANDLE));
    STRICT_EXPECTED_CALL(mocks, lws_context_destroy(TEST_LIBWEBSOCKET_CONTEXT));

    // act
    (void)wsio_close(wsio, NULL, NULL);

    // assert
    mocks.AssertActualAndExpectedCalls();

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_060: [The argument on_send_complete shall be optional, if NULL is passed by the caller then no send complete callback shall be triggered.] */
TEST_FUNCTION(wsio_close_with_a_pending_send_cancels_the_send_but_doen_not_call_calback_if_no_callback_was_specified)
{
    // arrange
    wsio_mocks mocks;
    unsigned char test_buffer[] = { 0x42 };
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    STRICT_EXPECTED_CALL(mocks, lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_ESTABLISHED, saved_ws_callback_context, NULL, 0);
    (void)wsio_send(wsio, test_buffer, sizeof(test_buffer), NULL, (void*)0x4243);
    mocks.ResetAllCalls();

    STRICT_EXPECTED_CALL(mocks, list_get_head_item(TEST_LIST_HANDLE));
    EXPECTED_CALL(mocks, list_item_get_value(IGNORED_PTR_ARG));
    EXPECTED_CALL(mocks, gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(mocks, gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(mocks, list_remove(TEST_LIST_HANDLE, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(mocks, list_get_head_item(TEST_LIST_HANDLE));
    STRICT_EXPECTED_CALL(mocks, lws_context_destroy(TEST_LIBWEBSOCKET_CONTEXT));

    // act
    (void)wsio_close(wsio, NULL, NULL);

    // assert
    mocks.AssertActualAndExpectedCalls();

    // cleanup
    wsio_destroy(wsio);
}

/* wsio_dowork */

/* Tests_SRS_WSIO_01_061: [wsio_dowork shall service the libwebsockets context by calling lws_service and passing as argument the context obtained in wsio_open.] */
/* Tests_SRS_WSIO_01_112: [The timeout for lws_service shall be 0.] */
/* Tests_SRS_WSIO_01_062: [This shall be done if the IO is not closed.] */
TEST_FUNCTION(wsio_dowork_services_lws)
{
    // arrange
    wsio_mocks mocks;
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    STRICT_EXPECTED_CALL(mocks, lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_ESTABLISHED, saved_ws_callback_context, NULL, 0);
    mocks.ResetAllCalls();

    STRICT_EXPECTED_CALL(mocks, lws_service(TEST_LIBWEBSOCKET_CONTEXT, 0));

    // act
    wsio_dowork(wsio);

    // assert
    mocks.AssertActualAndExpectedCalls();

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_063: [If the ws_io argument is NULL, wsio_dowork shall do nothing.]  */
TEST_FUNCTION(wsio_dowork_with_NULL_handles_does_not_call_lws_service)
{
    // arrange
    wsio_mocks mocks;

    // act
    wsio_dowork(NULL);

    // assert
    // no explicit assert, uMock checks the calls
}

/* Tests_SRS_WSIO_01_061: [wsio_dowork shall service the libwebsockets context by calling lws_service and passing as argument the context obtained in wsio_open.] */
/* Tests_SRS_WSIO_01_112: [The timeout for lws_service shall be 0.] */
/* Tests_SRS_WSIO_01_062: [This shall be done if the IO is not closed.] */
TEST_FUNCTION(wsio_dowork_services_lws_when_still_opening)
{
    // arrange
    wsio_mocks mocks;
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    STRICT_EXPECTED_CALL(mocks, lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    mocks.ResetAllCalls();

    STRICT_EXPECTED_CALL(mocks, lws_service(TEST_LIBWEBSOCKET_CONTEXT, 0));

    // act
    wsio_dowork(wsio);

    // assert
    mocks.AssertActualAndExpectedCalls();

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_062: [This shall be done if the IO is not closed.] */
TEST_FUNCTION(wsio_dowork_does_not_service_lws_when_not_yet_open)
{
    // arrange
    wsio_mocks mocks;
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    mocks.ResetAllCalls();

    // act
    wsio_dowork(wsio);

    // assert
    mocks.AssertActualAndExpectedCalls();

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_062: [This shall be done if the IO is not closed.] */
TEST_FUNCTION(wsio_dowork_does_not_service_lws_when_closed)
{
    // arrange
    wsio_mocks mocks;
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    STRICT_EXPECTED_CALL(mocks, lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_ESTABLISHED, saved_ws_callback_context, NULL, 0);
    wsio_close(wsio, NULL, NULL);
    mocks.ResetAllCalls();

    // act
    wsio_dowork(wsio);

    // assert
    mocks.AssertActualAndExpectedCalls();

    // cleanup
    wsio_destroy(wsio);
}

/* wsio_get_interface_description */

/* Tests_SRS_WSIO_01_064: [wsio_get_interface_description shall return a pointer to an IO_INTERFACE_DESCRIPTION structure that contains pointers to the functions: wsio_create, wsio_destroy, wsio_open, wsio_close, wsio_send and wsio_dowork.] */
TEST_FUNCTION(wsio_get_interface_description_fills_the_interface_structure)
{
    // arrange
    wsio_mocks mocks;

    // act
    const IO_INTERFACE_DESCRIPTION* if_description = wsio_get_interface_description();

    // assert
    ASSERT_ARE_EQUAL(void_ptr, (void_ptr)wsio_create, (void_ptr)if_description->concrete_io_create);
    ASSERT_ARE_EQUAL(void_ptr, (void_ptr)wsio_destroy, (void_ptr)if_description->concrete_io_destroy);
    ASSERT_ARE_EQUAL(void_ptr, (void_ptr)wsio_open, (void_ptr)if_description->concrete_io_open);
    ASSERT_ARE_EQUAL(void_ptr, (void_ptr)wsio_close, (void_ptr)if_description->concrete_io_close);
    ASSERT_ARE_EQUAL(void_ptr, (void_ptr)wsio_send, (void_ptr)if_description->concrete_io_send);
    ASSERT_ARE_EQUAL(void_ptr, (void_ptr)wsio_dowork, (void_ptr)if_description->concrete_io_dowork);
}

/* on_ws_callback */

/* Tests_SRS_WSIO_01_066: [If an open action is pending, the on_io_open_complete callback shall be triggered with IO_OPEN_OK and from now on it shall be possible to send/receive data.] */
TEST_FUNCTION(CLIENT_CONNECTED_when_opening_triggers_an_open_complete_callback)
{
    // arrange
    wsio_mocks mocks;
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    mocks.ResetAllCalls();

    STRICT_EXPECTED_CALL(mocks, lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(mocks, lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    STRICT_EXPECTED_CALL(mocks, test_on_io_open_complete((void*)0x4242, IO_OPEN_OK));

    // act
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_ESTABLISHED, saved_ws_callback_context, NULL, 0);

    // assert
    mocks.AssertActualAndExpectedCalls();

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_068: [If the IO is already open, the on_io_error callback shall be triggered.] */
TEST_FUNCTION(CLIENT_CONNECTED_when_already_open_triggers_an_error)
{
    // arrange
    wsio_mocks mocks;
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    STRICT_EXPECTED_CALL(mocks, lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_ESTABLISHED, saved_ws_callback_context, NULL, 0);
    mocks.ResetAllCalls();

    STRICT_EXPECTED_CALL(mocks, lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(mocks, lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    STRICT_EXPECTED_CALL(mocks, test_on_io_error((void*)0x4242));

    // act
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_ESTABLISHED, saved_ws_callback_context, NULL, 0);

    // assert
    mocks.AssertActualAndExpectedCalls();

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_069: [If an open action is pending, the on_io_open_complete callback shall be triggered with IO_OPEN_ERROR.] */
TEST_FUNCTION(CLIENT_CONNECTION_ERROR_when_opening_yields_open_complete_with_error)
{
    // arrange
    wsio_mocks mocks;
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    mocks.ResetAllCalls();

    STRICT_EXPECTED_CALL(mocks, lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(mocks, lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    STRICT_EXPECTED_CALL(mocks, test_on_io_open_complete((void*)0x4242, IO_OPEN_ERROR));
    STRICT_EXPECTED_CALL(mocks, lws_context_destroy(TEST_LIBWEBSOCKET_CONTEXT));

    // act
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_CONNECTION_ERROR, saved_ws_callback_context, NULL, 0);

    // assert
    mocks.AssertActualAndExpectedCalls();

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_069: [If an open action is pending, the on_io_open_complete callback shall be triggered with IO_OPEN_ERROR.] */
TEST_FUNCTION(CLIENT_CONNECTION_ERROR_when_opening_with_NULL_open_complete_callback_still_frees_the_lws_context)
{
    // arrange
    wsio_mocks mocks;
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, NULL, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    mocks.ResetAllCalls();

    STRICT_EXPECTED_CALL(mocks, lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(mocks, lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    STRICT_EXPECTED_CALL(mocks, lws_context_destroy(TEST_LIBWEBSOCKET_CONTEXT));

    // act
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_CONNECTION_ERROR, saved_ws_callback_context, NULL, 0);

    // assert
    mocks.AssertActualAndExpectedCalls();

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_070: [If the IO is already open, the on_io_error callback shall be triggered.] */
TEST_FUNCTION(CLIENT_CONNECTION_ERROR_when_already_open_indicates_an_error_to_the_consumer)
{
    // arrange
    wsio_mocks mocks;
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    STRICT_EXPECTED_CALL(mocks, lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_ESTABLISHED, saved_ws_callback_context, NULL, 0);
    mocks.ResetAllCalls();

    STRICT_EXPECTED_CALL(mocks, lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(mocks, lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    STRICT_EXPECTED_CALL(mocks, test_on_io_error((void*)0x4242));

    // act
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_CONNECTION_ERROR, saved_ws_callback_context, NULL, 0);

    // assert
    mocks.AssertActualAndExpectedCalls();

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_071: [If any pending IO chunks queued in wsio_send are to be sent, then the first one shall be retrieved from the queue.] */
/* Tests_SRS_WSIO_01_072: [Enough space to fit the data and LWS_SEND_BUFFER_PRE_PADDING and LWS_SEND_BUFFER_POST_PADDING shall be allocated.] */
/* Tests_SRS_WSIO_01_074: [The payload queued in wsio_send shall be copied to the newly allocated buffer at the position LWS_SEND_BUFFER_PRE_PADDING.] */
/* Tests_SRS_WSIO_01_075: [lws_write shall be called with the websockets interface obtained in wsio_open, the newly constructed padded buffer, the data size queued in wsio_send (actual payload) and the payload type should be set to LWS_WRITE_BINARY.] */
/* Tests_SRS_WSIO_01_077: [If lws_write succeeds and the complete payload has been sent, the queued pending IO shall be removed from the pending list.] */
/* Tests_SRS_WSIO_01_078: [If the pending IO had an associated on_send_complete, then the on_send_complete function shall be called with the callback_context and IO_SEND_OK as arguments.] */
/* Tests_SRS_WSIO_01_120: [This event shall only be processed if the IO is open.] */
TEST_FUNCTION(CLIENT_WRITABLE_when_open_and_one_chunk_queued_sends_the_chunk)
{
    // arrange
    wsio_mocks mocks;
    unsigned char test_buffer[] = { 0x42 };
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    STRICT_EXPECTED_CALL(mocks, lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_ESTABLISHED, saved_ws_callback_context, NULL, 0);
    (void)wsio_send(wsio, test_buffer, sizeof(test_buffer), NULL, (void*)0x4243);
    mocks.ResetAllCalls();

    STRICT_EXPECTED_CALL(mocks, lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(mocks, lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    STRICT_EXPECTED_CALL(mocks, list_get_head_item(TEST_LIST_HANDLE));
    EXPECTED_CALL(mocks, list_item_get_value(IGNORED_PTR_ARG));
    EXPECTED_CALL(mocks, gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(mocks, lws_write(TEST_LIBWEBSOCKET, IGNORED_PTR_ARG, sizeof(test_buffer), LWS_WRITE_BINARY))
        .ValidateArgumentBuffer(2, test_buffer, sizeof(test_buffer))
        .SetReturn((int)sizeof(test_buffer));
    EXPECTED_CALL(mocks, gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(mocks, gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(mocks, list_remove(TEST_LIST_HANDLE, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(mocks, list_get_head_item(TEST_LIST_HANDLE));
    EXPECTED_CALL(mocks, gballoc_free(IGNORED_PTR_ARG));

    // act
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_WRITEABLE, saved_ws_callback_context, NULL, 0);

    // assert
    mocks.AssertActualAndExpectedCalls();

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_121: [If this event is received in while an open action is incomplete, the open_complete callback shall be called with IO_OPEN_ERROR.] */
TEST_FUNCTION(CLIENT_WRITABLE_when_opening_yields_an_open_complete_error)
{
    // arrange
    wsio_mocks mocks;
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    mocks.ResetAllCalls();

    STRICT_EXPECTED_CALL(mocks, lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(mocks, lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    STRICT_EXPECTED_CALL(mocks, test_on_io_open_complete((void*)0x4242, IO_OPEN_ERROR));
    STRICT_EXPECTED_CALL(mocks, lws_context_destroy(TEST_LIBWEBSOCKET_CONTEXT));

    // act
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_WRITEABLE, saved_ws_callback_context, NULL, 0);

    // assert
    mocks.AssertActualAndExpectedCalls();

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_071: [If any pending IO chunks queued in wsio_send are to be sent, then the first one shall be retrieved from the queue.] */
TEST_FUNCTION(when_no_items_are_pending_in_CLIENT_WRITABLE_nothing_happens)
{
    // arrange
    wsio_mocks mocks;
    unsigned char test_buffer[] = { 0x42 };
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    STRICT_EXPECTED_CALL(mocks, lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_ESTABLISHED, saved_ws_callback_context, NULL, 0);
    (void)wsio_send(wsio, test_buffer, sizeof(test_buffer), NULL, (void*)0x4243);
    mocks.ResetAllCalls();

    STRICT_EXPECTED_CALL(mocks, lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(mocks, lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    STRICT_EXPECTED_CALL(mocks, list_get_head_item(TEST_LIST_HANDLE))
        .SetReturn((LIST_ITEM_HANDLE)NULL);

    // act
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_WRITEABLE, saved_ws_callback_context, NULL, 0);

    // assert
    mocks.AssertActualAndExpectedCalls();

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_071: [If any pending IO chunks queued in wsio_send are to be sent, then the first one shall be retrieved from the queue.] */
TEST_FUNCTION(when_getting_the_pending_io_data_in_CLIENT_WRITABLE_fails_then_an_error_is_indicated)
{
    // arrange
    wsio_mocks mocks;
    unsigned char test_buffer[] = { 0x42 };
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    STRICT_EXPECTED_CALL(mocks, lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_ESTABLISHED, saved_ws_callback_context, NULL, 0);
    (void)wsio_send(wsio, test_buffer, sizeof(test_buffer), NULL, (void*)0x4243);
    mocks.ResetAllCalls();

    STRICT_EXPECTED_CALL(mocks, lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(mocks, lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    STRICT_EXPECTED_CALL(mocks, list_get_head_item(TEST_LIST_HANDLE));
    EXPECTED_CALL(mocks, list_item_get_value(IGNORED_PTR_ARG))
        .SetReturn((void*)NULL);
    STRICT_EXPECTED_CALL(mocks, test_on_io_error((void*)0x4242));

    // act
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_WRITEABLE, saved_ws_callback_context, NULL, 0);

    // assert
    mocks.AssertActualAndExpectedCalls();

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_073: [If allocating the memory fails then the send_result callback callback shall be triggered with IO_SEND_ERROR.] */
/* Tests_SRS_WSIO_01_081: [If no errors prohibiting further processing of other pending IO chunks have happened, then lws_callback_on_writable shall be called, while passing the websockets context and interface obtained in wsio_open as arguments.] */
TEST_FUNCTION(when_allocating_memory_for_lws_in_CLIENT_WRITABLE_fails_then_an_error_is_indicated)
{
    // arrange
    wsio_mocks mocks;
    unsigned char test_buffer[] = { 0x42 };
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    STRICT_EXPECTED_CALL(mocks, lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_ESTABLISHED, saved_ws_callback_context, NULL, 0);
    (void)wsio_send(wsio, test_buffer, sizeof(test_buffer), test_on_send_complete, (void*)0x4243);
    mocks.ResetAllCalls();

    STRICT_EXPECTED_CALL(mocks, lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(mocks, lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    STRICT_EXPECTED_CALL(mocks, list_get_head_item(TEST_LIST_HANDLE));
    EXPECTED_CALL(mocks, list_item_get_value(IGNORED_PTR_ARG));
    EXPECTED_CALL(mocks, gballoc_malloc(IGNORED_NUM_ARG))
        .SetReturn((void*)NULL);
    STRICT_EXPECTED_CALL(mocks, test_on_send_complete((void*)0x4243, IO_SEND_ERROR));
    EXPECTED_CALL(mocks, gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(mocks, gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(mocks, list_remove(TEST_LIST_HANDLE, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(mocks, list_get_head_item(TEST_LIST_HANDLE))
        .SetReturn((LIST_ITEM_HANDLE)NULL);

    // act
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_WRITEABLE, saved_ws_callback_context, NULL, 0);

    // assert
    mocks.AssertActualAndExpectedCalls();

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_076: [If lws_write fails (result is less than 0) then the send_complete callback shall be triggered with IO_SEND_ERROR.] */
TEST_FUNCTION(when_lws_write_fails_in_CLIENT_WRITABLE_then_an_error_is_indicated)
{
    // arrange
    wsio_mocks mocks;
    unsigned char test_buffer[] = { 0x42 };
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    STRICT_EXPECTED_CALL(mocks, lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_ESTABLISHED, saved_ws_callback_context, NULL, 0);
    (void)wsio_send(wsio, test_buffer, sizeof(test_buffer), test_on_send_complete, (void*)0x4243);
    mocks.ResetAllCalls();

    STRICT_EXPECTED_CALL(mocks, lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(mocks, lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    STRICT_EXPECTED_CALL(mocks, list_get_head_item(TEST_LIST_HANDLE));
    EXPECTED_CALL(mocks, list_item_get_value(IGNORED_PTR_ARG));
    EXPECTED_CALL(mocks, gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(mocks, lws_write(TEST_LIBWEBSOCKET, IGNORED_PTR_ARG, sizeof(test_buffer), LWS_WRITE_BINARY))
        .ValidateArgumentBuffer(2, test_buffer, sizeof(test_buffer))
        .SetReturn(-1);
    EXPECTED_CALL(mocks, gballoc_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(mocks, test_on_send_complete((void*)0x4243, IO_SEND_ERROR));
    EXPECTED_CALL(mocks, gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(mocks, gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(mocks, list_remove(TEST_LIST_HANDLE, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(mocks, list_get_head_item(TEST_LIST_HANDLE))
        .SetReturn((LIST_ITEM_HANDLE)NULL);

    // act
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_WRITEABLE, saved_ws_callback_context, NULL, 0);

    // assert
    mocks.AssertActualAndExpectedCalls();

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_113: [If allocating the memory fails for a pending IO that has been partially sent already then the on_io_error callback shall also be triggered.] */
TEST_FUNCTION(when_allocating_memory_for_the_lws_write_fails_in_CLIENT_WRITABLE_for_a_pending_io_that_was_partially_sent_an_error_is_indicated)
{
    // arrange
    wsio_mocks mocks;
    unsigned char test_buffer[] = { 0x42, 0x43 };
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    STRICT_EXPECTED_CALL(mocks, lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_ESTABLISHED, saved_ws_callback_context, NULL, 0);
    (void)wsio_send(wsio, test_buffer, sizeof(test_buffer), test_on_send_complete, (void*)0x4243);

    STRICT_EXPECTED_CALL(mocks, lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    STRICT_EXPECTED_CALL(mocks, lws_write(TEST_LIBWEBSOCKET, IGNORED_PTR_ARG, sizeof(test_buffer), LWS_WRITE_BINARY))
        .ValidateArgumentBuffer(2, test_buffer, sizeof(test_buffer))
        .SetReturn(1);
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_WRITEABLE, saved_ws_callback_context, NULL, 0);
    mocks.ResetAllCalls();

    STRICT_EXPECTED_CALL(mocks, lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(mocks, lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    STRICT_EXPECTED_CALL(mocks, list_get_head_item(TEST_LIST_HANDLE));
    EXPECTED_CALL(mocks, list_item_get_value(IGNORED_PTR_ARG));
    EXPECTED_CALL(mocks, gballoc_malloc(IGNORED_NUM_ARG))
        .SetReturn((void*)NULL);
    STRICT_EXPECTED_CALL(mocks, test_on_send_complete((void*)0x4243, IO_SEND_ERROR));
    STRICT_EXPECTED_CALL(mocks, test_on_io_error((void*)0x4242));
    EXPECTED_CALL(mocks, gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(mocks, gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(mocks, list_remove(TEST_LIST_HANDLE, IGNORED_PTR_ARG));

    // act
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_WRITEABLE, saved_ws_callback_context, NULL, 0);

    // assert
    mocks.AssertActualAndExpectedCalls();

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_114: [Additionally, if the failure is for a pending IO that has been partially sent already then the on_io_error callback shall also be triggered.] */
TEST_FUNCTION(when_lws_write_fails_in_CLIENT_WRITABLE_for_a_pending_io_that_was_partially_sent_an_error_is_indicated)
{
    // arrange
    wsio_mocks mocks;
    unsigned char test_buffer[] = { 0x42, 0x43 };
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    STRICT_EXPECTED_CALL(mocks, lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_ESTABLISHED, saved_ws_callback_context, NULL, 0);
    (void)wsio_send(wsio, test_buffer, sizeof(test_buffer), test_on_send_complete, (void*)0x4243);

    STRICT_EXPECTED_CALL(mocks, lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    STRICT_EXPECTED_CALL(mocks, lws_write(TEST_LIBWEBSOCKET, IGNORED_PTR_ARG, sizeof(test_buffer), LWS_WRITE_BINARY))
        .ValidateArgumentBuffer(2, test_buffer, sizeof(test_buffer))
        .SetReturn(1);
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_WRITEABLE, saved_ws_callback_context, NULL, 0);
    mocks.ResetAllCalls();

    STRICT_EXPECTED_CALL(mocks, lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(mocks, lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    STRICT_EXPECTED_CALL(mocks, list_get_head_item(TEST_LIST_HANDLE));
    EXPECTED_CALL(mocks, list_item_get_value(IGNORED_PTR_ARG));
    EXPECTED_CALL(mocks, gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(mocks, lws_write(TEST_LIBWEBSOCKET, IGNORED_PTR_ARG, 1, LWS_WRITE_BINARY))
        .ValidateArgumentBuffer(2, test_buffer + 1, 1)
        .SetReturn(-1);
    EXPECTED_CALL(mocks, gballoc_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(mocks, test_on_send_complete((void*)0x4243, IO_SEND_ERROR));
    STRICT_EXPECTED_CALL(mocks, test_on_io_error((void*)0x4242));
    EXPECTED_CALL(mocks, gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(mocks, gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(mocks, list_remove(TEST_LIST_HANDLE, IGNORED_PTR_ARG));

    // act
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_WRITEABLE, saved_ws_callback_context, NULL, 0);

    // assert
    mocks.AssertActualAndExpectedCalls();

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_079: [If the send was successful and any error occurs during removing the pending IO from the list then the on_io_error callback shall be triggered.]  */
TEST_FUNCTION(when_removing_the_pending_IO_after_a_succesfull_write_lws_write_fails_in_CLIENT_WRITABLE_then_an_error_is_indicated)
{
    // arrange
    wsio_mocks mocks;
    unsigned char test_buffer[] = { 0x42, 0x43 };
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    STRICT_EXPECTED_CALL(mocks, lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_ESTABLISHED, saved_ws_callback_context, NULL, 0);
    (void)wsio_send(wsio, test_buffer, sizeof(test_buffer), test_on_send_complete, (void*)0x4243);
    mocks.ResetAllCalls();

    STRICT_EXPECTED_CALL(mocks, lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(mocks, lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    STRICT_EXPECTED_CALL(mocks, list_get_head_item(TEST_LIST_HANDLE));
    EXPECTED_CALL(mocks, list_item_get_value(IGNORED_PTR_ARG));
    EXPECTED_CALL(mocks, gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(mocks, lws_write(TEST_LIBWEBSOCKET, IGNORED_PTR_ARG, sizeof(test_buffer), LWS_WRITE_BINARY))
        .ValidateArgumentBuffer(2, test_buffer, sizeof(test_buffer))
        .SetReturn(2);
    EXPECTED_CALL(mocks, gballoc_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(mocks, test_on_send_complete((void*)0x4243, IO_SEND_OK));
    EXPECTED_CALL(mocks, gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(mocks, gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(mocks, list_remove(TEST_LIST_HANDLE, IGNORED_PTR_ARG))
        .SetReturn(1);
    STRICT_EXPECTED_CALL(mocks, test_on_io_error((void*)0x4242));

    // act
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_WRITEABLE, saved_ws_callback_context, NULL, 0);

    // assert
    mocks.AssertActualAndExpectedCalls();

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_081: [If any pending IOs are in the list, lws_callback_on_writable shall be called, while passing the websockets instance obtained in wsio_open as arguments if:] */
/* Tests_SRS_WSIO_01_116: [The send failed writing to lws or allocating memory for the data to be passed to lws and no partial data has been sent previously for the pending IO.] */
TEST_FUNCTION(when_allocating_memory_for_lws_in_CLIENT_WRITABLE_and_another_pending_IO_exists_then_callback_on_writable_is_called)
{
    // arrange
    wsio_mocks mocks;
    unsigned char test_buffer[] = { 0x42 };
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    STRICT_EXPECTED_CALL(mocks, lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_ESTABLISHED, saved_ws_callback_context, NULL, 0);
    (void)wsio_send(wsio, test_buffer, sizeof(test_buffer), test_on_send_complete, (void*)0x4243);
    mocks.ResetAllCalls();

    STRICT_EXPECTED_CALL(mocks, lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(mocks, lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    STRICT_EXPECTED_CALL(mocks, list_get_head_item(TEST_LIST_HANDLE));
    EXPECTED_CALL(mocks, list_item_get_value(IGNORED_PTR_ARG));
    EXPECTED_CALL(mocks, gballoc_malloc(IGNORED_NUM_ARG))
        .SetReturn((void*)NULL);
    STRICT_EXPECTED_CALL(mocks, test_on_send_complete((void*)0x4243, IO_SEND_ERROR));
    EXPECTED_CALL(mocks, gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(mocks, gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(mocks, list_remove(TEST_LIST_HANDLE, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(mocks, list_get_head_item(TEST_LIST_HANDLE));
    STRICT_EXPECTED_CALL(mocks, lws_callback_on_writable(TEST_LIBWEBSOCKET));

    // act
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_WRITEABLE, saved_ws_callback_context, NULL, 0);

    // assert
    mocks.AssertActualAndExpectedCalls();

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_081: [If any pending IOs are in the list, lws_callback_on_writable shall be called, while passing the websockets instance obtained in wsio_open as arguments if:] */
/* Tests_SRS_WSIO_01_116: [The send failed writing to lws or allocating memory for the data to be passed to lws and no partial data has been sent previously for the pending IO.] */
TEST_FUNCTION(when_lws_write_in_CLIENT_WRITABLE_fails_and_another_pending_IO_exists_then_callback_on_writable_is_called)
{
    // arrange
    wsio_mocks mocks;
    unsigned char test_buffer[] = { 0x42 };
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    STRICT_EXPECTED_CALL(mocks, lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_ESTABLISHED, saved_ws_callback_context, NULL, 0);
    (void)wsio_send(wsio, test_buffer, sizeof(test_buffer), test_on_send_complete, (void*)0x4243);
    mocks.ResetAllCalls();

    STRICT_EXPECTED_CALL(mocks, lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(mocks, lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    STRICT_EXPECTED_CALL(mocks, list_get_head_item(TEST_LIST_HANDLE));
    EXPECTED_CALL(mocks, list_item_get_value(IGNORED_PTR_ARG));
    EXPECTED_CALL(mocks, gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(mocks, lws_write(TEST_LIBWEBSOCKET, IGNORED_PTR_ARG, sizeof(test_buffer), LWS_WRITE_BINARY))
        .ValidateArgumentBuffer(2, test_buffer, sizeof(test_buffer))
        .SetReturn(-1);
    EXPECTED_CALL(mocks, gballoc_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(mocks, test_on_send_complete((void*)0x4243, IO_SEND_ERROR));
    EXPECTED_CALL(mocks, gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(mocks, gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(mocks, list_remove(TEST_LIST_HANDLE, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(mocks, list_get_head_item(TEST_LIST_HANDLE));
    STRICT_EXPECTED_CALL(mocks, lws_callback_on_writable(TEST_LIBWEBSOCKET));

    // act
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_WRITEABLE, saved_ws_callback_context, NULL, 0);

    // assert
    mocks.AssertActualAndExpectedCalls();

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_081: [If any pending IOs are in the list, lws_callback_on_writable shall be called, while passing the websockets instance obtained in wsio_open as arguments if:] */
/* Tests_SRS_WSIO_01_116: [The send failed writing to lws or allocating memory for the data to be passed to lws and no partial data has been sent previously for the pending IO.] */
TEST_FUNCTION(when_allocating_memory_in_CLIENT_WRITABLE_fails_after_a_partial_write_and_another_pending_IO_exists_then_callback_on_writable_is_not_called)
{
    // arrange
    wsio_mocks mocks;
    unsigned char test_buffer[] = { 0x42, 0x43 };
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    STRICT_EXPECTED_CALL(mocks, lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_ESTABLISHED, saved_ws_callback_context, NULL, 0);
    (void)wsio_send(wsio, test_buffer, sizeof(test_buffer), test_on_send_complete, (void*)0x4243);
    STRICT_EXPECTED_CALL(mocks, lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    STRICT_EXPECTED_CALL(mocks, lws_write(TEST_LIBWEBSOCKET, IGNORED_PTR_ARG, sizeof(test_buffer), LWS_WRITE_BINARY))
        .ValidateArgumentBuffer(2, test_buffer, sizeof(test_buffer))
        .SetReturn(1);
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_WRITEABLE, saved_ws_callback_context, NULL, 0);
    mocks.ResetAllCalls();

    STRICT_EXPECTED_CALL(mocks, lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(mocks, lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    STRICT_EXPECTED_CALL(mocks, list_get_head_item(TEST_LIST_HANDLE));
    EXPECTED_CALL(mocks, list_item_get_value(IGNORED_PTR_ARG));
    EXPECTED_CALL(mocks, gballoc_malloc(IGNORED_NUM_ARG))
        .SetReturn((void*)NULL);
    STRICT_EXPECTED_CALL(mocks, test_on_send_complete((void*)0x4243, IO_SEND_ERROR));
    STRICT_EXPECTED_CALL(mocks, test_on_io_error((void*)0x4242));
    EXPECTED_CALL(mocks, gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(mocks, gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(mocks, list_remove(TEST_LIST_HANDLE, IGNORED_PTR_ARG));

    // act
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_WRITEABLE, saved_ws_callback_context, NULL, 0);

    // assert
    mocks.AssertActualAndExpectedCalls();

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_081: [If any pending IOs are in the list, lws_callback_on_writable shall be called, while passing the websockets instance obtained in wsio_open as arguments if:] */
/* Tests_SRS_WSIO_01_116: [The send failed writing to lws or allocating memory for the data to be passed to lws and no partial data has been sent previously for the pending IO.] */
TEST_FUNCTION(when_lws_write_in_CLIENT_WRITABLE_fails_after_a_partial_write_and_another_pending_IO_exists_then_callback_on_writable_is_not_called)
{
    // arrange
    wsio_mocks mocks;
    unsigned char test_buffer[] = { 0x42, 0x43 };
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    STRICT_EXPECTED_CALL(mocks, lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_ESTABLISHED, saved_ws_callback_context, NULL, 0);
    (void)wsio_send(wsio, test_buffer, sizeof(test_buffer), test_on_send_complete, (void*)0x4243);
    STRICT_EXPECTED_CALL(mocks, lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    STRICT_EXPECTED_CALL(mocks, lws_write(TEST_LIBWEBSOCKET, IGNORED_PTR_ARG, sizeof(test_buffer), LWS_WRITE_BINARY))
        .ValidateArgumentBuffer(2, test_buffer, sizeof(test_buffer))
        .SetReturn(1);
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_WRITEABLE, saved_ws_callback_context, NULL, 0);
    mocks.ResetAllCalls();

    STRICT_EXPECTED_CALL(mocks, lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(mocks, lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    STRICT_EXPECTED_CALL(mocks, list_get_head_item(TEST_LIST_HANDLE));
    EXPECTED_CALL(mocks, list_item_get_value(IGNORED_PTR_ARG));
    EXPECTED_CALL(mocks, gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(mocks, lws_write(TEST_LIBWEBSOCKET, IGNORED_PTR_ARG, 1, LWS_WRITE_BINARY))
        .ValidateArgumentBuffer(2, test_buffer + 1, 1)
        .SetReturn(-1);
    EXPECTED_CALL(mocks, gballoc_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(mocks, test_on_send_complete((void*)0x4243, IO_SEND_ERROR));
    STRICT_EXPECTED_CALL(mocks, test_on_io_error((void*)0x4242));
    EXPECTED_CALL(mocks, gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(mocks, gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(mocks, list_remove(TEST_LIST_HANDLE, IGNORED_PTR_ARG));

    // act
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_WRITEABLE, saved_ws_callback_context, NULL, 0);

    // assert
    mocks.AssertActualAndExpectedCalls();

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_081: [If any pending IOs are in the list, lws_callback_on_writable shall be called, while passing the websockets instance obtained in wsio_open as arguments if:] */
/* Tests_SRS_WSIO_01_115: [The send over websockets was successful] */
TEST_FUNCTION(when_send_is_succesfull_and_there_is_another_pending_IO_in_CLIENT_WRITABLE_then_callback_on_writable_is_called)
{
    // arrange
    wsio_mocks mocks;
    unsigned char test_buffer[] = { 0x42, 0x43 };
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    STRICT_EXPECTED_CALL(mocks, lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_ESTABLISHED, saved_ws_callback_context, NULL, 0);
    (void)wsio_send(wsio, test_buffer, sizeof(test_buffer), test_on_send_complete, (void*)0x4243);
    (void)wsio_send(wsio, test_buffer, sizeof(test_buffer), test_on_send_complete, (void*)0x4243);
    STRICT_EXPECTED_CALL(mocks, lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    STRICT_EXPECTED_CALL(mocks, lws_write(TEST_LIBWEBSOCKET, IGNORED_PTR_ARG, sizeof(test_buffer), LWS_WRITE_BINARY))
        .ValidateArgumentBuffer(2, test_buffer, sizeof(test_buffer))
        .SetReturn(1);
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_WRITEABLE, saved_ws_callback_context, NULL, 0);
    mocks.ResetAllCalls();

    STRICT_EXPECTED_CALL(mocks, lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(mocks, lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    STRICT_EXPECTED_CALL(mocks, list_get_head_item(TEST_LIST_HANDLE));
    EXPECTED_CALL(mocks, list_item_get_value(IGNORED_PTR_ARG));
    EXPECTED_CALL(mocks, gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(mocks, lws_write(TEST_LIBWEBSOCKET, IGNORED_PTR_ARG, 1, LWS_WRITE_BINARY))
        .ValidateArgumentBuffer(2, test_buffer + 1, 1)
        .SetReturn(1);
    EXPECTED_CALL(mocks, gballoc_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(mocks, test_on_send_complete((void*)0x4243, IO_SEND_OK));
    EXPECTED_CALL(mocks, gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(mocks, gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(mocks, list_remove(TEST_LIST_HANDLE, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(mocks, list_get_head_item(TEST_LIST_HANDLE));
    STRICT_EXPECTED_CALL(mocks, lws_callback_on_writable(TEST_LIBWEBSOCKET));

    // act
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_WRITEABLE, saved_ws_callback_context, NULL, 0);

    // assert
    mocks.AssertActualAndExpectedCalls();

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_117: [on_io_error should not be triggered twice when removing a pending IO that failed and a partial send for it has already been done.]  */
TEST_FUNCTION(when_removing_the_pending_IO_due_to_lws_write_failing_in_CLIENT_WRITABLE_fails_then_on_io_error_is_called_only_once)
{
    // arrange
    wsio_mocks mocks;
    unsigned char test_buffer[] = { 0x42, 0x43 };
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    STRICT_EXPECTED_CALL(mocks, lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_ESTABLISHED, saved_ws_callback_context, NULL, 0);
    (void)wsio_send(wsio, test_buffer, sizeof(test_buffer), test_on_send_complete, (void*)0x4243);
    STRICT_EXPECTED_CALL(mocks, lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    STRICT_EXPECTED_CALL(mocks, lws_write(TEST_LIBWEBSOCKET, IGNORED_PTR_ARG, sizeof(test_buffer), LWS_WRITE_BINARY))
        .ValidateArgumentBuffer(2, test_buffer, sizeof(test_buffer))
        .SetReturn(1);
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_WRITEABLE, saved_ws_callback_context, NULL, 0);
    mocks.ResetAllCalls();

    STRICT_EXPECTED_CALL(mocks, lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(mocks, lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    STRICT_EXPECTED_CALL(mocks, list_get_head_item(TEST_LIST_HANDLE));
    EXPECTED_CALL(mocks, list_item_get_value(IGNORED_PTR_ARG));
    EXPECTED_CALL(mocks, gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(mocks, lws_write(TEST_LIBWEBSOCKET, IGNORED_PTR_ARG, 1, LWS_WRITE_BINARY))
        .ValidateArgumentBuffer(2, test_buffer + 1, 1)
        .SetReturn(-1);
    EXPECTED_CALL(mocks, gballoc_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(mocks, test_on_send_complete((void*)0x4243, IO_SEND_ERROR));
    STRICT_EXPECTED_CALL(mocks, test_on_io_error((void*)0x4242));
    EXPECTED_CALL(mocks, gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(mocks, gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(mocks, list_remove(TEST_LIST_HANDLE, IGNORED_PTR_ARG))
        .SetReturn(1);

    // act
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_WRITEABLE, saved_ws_callback_context, NULL, 0);

    // assert
    mocks.AssertActualAndExpectedCalls();

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_117: [on_io_error should not be triggered twice when removing a pending IO that failed and a partial send for it has already been done.]  */
TEST_FUNCTION(when_removing_the_pending_IO_due_to_allocating_memory_failing_in_CLIENT_WRITABLE_fails_then_on_io_error_is_called_only_once)
{
    // arrange
    wsio_mocks mocks;
    unsigned char test_buffer[] = { 0x42, 0x43 };
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    STRICT_EXPECTED_CALL(mocks, lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_ESTABLISHED, saved_ws_callback_context, NULL, 0);
    (void)wsio_send(wsio, test_buffer, sizeof(test_buffer), test_on_send_complete, (void*)0x4243);
    STRICT_EXPECTED_CALL(mocks, lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    STRICT_EXPECTED_CALL(mocks, lws_write(TEST_LIBWEBSOCKET, IGNORED_PTR_ARG, sizeof(test_buffer), LWS_WRITE_BINARY))
        .ValidateArgumentBuffer(2, test_buffer, sizeof(test_buffer))
        .SetReturn(1);
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_WRITEABLE, saved_ws_callback_context, NULL, 0);
    mocks.ResetAllCalls();

    STRICT_EXPECTED_CALL(mocks, lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(mocks, lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    STRICT_EXPECTED_CALL(mocks, list_get_head_item(TEST_LIST_HANDLE));
    EXPECTED_CALL(mocks, list_item_get_value(IGNORED_PTR_ARG));
    EXPECTED_CALL(mocks, gballoc_malloc(IGNORED_NUM_ARG))
        .SetReturn((void*)NULL);
    STRICT_EXPECTED_CALL(mocks, test_on_send_complete((void*)0x4243, IO_SEND_ERROR));
    STRICT_EXPECTED_CALL(mocks, test_on_io_error((void*)0x4242));
    EXPECTED_CALL(mocks, gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(mocks, gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(mocks, list_remove(TEST_LIST_HANDLE, IGNORED_PTR_ARG))
        .SetReturn(1);

    // act
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_WRITEABLE, saved_ws_callback_context, NULL, 0);

    // assert
    mocks.AssertActualAndExpectedCalls();

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_080: [If lws_write succeeds and less bytes than the complete payload have been sent, then the sent bytes shall be removed from the pending IO and only the leftover bytes shall be left as pending and sent upon subsequent events.] */
TEST_FUNCTION(sending_partial_content_leaves_the_bytes_for_the_next_writable_event)
{
    // arrange
    wsio_mocks mocks;
    unsigned char test_buffer[] = { 0x42, 0x43 };
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    STRICT_EXPECTED_CALL(mocks, lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_ESTABLISHED, saved_ws_callback_context, NULL, 0);
    (void)wsio_send(wsio, test_buffer, sizeof(test_buffer), test_on_send_complete, (void*)0x4243);
    STRICT_EXPECTED_CALL(mocks, lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    STRICT_EXPECTED_CALL(mocks, lws_write(TEST_LIBWEBSOCKET, IGNORED_PTR_ARG, sizeof(test_buffer), LWS_WRITE_BINARY))
        .ValidateArgumentBuffer(2, test_buffer, sizeof(test_buffer))
        .SetReturn(1);
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_WRITEABLE, saved_ws_callback_context, NULL, 0);
    mocks.ResetAllCalls();

    STRICT_EXPECTED_CALL(mocks, lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(mocks, lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    STRICT_EXPECTED_CALL(mocks, list_get_head_item(TEST_LIST_HANDLE));
    EXPECTED_CALL(mocks, list_item_get_value(IGNORED_PTR_ARG));
    EXPECTED_CALL(mocks, gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(mocks, lws_write(TEST_LIBWEBSOCKET, IGNORED_PTR_ARG, 1, LWS_WRITE_BINARY))
        .ValidateArgumentBuffer(2, test_buffer + 1, 1)
        .SetReturn(1);
    EXPECTED_CALL(mocks, gballoc_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(mocks, test_on_send_complete((void*)0x4243, IO_SEND_OK));
    EXPECTED_CALL(mocks, gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(mocks, gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(mocks, list_remove(TEST_LIST_HANDLE, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(mocks, list_get_head_item(TEST_LIST_HANDLE));

    // act
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_WRITEABLE, saved_ws_callback_context, NULL, 0);

    // assert
    mocks.AssertActualAndExpectedCalls();

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_080: [If lws_write succeeds and less bytes than the complete payload have been sent, then the sent bytes shall be removed from the pending IO and only the leftover bytes shall be left as pending and sent upon subsequent events.] */
TEST_FUNCTION(sending_partial_content_of_2_bytes_works_and_leaves_the_bytes_for_the_next_writable_event)
{
    // arrange
    wsio_mocks mocks;
    unsigned char test_buffer[] = { 0x42, 0x43, 0x44 };
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    STRICT_EXPECTED_CALL(mocks, lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_ESTABLISHED, saved_ws_callback_context, NULL, 0);
    (void)wsio_send(wsio, test_buffer, sizeof(test_buffer), test_on_send_complete, (void*)0x4243);
    STRICT_EXPECTED_CALL(mocks, lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    STRICT_EXPECTED_CALL(mocks, lws_write(TEST_LIBWEBSOCKET, IGNORED_PTR_ARG, sizeof(test_buffer), LWS_WRITE_BINARY))
        .ValidateArgumentBuffer(2, test_buffer, sizeof(test_buffer))
        .SetReturn(2);
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_WRITEABLE, saved_ws_callback_context, NULL, 0);
    mocks.ResetAllCalls();

    STRICT_EXPECTED_CALL(mocks, lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(mocks, lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    STRICT_EXPECTED_CALL(mocks, list_get_head_item(TEST_LIST_HANDLE));
    EXPECTED_CALL(mocks, list_item_get_value(IGNORED_PTR_ARG));
    EXPECTED_CALL(mocks, gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(mocks, lws_write(TEST_LIBWEBSOCKET, IGNORED_PTR_ARG, 1, LWS_WRITE_BINARY))
        .ValidateArgumentBuffer(2, test_buffer + 2, 1)
        .SetReturn(1);
    EXPECTED_CALL(mocks, gballoc_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(mocks, test_on_send_complete((void*)0x4243, IO_SEND_OK));
    EXPECTED_CALL(mocks, gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(mocks, gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(mocks, list_remove(TEST_LIST_HANDLE, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(mocks, list_get_head_item(TEST_LIST_HANDLE));

    // act
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_WRITEABLE, saved_ws_callback_context, NULL, 0);

    // assert
    mocks.AssertActualAndExpectedCalls();

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_118: [If lws_write indicates more bytes sent than were passed to it an error shall be indicated via on_io_error.] */
TEST_FUNCTION(when_more_bytes_than_requested_are_indicated_by_lws_write_on_send_complete_indicates_an_error)
{
    // arrange
    wsio_mocks mocks;
    unsigned char test_buffer[] = { 0x42 };
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    STRICT_EXPECTED_CALL(mocks, lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_ESTABLISHED, saved_ws_callback_context, NULL, 0);
    (void)wsio_send(wsio, test_buffer, sizeof(test_buffer), test_on_send_complete, (void*)0x4243);
    mocks.ResetAllCalls();

    STRICT_EXPECTED_CALL(mocks, lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(mocks, lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    STRICT_EXPECTED_CALL(mocks, list_get_head_item(TEST_LIST_HANDLE));
    EXPECTED_CALL(mocks, list_item_get_value(IGNORED_PTR_ARG));
    EXPECTED_CALL(mocks, gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(mocks, lws_write(TEST_LIBWEBSOCKET, IGNORED_PTR_ARG, sizeof(test_buffer), LWS_WRITE_BINARY))
        .ValidateArgumentBuffer(2, test_buffer, sizeof(test_buffer))
        .SetReturn(2);
    EXPECTED_CALL(mocks, gballoc_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(mocks, test_on_send_complete((void*)0x4243, IO_SEND_ERROR));
    EXPECTED_CALL(mocks, gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(mocks, gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(mocks, list_remove(TEST_LIST_HANDLE, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(mocks, list_get_head_item(TEST_LIST_HANDLE))
        .SetReturn((LIST_ITEM_HANDLE)NULL);

    // act
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_WRITEABLE, saved_ws_callback_context, NULL, 0);

    // assert
    mocks.AssertActualAndExpectedCalls();

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_119: [If this error happens after the pending IO being partially sent, the on_io_error shall also be indicated.] */
TEST_FUNCTION(when_more_bytes_than_requested_are_indicated_by_lws_write_and_a_partial_send_has_been_done_then_on_io_error_is_triggered)
{
    // arrange
    wsio_mocks mocks;
    unsigned char test_buffer[] = { 0x42, 0x43 };
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    STRICT_EXPECTED_CALL(mocks, lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_ESTABLISHED, saved_ws_callback_context, NULL, 0);
    (void)wsio_send(wsio, test_buffer, sizeof(test_buffer), test_on_send_complete, (void*)0x4243);

    STRICT_EXPECTED_CALL(mocks, lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    STRICT_EXPECTED_CALL(mocks, lws_write(TEST_LIBWEBSOCKET, IGNORED_PTR_ARG, sizeof(test_buffer), LWS_WRITE_BINARY))
        .ValidateArgumentBuffer(2, test_buffer, sizeof(test_buffer))
        .SetReturn(1);
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_WRITEABLE, saved_ws_callback_context, NULL, 0);
    mocks.ResetAllCalls();

    STRICT_EXPECTED_CALL(mocks, lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(mocks, lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    STRICT_EXPECTED_CALL(mocks, list_get_head_item(TEST_LIST_HANDLE));
    EXPECTED_CALL(mocks, list_item_get_value(IGNORED_PTR_ARG));
    EXPECTED_CALL(mocks, gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(mocks, lws_write(TEST_LIBWEBSOCKET, IGNORED_PTR_ARG, 1, LWS_WRITE_BINARY))
        .ValidateArgumentBuffer(2, test_buffer + 1, 1)
        .SetReturn(2);
    EXPECTED_CALL(mocks, gballoc_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(mocks, test_on_send_complete((void*)0x4243, IO_SEND_ERROR));
    STRICT_EXPECTED_CALL(mocks, test_on_io_error((void*)0x4242));
    EXPECTED_CALL(mocks, gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(mocks, gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(mocks, list_remove(TEST_LIST_HANDLE, IGNORED_PTR_ARG));

    // act
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_WRITEABLE, saved_ws_callback_context, NULL, 0);

    // assert
    mocks.AssertActualAndExpectedCalls();

    // cleanup
    wsio_destroy(wsio);
}

/* LWS_CALLBACK_CLIENT_RECEIVE */

/* Tests_SRS_WSIO_01_082: [LWS_CALLBACK_CLIENT_RECEIVE shall only be processed when the IO is open.] */
/* Tests_SRS_WSIO_01_083: [When LWS_CALLBACK_CLIENT_RECEIVE is triggered and the IO is open, the on_bytes_received callback passed in wsio_open shall be called.] */
/* Tests_SRS_WSIO_01_084: [The bytes argument shall point to the received bytes as indicated by the LWS_CALLBACK_CLIENT_RECEIVE in argument.] */
TEST_FUNCTION(CLIENT_RECEIVE_while_the_IO_is_open_indicates_the_byte_as_received)
{
    // arrange
    wsio_mocks mocks;
    unsigned char test_buffer[] = { 0x42 };
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    STRICT_EXPECTED_CALL(mocks, lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_ESTABLISHED, saved_ws_callback_context, NULL, 0);
    mocks.ResetAllCalls();

    STRICT_EXPECTED_CALL(mocks, lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(mocks, lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    STRICT_EXPECTED_CALL(mocks, test_on_bytes_received((void*)0x4242, IGNORED_PTR_ARG, sizeof(test_buffer)))
        .ValidateArgumentBuffer(2, test_buffer, sizeof(test_buffer));

    // act
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_RECEIVE, saved_ws_callback_context, test_buffer, sizeof(test_buffer));

    // assert
    mocks.AssertActualAndExpectedCalls();

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_082: [LWS_CALLBACK_CLIENT_RECEIVE shall only be processed when the IO is open.] */
/* Tests_SRS_WSIO_01_083: [When LWS_CALLBACK_CLIENT_RECEIVE is triggered and the IO is open, the on_bytes_received callback passed in wsio_open shall be called.] */
/* Tests_SRS_WSIO_01_084: [The bytes argument shall point to the received bytes as indicated by the LWS_CALLBACK_CLIENT_RECEIVE in argument.] */
/* Tests_SRS_WSIO_01_085: [The length argument shall be set to the number of received bytes as indicated by the LWS_CALLBACK_CLIENT_RECEIVE len argument.] */
TEST_FUNCTION(CLIENT_RECEIVE_while_the_IO_is_open_indicates_the_2_bytes_as_received)
{
    // arrange
    wsio_mocks mocks;
    unsigned char test_buffer[] = { 0x42, 0x43 };
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    STRICT_EXPECTED_CALL(mocks, lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_ESTABLISHED, saved_ws_callback_context, NULL, 0);
    mocks.ResetAllCalls();

    STRICT_EXPECTED_CALL(mocks, lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(mocks, lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    STRICT_EXPECTED_CALL(mocks, test_on_bytes_received((void*)0x4242, IGNORED_PTR_ARG, sizeof(test_buffer)))
        .ValidateArgumentBuffer(2, test_buffer, sizeof(test_buffer));

    // act
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_RECEIVE, saved_ws_callback_context, test_buffer, sizeof(test_buffer));

    // assert
    mocks.AssertActualAndExpectedCalls();

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_122: [If an open action is in progress then the on_open_complete callback shall be invoked with IO_OPEN_ERROR.] */
TEST_FUNCTION(CLIENT_RECEIVE_while_opening_triggers_an_open_complete_with_IO_OPEN_ERROR)
{
    // arrange
    wsio_mocks mocks;
    unsigned char test_buffer[] = { 0x42 };
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    mocks.ResetAllCalls();

    STRICT_EXPECTED_CALL(mocks, lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(mocks, lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    STRICT_EXPECTED_CALL(mocks, test_on_io_open_complete((void*)0x4242, IO_OPEN_ERROR));
    STRICT_EXPECTED_CALL(mocks, lws_context_destroy(TEST_LIBWEBSOCKET_CONTEXT));

    // act
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_RECEIVE, saved_ws_callback_context, test_buffer, sizeof(test_buffer));

    // assert
    mocks.AssertActualAndExpectedCalls();

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_086: [The callback_context shall be set to the callback_context that was passed in wsio_open.] */
TEST_FUNCTION(CLIENT_RECEIVE_passes_the_proper_context_to_on_bytes_received)
{
    // arrange
    wsio_mocks mocks;
    unsigned char test_buffer[] = { 0x42, 0x43 };
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4546, test_on_bytes_received, (void*)0x4546, test_on_io_error, (void*)0x4546);
    STRICT_EXPECTED_CALL(mocks, lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_ESTABLISHED, saved_ws_callback_context, NULL, 0);
    mocks.ResetAllCalls();

    STRICT_EXPECTED_CALL(mocks, lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(mocks, lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    STRICT_EXPECTED_CALL(mocks, test_on_bytes_received((void*)0x4546, IGNORED_PTR_ARG, sizeof(test_buffer)))
        .ValidateArgumentBuffer(2, test_buffer, sizeof(test_buffer));

    // act
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_RECEIVE, saved_ws_callback_context, test_buffer, sizeof(test_buffer));

    // assert
    mocks.AssertActualAndExpectedCalls();

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_087: [If the number of bytes is 0 then the on_io_error callback shall be called.] */
TEST_FUNCTION(CLIENT_RECEIVE_with_0_bytes_yields_an_error)
{
    // arrange
    wsio_mocks mocks;
    unsigned char test_buffer[] = { 0x42, 0x43 };
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4546, test_on_bytes_received, (void*)0x4546, test_on_io_error, (void*)0x4546);
    STRICT_EXPECTED_CALL(mocks, lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_ESTABLISHED, saved_ws_callback_context, NULL, 0);
    mocks.ResetAllCalls();

    STRICT_EXPECTED_CALL(mocks, lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(mocks, lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    STRICT_EXPECTED_CALL(mocks, test_on_io_error((void*)0x4546));

    // act
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_RECEIVE, saved_ws_callback_context, test_buffer, 0);

    // assert
    mocks.AssertActualAndExpectedCalls();

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_088: [If the number of bytes received is positive, but the buffer indicated by the in parameter is NULL, then the on_io_error callback shall be called.] */
TEST_FUNCTION(CLIENT_RECEIVE_with_NULL_input_buffer_yields_an_error)
{
    // arrange
    wsio_mocks mocks;
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4546, test_on_bytes_received, (void*)0x4546, test_on_io_error, (void*)0x4546);
    STRICT_EXPECTED_CALL(mocks, lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_ESTABLISHED, saved_ws_callback_context, NULL, 0);
    mocks.ResetAllCalls();

    STRICT_EXPECTED_CALL(mocks, lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(mocks, lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    STRICT_EXPECTED_CALL(mocks, test_on_io_error((void*)0x4546));

    // act
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_RECEIVE, saved_ws_callback_context, NULL, 1);

    // assert
    mocks.AssertActualAndExpectedCalls();

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_082: [LWS_CALLBACK_CLIENT_RECEIVE shall only be processed when the IO is open.] */
/* Tests_SRS_WSIO_01_083: [When LWS_CALLBACK_CLIENT_RECEIVE is triggered and the IO is open, the on_bytes_received callback passed in wsio_open shall be called.] */
/* Tests_SRS_WSIO_01_084: [The bytes argument shall point to the received bytes as indicated by the LWS_CALLBACK_CLIENT_RECEIVE in argument.] */
/* Tests_SRS_WSIO_01_085: [The length argument shall be set to the number of received bytes as indicated by the LWS_CALLBACK_CLIENT_RECEIVE len argument.] */
TEST_FUNCTION(CLIENT_RECEIVE_twice_indicates_2_receives)
{
    // arrange
    wsio_mocks mocks;
    unsigned char test_buffer1[] = { 0x42, 0x43 };
    unsigned char test_buffer2[] = { 0x44 };
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    STRICT_EXPECTED_CALL(mocks, lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_ESTABLISHED, saved_ws_callback_context, NULL, 0);
    mocks.ResetAllCalls();

    STRICT_EXPECTED_CALL(mocks, lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(mocks, lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    STRICT_EXPECTED_CALL(mocks, test_on_bytes_received((void*)0x4242, IGNORED_PTR_ARG, sizeof(test_buffer1)))
        .ValidateArgumentBuffer(2, test_buffer1, sizeof(test_buffer1));
    STRICT_EXPECTED_CALL(mocks, lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(mocks, lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    STRICT_EXPECTED_CALL(mocks, test_on_bytes_received((void*)0x4242, IGNORED_PTR_ARG, sizeof(test_buffer2)))
        .ValidateArgumentBuffer(2, test_buffer2, sizeof(test_buffer2));

    // act
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_RECEIVE, saved_ws_callback_context, test_buffer1, sizeof(test_buffer1));
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_RECEIVE, saved_ws_callback_context, test_buffer2, sizeof(test_buffer2));

    // assert
    mocks.AssertActualAndExpectedCalls();

    // cleanup
    wsio_destroy(wsio);
}

/* LWS_CALLBACK_OPENSSL_LOAD_EXTRA_CLIENT_VERIFY_CERTS */

/* Tests_SRS_WSIO_01_089: [When LWS_CALLBACK_OPENSSL_LOAD_EXTRA_CLIENT_VERIFY_CERTS is triggered, the certificates passed in the trusted_ca member of WSIO_CONFIG passed in wsio_init shall be loaded in the certificate store.] */
/* Tests_SRS_WSIO_01_090: [The OpenSSL certificate store is passed in the user argument.] */
/* Tests_SRS_WSIO_01_131: [Get the certificate store for the OpenSSL context by calling SSL_CTX_get_cert_store] */
/* Tests_SRS_WSIO_01_123: [Creating a new BIO by calling BIO_new.] */
/* Tests_SRS_WSIO_01_124: [The BIO shall be a memory one (obtained by calling BIO_s_mem).] */
/* Tests_SRS_WSIO_01_125: [Setting the certificates string as the input by using BIO_puts.] */
/* Tests_SRS_WSIO_01_126: [Reading every certificate by calling PEM_read_bio_X509] */
/* Tests_SRS_WSIO_01_132: [When PEM_read_bio_X509 returns NULL then no more certificates are available in the input string.] */
/* Tests_SRS_WSIO_01_128: [Freeing the BIO] */
TEST_FUNCTION(LOAD_EXTRA_CLIENT_VERIFY_CERTS_with_a_string_that_has_no_certs_add_no_certs_to_the_store)
{
    // arrange
    wsio_mocks mocks;
    unsigned char test_buffer1[] = { 0x42, 0x43 };
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    mocks.ResetAllCalls();

    STRICT_EXPECTED_CALL(mocks, lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(mocks, lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    STRICT_EXPECTED_CALL(mocks, SSL_CTX_get_cert_store(TEST_SSL_CONTEXT));
    STRICT_EXPECTED_CALL(mocks, BIO_s_mem());
    STRICT_EXPECTED_CALL(mocks, BIO_new(TEST_BIO_METHOD));
    STRICT_EXPECTED_CALL(mocks, BIO_puts(TEST_BIO, default_wsio_config.trusted_ca))
        .SetReturn((int)strlen(default_wsio_config.trusted_ca));
    STRICT_EXPECTED_CALL(mocks, PEM_read_bio_X509(TEST_BIO, NULL, NULL, NULL))
        .SetReturn((X509*)NULL);
    STRICT_EXPECTED_CALL(mocks, BIO_free(TEST_BIO));

    // act
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_OPENSSL_LOAD_EXTRA_CLIENT_VERIFY_CERTS, (void*)TEST_SSL_CONTEXT, NULL, 0);

    // assert
    mocks.AssertActualAndExpectedCalls();

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_126: [Reading every certificate by calling PEM_read_bio_X509] */
/* Tests_SRS_WSIO_01_127: [Adding the read certificate to the store by calling X509_STORE_add_cert]  */
TEST_FUNCTION(LOAD_EXTRA_CLIENT_VERIFY_CERTS_with_a_string_that_has_1_cert_loads_that_cert)
{
    // arrange
    wsio_mocks mocks;
    unsigned char test_buffer1[] = { 0x42, 0x43 };
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    mocks.ResetAllCalls();

    STRICT_EXPECTED_CALL(mocks, lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(mocks, lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    STRICT_EXPECTED_CALL(mocks, SSL_CTX_get_cert_store(TEST_SSL_CONTEXT));
    STRICT_EXPECTED_CALL(mocks, BIO_s_mem());
    STRICT_EXPECTED_CALL(mocks, BIO_new(TEST_BIO_METHOD));
    STRICT_EXPECTED_CALL(mocks, BIO_puts(TEST_BIO, default_wsio_config.trusted_ca))
        .SetReturn((int)strlen(default_wsio_config.trusted_ca));
    STRICT_EXPECTED_CALL(mocks, PEM_read_bio_X509(TEST_BIO, NULL, NULL, NULL))
        .SetReturn(TEST_X509_CERT_1);
    STRICT_EXPECTED_CALL(mocks, X509_STORE_add_cert(TEST_CERT_STORE, TEST_X509_CERT_1));
    STRICT_EXPECTED_CALL(mocks, PEM_read_bio_X509(TEST_BIO, NULL, NULL, NULL))
        .SetReturn((X509*)NULL);
    STRICT_EXPECTED_CALL(mocks, BIO_free(TEST_BIO));

    // act
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_OPENSSL_LOAD_EXTRA_CLIENT_VERIFY_CERTS, (void*)TEST_SSL_CONTEXT, NULL, 0);

    // assert
    mocks.AssertActualAndExpectedCalls();

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_126: [Reading every certificate by calling PEM_read_bio_X509] */
/* Tests_SRS_WSIO_01_127: [Adding the read certificate to the store by calling X509_STORE_add_cert] */
TEST_FUNCTION(LOAD_EXTRA_CLIENT_VERIFY_CERTS_with_a_string_that_has_2_certs_loads_that_certs)
{
    // arrange
    wsio_mocks mocks;
    unsigned char test_buffer1[] = { 0x42, 0x43 };
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    mocks.ResetAllCalls();

    STRICT_EXPECTED_CALL(mocks, lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(mocks, lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    STRICT_EXPECTED_CALL(mocks, SSL_CTX_get_cert_store(TEST_SSL_CONTEXT));
    STRICT_EXPECTED_CALL(mocks, BIO_s_mem());
    STRICT_EXPECTED_CALL(mocks, BIO_new(TEST_BIO_METHOD));
    STRICT_EXPECTED_CALL(mocks, BIO_puts(TEST_BIO, default_wsio_config.trusted_ca))
        .SetReturn((int)strlen(default_wsio_config.trusted_ca));
    STRICT_EXPECTED_CALL(mocks, PEM_read_bio_X509(TEST_BIO, NULL, NULL, NULL))
        .SetReturn(TEST_X509_CERT_1);
    STRICT_EXPECTED_CALL(mocks, X509_STORE_add_cert(TEST_CERT_STORE, TEST_X509_CERT_1));
    STRICT_EXPECTED_CALL(mocks, PEM_read_bio_X509(TEST_BIO, NULL, NULL, NULL))
        .SetReturn(TEST_X509_CERT_2);
    STRICT_EXPECTED_CALL(mocks, X509_STORE_add_cert(TEST_CERT_STORE, TEST_X509_CERT_2));
    STRICT_EXPECTED_CALL(mocks, PEM_read_bio_X509(TEST_BIO, NULL, NULL, NULL))
        .SetReturn((X509*)NULL);
    STRICT_EXPECTED_CALL(mocks, BIO_free(TEST_BIO));

    // act
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_OPENSSL_LOAD_EXTRA_CLIENT_VERIFY_CERTS, (void*)TEST_SSL_CONTEXT, NULL, 0);

    // assert
    mocks.AssertActualAndExpectedCalls();

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_130: [If the event is received when the IO is already open the on_io_error callback shall be triggered.] */
TEST_FUNCTION(LOAD_EXTRA_CLIENT_VERIFY_CERTS_when_already_open_yields_an_error)
{
    // arrange
    wsio_mocks mocks;
    unsigned char test_buffer1[] = { 0x42, 0x43 };
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    STRICT_EXPECTED_CALL(mocks, lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_ESTABLISHED, saved_ws_callback_context, NULL, 0);
    mocks.ResetAllCalls();

    STRICT_EXPECTED_CALL(mocks, lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(mocks, lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    STRICT_EXPECTED_CALL(mocks, test_on_io_error((void*)0x4242));

    // act
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_OPENSSL_LOAD_EXTRA_CLIENT_VERIFY_CERTS, (void*)TEST_SSL_CONTEXT, NULL, 0);

    // assert
    mocks.AssertActualAndExpectedCalls();

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_129: [If any of the APIs fails and an open call is pending the on_open_complete callback shall be triggered with IO_OPEN_ERROR.] */
TEST_FUNCTION(when_getting_the_cert_store_fails_in_LOAD_EXTRA_CLIENT_VERIFY_CERTS_an_on_open_complete_is_triggered_with_error)
{
    // arrange
    wsio_mocks mocks;
    unsigned char test_buffer1[] = { 0x42, 0x43 };
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    mocks.ResetAllCalls();

    STRICT_EXPECTED_CALL(mocks, lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(mocks, lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    STRICT_EXPECTED_CALL(mocks, SSL_CTX_get_cert_store(TEST_SSL_CONTEXT))
        .SetReturn((X509_STORE*)NULL);
    STRICT_EXPECTED_CALL(mocks, test_on_io_open_complete((void*)0x4242, IO_OPEN_ERROR));

    // act
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_OPENSSL_LOAD_EXTRA_CLIENT_VERIFY_CERTS, (void*)TEST_SSL_CONTEXT, NULL, 0);

    // assert
    mocks.AssertActualAndExpectedCalls();

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_129: [If any of the APIs fails and an open call is pending the on_open_complete callback shall be triggered with IO_OPEN_ERROR.] */
TEST_FUNCTION(when_getting_the_memory_BIO_METHOD_fails_in_LOAD_EXTRA_CLIENT_VERIFY_CERTS_an_on_open_complete_is_triggered_with_error)
{
    // arrange
    wsio_mocks mocks;
    unsigned char test_buffer1[] = { 0x42, 0x43 };
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    mocks.ResetAllCalls();

    STRICT_EXPECTED_CALL(mocks, lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(mocks, lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    STRICT_EXPECTED_CALL(mocks, SSL_CTX_get_cert_store(TEST_SSL_CONTEXT));
    STRICT_EXPECTED_CALL(mocks, BIO_s_mem())
        .SetReturn((BIO_METHOD*)NULL);
    STRICT_EXPECTED_CALL(mocks, test_on_io_open_complete((void*)0x4242, IO_OPEN_ERROR));

    // act
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_OPENSSL_LOAD_EXTRA_CLIENT_VERIFY_CERTS, (void*)TEST_SSL_CONTEXT, NULL, 0);

    // assert
    mocks.AssertActualAndExpectedCalls();

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_129: [If any of the APIs fails and an open call is pending the on_open_complete callback shall be triggered with IO_OPEN_ERROR.] */
TEST_FUNCTION(when_creating_the_BIO_fails_in_LOAD_EXTRA_CLIENT_VERIFY_CERTS_an_on_open_complete_is_triggered_with_error)
{
    // arrange
    wsio_mocks mocks;
    unsigned char test_buffer1[] = { 0x42, 0x43 };
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    mocks.ResetAllCalls();

    STRICT_EXPECTED_CALL(mocks, lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(mocks, lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    STRICT_EXPECTED_CALL(mocks, SSL_CTX_get_cert_store(TEST_SSL_CONTEXT));
    STRICT_EXPECTED_CALL(mocks, BIO_s_mem());
    STRICT_EXPECTED_CALL(mocks, BIO_new(TEST_BIO_METHOD))
        .SetReturn((BIO*)NULL);
    STRICT_EXPECTED_CALL(mocks, test_on_io_open_complete((void*)0x4242, IO_OPEN_ERROR));

    // act
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_OPENSSL_LOAD_EXTRA_CLIENT_VERIFY_CERTS, (void*)TEST_SSL_CONTEXT, NULL, 0);

    // assert
    mocks.AssertActualAndExpectedCalls();

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_129: [If any of the APIs fails and an open call is pending the on_open_complete callback shall be triggered with IO_OPEN_ERROR.] */
TEST_FUNCTION(when_setting_the_input_string_for_the_memory_BIO_fails_with_0_in_LOAD_EXTRA_CLIENT_VERIFY_CERTS_an_on_open_complete_is_triggered_with_error)
{
    // arrange
    wsio_mocks mocks;
    unsigned char test_buffer1[] = { 0x42, 0x43 };
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    mocks.ResetAllCalls();

    STRICT_EXPECTED_CALL(mocks, lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(mocks, lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    STRICT_EXPECTED_CALL(mocks, SSL_CTX_get_cert_store(TEST_SSL_CONTEXT));
    STRICT_EXPECTED_CALL(mocks, BIO_s_mem());
    STRICT_EXPECTED_CALL(mocks, BIO_new(TEST_BIO_METHOD));
    STRICT_EXPECTED_CALL(mocks, BIO_puts(TEST_BIO, default_wsio_config.trusted_ca))
        .SetReturn(0);
    STRICT_EXPECTED_CALL(mocks, BIO_free(TEST_BIO));
    STRICT_EXPECTED_CALL(mocks, test_on_io_open_complete((void*)0x4242, IO_OPEN_ERROR));

    // act
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_OPENSSL_LOAD_EXTRA_CLIENT_VERIFY_CERTS, (void*)TEST_SSL_CONTEXT, NULL, 0);

    // assert
    mocks.AssertActualAndExpectedCalls();

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_129: [If any of the APIs fails and an open call is pending the on_open_complete callback shall be triggered with IO_OPEN_ERROR.] */
TEST_FUNCTION(when_setting_the_input_string_for_the_memory_BIO_fails_with_len_minus_1_in_LOAD_EXTRA_CLIENT_VERIFY_CERTS_an_on_open_complete_is_triggered_with_error)
{
    // arrange
    wsio_mocks mocks;
    unsigned char test_buffer1[] = { 0x42, 0x43 };
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    mocks.ResetAllCalls();

    STRICT_EXPECTED_CALL(mocks, lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(mocks, lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    STRICT_EXPECTED_CALL(mocks, SSL_CTX_get_cert_store(TEST_SSL_CONTEXT));
    STRICT_EXPECTED_CALL(mocks, BIO_s_mem());
    STRICT_EXPECTED_CALL(mocks, BIO_new(TEST_BIO_METHOD));
    STRICT_EXPECTED_CALL(mocks, BIO_puts(TEST_BIO, default_wsio_config.trusted_ca))
        .SetReturn((int)strlen(default_wsio_config.trusted_ca) - 1);
    STRICT_EXPECTED_CALL(mocks, test_on_io_open_complete((void*)0x4242, IO_OPEN_ERROR));
    STRICT_EXPECTED_CALL(mocks, BIO_free(TEST_BIO));

    // act
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_OPENSSL_LOAD_EXTRA_CLIENT_VERIFY_CERTS, (void*)TEST_SSL_CONTEXT, NULL, 0);

    // assert
    mocks.AssertActualAndExpectedCalls();

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_129: [If any of the APIs fails and an open call is pending the on_open_complete callback shall be triggered with IO_OPEN_ERROR.] */
TEST_FUNCTION(when_setting_the_input_string_for_the_memory_BIO_fails_with_len_plus_1_in_LOAD_EXTRA_CLIENT_VERIFY_CERTS_an_on_open_complete_is_triggered_with_error)
{
    // arrange
    wsio_mocks mocks;
    unsigned char test_buffer1[] = { 0x42, 0x43 };
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    mocks.ResetAllCalls();

    STRICT_EXPECTED_CALL(mocks, lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(mocks, lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    STRICT_EXPECTED_CALL(mocks, SSL_CTX_get_cert_store(TEST_SSL_CONTEXT));
    STRICT_EXPECTED_CALL(mocks, BIO_s_mem());
    STRICT_EXPECTED_CALL(mocks, BIO_new(TEST_BIO_METHOD));
    STRICT_EXPECTED_CALL(mocks, BIO_puts(TEST_BIO, default_wsio_config.trusted_ca))
        .SetReturn((int)strlen(default_wsio_config.trusted_ca) + 1);
    STRICT_EXPECTED_CALL(mocks, BIO_free(TEST_BIO));
    STRICT_EXPECTED_CALL(mocks, test_on_io_open_complete((void*)0x4242, IO_OPEN_ERROR));

    // act
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_OPENSSL_LOAD_EXTRA_CLIENT_VERIFY_CERTS, (void*)TEST_SSL_CONTEXT, NULL, 0);

    // assert
    mocks.AssertActualAndExpectedCalls();

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_129: [If any of the APIs fails and an open call is pending the on_open_complete callback shall be triggered with IO_OPEN_ERROR.] */
/* Tests_SRS_WSIO_01_133: [If X509_STORE_add_cert fails then the certificate obtained by calling PEM_read_bio_X509 shall be freed with X509_free.] */
TEST_FUNCTION(when_adding_the_cert_to_the_store_fails_in_LOAD_EXTRA_CLIENT_VERIFY_CERTS_the_cert_is_freed_and_on_open_complete_is_triggered_with_error)
{
    // arrange
    wsio_mocks mocks;
    unsigned char test_buffer1[] = { 0x42, 0x43 };

    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    mocks.ResetAllCalls();

    STRICT_EXPECTED_CALL(mocks, lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(mocks, lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    STRICT_EXPECTED_CALL(mocks, SSL_CTX_get_cert_store(TEST_SSL_CONTEXT));
    STRICT_EXPECTED_CALL(mocks, BIO_s_mem());
    STRICT_EXPECTED_CALL(mocks, BIO_new(TEST_BIO_METHOD));
    STRICT_EXPECTED_CALL(mocks, BIO_puts(TEST_BIO, default_wsio_config.trusted_ca))
        .SetReturn((int)strlen(default_wsio_config.trusted_ca));
    STRICT_EXPECTED_CALL(mocks, PEM_read_bio_X509(TEST_BIO, NULL, NULL, NULL))
        .SetReturn(TEST_X509_CERT_1);
    STRICT_EXPECTED_CALL(mocks, X509_STORE_add_cert(TEST_CERT_STORE, TEST_X509_CERT_1))
        .SetReturn(0);
    STRICT_EXPECTED_CALL(mocks, X509_free(TEST_X509_CERT_1));
    STRICT_EXPECTED_CALL(mocks, BIO_free(TEST_BIO));
    STRICT_EXPECTED_CALL(mocks, test_on_io_open_complete((void*)0x4242, IO_OPEN_ERROR));

    // act
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_OPENSSL_LOAD_EXTRA_CLIENT_VERIFY_CERTS, (void*)TEST_SSL_CONTEXT, NULL, 0);

    // assert
    mocks.AssertActualAndExpectedCalls();

    // cleanup
    wsio_destroy(wsio);
}

/*Tests_SRS_WSIO_02_001: [ If parameter handle is NULL then wsio_retrieveoptions shall fail and return NULL. ]*/
TEST_FUNCTION(wsio_retrieveoptions_with_NULL_handle_returns_NULL)
{
    ///arrange
    wsio_mocks mocks;

    ///act
    OPTIONHANDLER_HANDLE h = wsio_get_interface_description()->concrete_io_retrieveoptions(NULL);

    ///assert
    ASSERT_IS_NULL(h);
    mocks.AssertActualAndExpectedCalls();

}

/*Tests_SRS_WSIO_02_002: [ wsio_retrieveoptions shall produce an empty OPTIOHANDLER_HANDLE. ]*/
TEST_FUNCTION(wsio_retrieveoptions_returns_non_NULL_empty)
{
    ///arrange
    wsio_mocks mocks;
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    mocks.ResetAllCalls();

    STRICT_EXPECTED_CALL(mocks, OptionHandler_Create(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreAllArguments();

    ///act
    OPTIONHANDLER_HANDLE h = wsio_get_interface_description()->concrete_io_retrieveoptions(wsio);

    ///assert
    ASSERT_IS_NOT_NULL(h);
    mocks.AssertActualAndExpectedCalls();

    ///cleanup
    wsio_destroy(wsio);
    free(h);
}

/*Tests_SRS_WSIO_02_003: [ If producing the OPTIONHANDLER_HANDLE fails then wsio_retrieveoptions shall fail and return NULL. ]*/
TEST_FUNCTION(wsio_retrieveoptions_when_OptionHandler_Create_fails_it_fails)
{
    ///arrange
    wsio_mocks mocks;
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    mocks.ResetAllCalls();

    STRICT_EXPECTED_CALL(mocks, OptionHandler_Create(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreAllArguments()
        .SetFailReturn((OPTIONHANDLER_HANDLE)NULL);

    ///act
    OPTIONHANDLER_HANDLE h = wsio_get_interface_description()->concrete_io_retrieveoptions(wsio);

    ///assert
    ASSERT_IS_NULL(h);
    mocks.AssertActualAndExpectedCalls();

    ///cleanup
    wsio_destroy(wsio);
}

END_TEST_SUITE(wsio_ut)
