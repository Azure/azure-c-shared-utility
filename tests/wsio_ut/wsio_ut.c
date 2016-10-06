// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "testrunnerswitcher.h"
#include "libwebsockets.h"
#include "openssl/ssl.h"

#include "umock_c.h"
#include "umocktypes_charptr.h"

#define ENABLE_MOCKS

#include "azure_c_shared_utility/xio.h"
#include "azure_c_shared_utility/optionhandler.h"
#include "azure_c_shared_utility/shared_util_options.h"
#include "azure_c_shared_utility/crt_abstractions.h"
#include "azure_c_shared_utility/singlylinkedlist.h"

static const void** list_items = NULL;
static size_t list_item_count = 0;
static const char* TEST_HOST_ADDRESS = "host_address.com";
static const char* TEST_USERNAME = "user_name";
static const char* TEST_PASSWORD = "user_pwd";

static const SINGLYLINKEDLIST_HANDLE TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE = (SINGLYLINKEDLIST_HANDLE)0x4242;
static const LIST_ITEM_HANDLE TEST_LIST_ITEM_HANDLE = (LIST_ITEM_HANDLE)0x11;
static struct lws_context* TEST_LIBWEBSOCKET_CONTEXT = (struct lws_context*)0x4243;
static void* TEST_USER_CONTEXT = (void*)0x4244;
static struct lws* TEST_LIBWEBSOCKET = (struct lws*)0x4245;
static struct lws_extension extensions[] = { { NULL } };
static struct lws_extension* TEST_INTERNAL_EXTENSIONS = extensions;
static BIO* TEST_BIO = (BIO*)0x4247;
static BIO_METHOD* TEST_BIO_METHOD = (BIO_METHOD*)0x4248;
static X509_STORE* TEST_CERT_STORE = (X509_STORE*)0x4249;
static X509* TEST_X509_CERT_1 = (X509*)0x424A;
static X509* TEST_X509_CERT_2 = (X509*)0x424B;
static const SSL_CTX* TEST_SSL_CONTEXT = (const SSL_CTX*)0x424C;
static callback_function* saved_ws_callback;
static void* saved_ws_callback_context;

#define TEST_LOCK_HANDLE (LOCK_HANDLE)0x4443

#define lws_write_protocol_VALUES \
    LWS_WRITE_TEXT, \
    LWS_WRITE_BINARY, \
    LWS_WRITE_CONTINUATION, \
    LWS_WRITE_HTTP, \
    LWS_WRITE_CLOSE, \
    LWS_WRITE_PING, \
    LWS_WRITE_PONG, \
    LWS_WRITE_HTTP_FINAL, \
    LWS_WRITE_HTTP_HEADERS, \
    LWS_WRITE_NO_FIN, \
    LWS_WRITE_CLIENT_IGNORE_XOR_MASK

typedef enum lws_write_protocol my_lws_write_protocol_enum;

TEST_DEFINE_ENUM_TYPE(IO_OPEN_RESULT, IO_OPEN_RESULT_VALUES);
IMPLEMENT_UMOCK_C_ENUM_TYPE(IO_OPEN_RESULT, IO_OPEN_RESULT_VALUES);
TEST_DEFINE_ENUM_TYPE(my_lws_write_protocol_enum, lws_write_protocol_VALUES);
IMPLEMENT_UMOCK_C_ENUM_TYPE(my_lws_write_protocol_enum, lws_write_protocol_VALUES);
TEST_DEFINE_ENUM_TYPE(IO_SEND_RESULT, IO_SEND_RESULT_VALUES);
IMPLEMENT_UMOCK_C_ENUM_TYPE(IO_SEND_RESULT, IO_SEND_RESULT_VALUES);
TEST_DEFINE_ENUM_TYPE(OPTIONHANDLER_RESULT, OPTIONHANDLER_RESULT_VALUES);
IMPLEMENT_UMOCK_C_ENUM_TYPE(OPTIONHANDLER_RESULT, OPTIONHANDLER_RESULT_VALUES);

static size_t currentmalloc_call;
static size_t whenShallmalloc_fail;

static void* my_gballoc_malloc(size_t size)
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

static void my_gballoc_free(void* ptr)
{
    free(ptr);
}

static char* umocktypes_stringify_const_struct_lws_context_creation_info_ptr(const struct lws_context_creation_info** value)
{
    char* result = NULL;
    char temp_buffer[1024];
    int length;
    length = sprintf(temp_buffer, "{ port = %d, iface = %s, protocols = %p, extensions = %p, token_limits = %p, ssl_private_key_password = %s, ssl_cert_filepath = %s, ssl_private_key_filepath = %s, ssl_ca_filepath = %s, ssl_cipher_list = %s, http_proxy_address = %s, http_proxy_port = %u, gid = %d, uid = %d, options = %u, user = %p, ka_time = %d, ka_probes = %d, ka_interval = %d, provided_client_ssl_ctx = %p }",
        (*value)->port,
        (*value)->iface,
        (*value)->protocols,
        (*value)->extensions,
        (*value)->token_limits,
        (*value)->ssl_private_key_password,
        (*value)->ssl_cert_filepath,
        (*value)->ssl_private_key_filepath,
        (*value)->ssl_ca_filepath,
        (*value)->ssl_cipher_list,
        (*value)->http_proxy_address,
        (*value)->http_proxy_port,
        (*value)->gid,
        (*value)->uid,
        (*value)->options,
        (*value)->user,
        (*value)->ka_time,
        (*value)->ka_probes,
        (*value)->ka_interval,
        (*value)->provided_client_ssl_ctx);

    if (length > 0)
    {
        result = (char*)malloc(strlen(temp_buffer) + 1);
        if (result != NULL)
        {
            (void)memcpy(result, temp_buffer, strlen(temp_buffer) + 1);
        }
    }

    return result;
}

static int umocktypes_are_equal_const_struct_lws_context_creation_info_ptr(const struct lws_context_creation_info** left, const struct lws_context_creation_info** right)
{
    int result;

    size_t currentProtocol = 0;

    if ((left == NULL) ||
        (right == NULL))
    {
        result = -1;
    }
    else
    {
        size_t extensions_count = 0;
        result = (*left)->gid == (*right)->gid;
        if (((*left)->extensions != NULL) &&
            ((*right)->extensions != NULL))
        {
            while ((*left)->extensions[extensions_count].name != NULL)
            {
                result = result && ((*left)->extensions[extensions_count].callback == (*right)->extensions[extensions_count].callback);
                result = result && ((*left)->extensions[extensions_count].per_context_private_data == (*right)->extensions[extensions_count].per_context_private_data);
                result = result && ((*left)->extensions[extensions_count].per_session_data_size == (*right)->extensions[extensions_count].per_session_data_size);
                result = result && (strcmp((*left)->extensions[extensions_count].name, (*right)->extensions[extensions_count].name) == 0);
                extensions_count++;
            }
        }
        else
        {
            result = result && ((*left)->extensions == (*right)->extensions);
        }
        result = result && ((*left)->token_limits == (*right)->token_limits);
        result = result && ((*left)->uid == (*right)->uid);
        result = result && ((*left)->ka_time == (*right)->ka_time);
        if ((*right)->ka_time != 0)
        {
            result = result && ((*left)->ka_interval == (*right)->ka_interval);
            result = result && ((*left)->ka_probes == (*right)->ka_probes);
        }

        result = result && ((*left)->options == (*right)->options);
        result = result && ((*left)->iface == (*right)->iface);
        result = result && ((*left)->port == (*right)->port);
        result = result && ((*left)->provided_client_ssl_ctx == (*right)->provided_client_ssl_ctx);
        if ((*right)->http_proxy_address == NULL)
        {
            result = result && ((*left)->http_proxy_address == (*right)->http_proxy_address);
        }
        else
        {
            result = result && (strcmp((*left)->http_proxy_address, (*right)->http_proxy_address) == 0);
            result = result && ((*left)->http_proxy_port == (*right)->http_proxy_port);
        }

        if ((*right)->ssl_ca_filepath == NULL)
        {
            result = result && ((*left)->ssl_ca_filepath == (*right)->ssl_ca_filepath);
        }
        else
        {
            result = result && (strcmp((*left)->ssl_ca_filepath, (*right)->ssl_ca_filepath) == 0);
        }

        if ((*right)->ssl_cert_filepath == NULL)
        {
            result = result && ((*left)->ssl_cert_filepath == (*right)->ssl_cert_filepath);
        }
        else
        {
            result = result && (strcmp((*left)->ssl_cert_filepath, (*right)->ssl_cert_filepath) == 0);
        }

        if ((*right)->ssl_cipher_list == NULL)
        {
            result = result && ((*left)->ssl_cipher_list == (*right)->ssl_cipher_list);
        }
        else
        {
            result = result && (strcmp((*left)->ssl_cipher_list, (*right)->ssl_cipher_list) == 0);
        }

        if ((*right)->ssl_private_key_filepath == NULL)
        {
            result = result && ((*left)->ssl_private_key_filepath == (*right)->ssl_private_key_filepath);
        }
        else
        {
            result = result && (strcmp((*left)->ssl_private_key_filepath, (*right)->ssl_private_key_filepath) == 0);
        }

        if ((*right)->ssl_private_key_password == NULL)
        {
            result = result && ((*left)->ssl_private_key_password == (*right)->ssl_private_key_password);
        }
        else
        {
            result = result && (strcmp((*left)->ssl_private_key_password, (*right)->ssl_private_key_password) == 0);
        }

        while ((*left)->protocols[currentProtocol].name != NULL)
        {
            result = result && ((*left)->protocols[currentProtocol].id == (*right)->protocols[currentProtocol].id);
            result = result && (strcmp((*left)->protocols[currentProtocol].name, (*right)->protocols[currentProtocol].name) == 0);
            result = result && ((*left)->protocols[currentProtocol].per_session_data_size == (*right)->protocols[currentProtocol].per_session_data_size);
            result = result && ((*left)->protocols[currentProtocol].rx_buffer_size == (*right)->protocols[currentProtocol].rx_buffer_size);
            result = result && ((*left)->protocols[currentProtocol].user == (*right)->protocols[currentProtocol].user);

            currentProtocol++;
        }

        if ((*right)->protocols[currentProtocol].name != NULL)
        {
            result = 0;
        }
    }

    return result;
}

static char* copy_string(const char* source)
{
    char* result;

    if (source == NULL)
    {
        result = NULL;
    }
    else
    {
        size_t length = strlen(source);
        result = (char*)malloc(length + 1);
        (void)memcpy(result, source, length + 1);
    }

    return result;
}

static int umocktypes_copy_const_struct_lws_context_creation_info_ptr(struct lws_context_creation_info** destination, const struct lws_context_creation_info** source)
{
    int result;

    *destination = (struct lws_context_creation_info*)malloc(sizeof(struct lws_context_creation_info));
    if (*destination == NULL)
    {
        result = __LINE__;
    }
    else
    {
        if ((*source)->protocols == NULL)
        {
            (*destination)->protocols = NULL;
        }
        else
        {
            size_t protocols_count = 0;
            while ((*source)->protocols[protocols_count].name != NULL)
            {
                protocols_count++;
            }

            (*destination)->protocols = (const struct lws_protocols*)malloc(sizeof(struct lws_protocols) * (protocols_count + 1));

            protocols_count = 0;
            while ((*source)->protocols[protocols_count].name != NULL)
            {
                ((struct lws_protocols*)(*destination)->protocols)[protocols_count].name = copy_string((*source)->protocols[protocols_count].name);
                ((struct lws_protocols*)(*destination)->protocols)[protocols_count].callback = (*source)->protocols[protocols_count].callback;
                ((struct lws_protocols*)(*destination)->protocols)[protocols_count].per_session_data_size = (*source)->protocols[protocols_count].per_session_data_size;
                ((struct lws_protocols*)(*destination)->protocols)[protocols_count].rx_buffer_size = (*source)->protocols[protocols_count].rx_buffer_size;
                ((struct lws_protocols*)(*destination)->protocols)[protocols_count].id = (*source)->protocols[protocols_count].id;
                ((struct lws_protocols*)(*destination)->protocols)[protocols_count].user = (*source)->protocols[protocols_count].user;
                protocols_count++;
            }

            ((struct lws_protocols*)(*destination)->protocols)[protocols_count].name = NULL;
        }

        if ((*source)->extensions == NULL)
        {
            (*destination)->extensions = NULL;
        }
        else
        {
            size_t extensions_count = 0;
            while ((*source)->extensions[extensions_count].name != NULL)
            {
                extensions_count++;
            }

            (*destination)->extensions = (const struct lws_extension*)malloc(sizeof(struct lws_extension) * (extensions_count + 1));

            extensions_count = 0;
            while ((*source)->extensions[extensions_count].name != NULL)
            {
                ((struct lws_extension*)(*destination)->extensions)[extensions_count].name = copy_string((*source)->extensions[extensions_count].name);
                ((struct lws_extension*)(*destination)->extensions)[extensions_count].callback = (*source)->extensions[extensions_count].callback;
                ((struct lws_extension*)(*destination)->extensions)[extensions_count].per_session_data_size = (*source)->extensions[extensions_count].per_session_data_size;
                ((struct lws_extension*)(*destination)->extensions)[extensions_count].per_context_private_data = (*source)->extensions[extensions_count].per_context_private_data;
                extensions_count++;
            }

            ((struct lws_extension*)(*destination)->extensions)[extensions_count].name = NULL;
        }

        (*destination)->iface = copy_string((*source)->iface);
        (*destination)->token_limits = (*source)->token_limits;
        (*destination)->port = (*source)->port;
        (*destination)->ssl_private_key_password = copy_string((*source)->ssl_private_key_password);
        (*destination)->ssl_cert_filepath = copy_string((*source)->ssl_cert_filepath);
        (*destination)->ssl_private_key_filepath = copy_string((*source)->ssl_private_key_filepath);
        (*destination)->ssl_ca_filepath = copy_string((*source)->ssl_ca_filepath);
        (*destination)->ssl_cipher_list = copy_string((*source)->ssl_cipher_list);
        (*destination)->http_proxy_address = copy_string((*source)->http_proxy_address);
        (*destination)->http_proxy_port = (*source)->http_proxy_port;
        (*destination)->gid = (*source)->gid;
        (*destination)->uid = (*source)->uid;
        (*destination)->options = (*source)->options;
        (*destination)->user = (*source)->user;
        (*destination)->ka_time = (*source)->ka_time;
        (*destination)->ka_probes = (*source)->ka_probes;
        (*destination)->ka_interval = (*source)->ka_interval;
        (*destination)->provided_client_ssl_ctx = (*source)->provided_client_ssl_ctx;

        result = 0;
    }

    return result;
}

static void umocktypes_free_const_struct_lws_context_creation_info_ptr(struct lws_context_creation_info** value)
{
    /* free protocols, extensions, token_limits */
    size_t protocols_count = 0;
    size_t extensions_count = 0;

    if ((*value)->protocols != NULL)
    {
        while ((*value)->protocols[protocols_count].name != NULL)
        {
            free((void*)(*value)->protocols[protocols_count].name);
            protocols_count++;
        }
        free((void*)(*value)->protocols);
    }

    if ((*value)->extensions != NULL)
    {
        while ((*value)->extensions[extensions_count].name != NULL)
        {
            free((void*)(*value)->extensions[extensions_count].name);
            extensions_count++;
        }
        free((void*)(*value)->extensions);
    }

    free((void*)(*value)->ssl_private_key_password);
    free((void*)(*value)->ssl_cert_filepath);
    free((void*)(*value)->ssl_private_key_filepath);
    free((void*)(*value)->ssl_ca_filepath);
    free((void*)(*value)->ssl_cipher_list);
    free((void*)(*value)->http_proxy_address);
    free(*value);
}

static LIST_ITEM_HANDLE add_to_list(const void* item)
{
    const void** items = (const void**)realloc((void*)list_items, (list_item_count + 1) * sizeof(item));
    if (items != NULL)
    {
        list_items = items;
        list_items[list_item_count++] = item;
    }
    return (LIST_ITEM_HANDLE)list_item_count;
}

static int singlylinkedlist_remove_result;

static int my_singlylinkedlist_remove(SINGLYLINKEDLIST_HANDLE list, LIST_ITEM_HANDLE item)
{
    size_t index = (size_t)item - 1;
    (void)list;
    (void)memmove((void*)&list_items[index], &list_items[index + 1], sizeof(const void*) * (list_item_count - index - 1));
    list_item_count--;
    if (list_item_count == 0)
    {
        free((void*)list_items);
        list_items = NULL;
    }
    return singlylinkedlist_remove_result;
}

static LIST_ITEM_HANDLE my_singlylinkedlist_get_head_item(SINGLYLINKEDLIST_HANDLE list)
{
    LIST_ITEM_HANDLE list_item_handle = NULL;
    (void)list;
    if (list_item_count > 0)
    {
        list_item_handle = (LIST_ITEM_HANDLE)1;
    }
    else
    {
        list_item_handle = NULL;
    }
    return list_item_handle;
}

LIST_ITEM_HANDLE my_singlylinkedlist_add(SINGLYLINKEDLIST_HANDLE list, const void* item)
{
    (void)list;
    return add_to_list(item);
}

const void* my_singlylinkedlist_item_get_value(LIST_ITEM_HANDLE item_handle)
{
    return (const void*)list_items[(size_t)item_handle - 1];
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

OPTIONHANDLER_HANDLE my_OptionHandler_Create(pfCloneOption clone, pfDestroyOption destroy, pfSetOption setoption)
{
    (void)clone, destroy, setoption;
    return (OPTIONHANDLER_HANDLE)malloc(1);
}

void my_OptionHandler_Destroy(OPTIONHANDLER_HANDLE handle)
{
    free(handle);
}

int my_mallocAndStrcpy_s(char** destination, const char* source)
{
    *destination = (char*)malloc(strlen(source) + 1);
    (void)strcpy(*destination, source);
    return 0;
}

#include "azure_c_shared_utility/gballoc.h"

#undef ENABLE_MOCKS

#include "azure_c_shared_utility/wsio.h"

static WSIO_CONFIG default_wsio_config =
{
    "test_host",
    443,
    "test_ws_protocol",
    "a/b/c",
    false
};

// libwebsockets mocks
MOCK_FUNCTION_WITH_CODE(, struct lws_context*, lws_create_context, struct lws_context_creation_info*, info)
    saved_ws_callback = info->protocols[0].callback;
    saved_ws_callback_context = info->user;
MOCK_FUNCTION_END(TEST_LIBWEBSOCKET_CONTEXT)
MOCK_FUNCTION_WITH_CODE(, void, lws_context_destroy, struct lws_context*, context)
MOCK_FUNCTION_END()
MOCK_FUNCTION_WITH_CODE(, int, lws_service, struct lws_context*, context, int, timeout_ms)
MOCK_FUNCTION_END(0)
MOCK_FUNCTION_WITH_CODE(, struct lws_context*, lws_get_context, const struct lws*, wsi)
MOCK_FUNCTION_END(TEST_LIBWEBSOCKET_CONTEXT)
MOCK_FUNCTION_WITH_CODE(, void*, lws_context_user, struct lws_context*, context)
MOCK_FUNCTION_END(TEST_USER_CONTEXT)
MOCK_FUNCTION_WITH_CODE(, int, lws_write, struct lws*, wsi, unsigned char*, buf, size_t, len, enum lws_write_protocol, protocol)
MOCK_FUNCTION_END(0)
MOCK_FUNCTION_WITH_CODE(, int, lws_callback_on_writable, struct lws*, wsi)
MOCK_FUNCTION_END(0)
MOCK_FUNCTION_WITH_CODE(, struct lws*, lws_client_connect, struct lws_context*, clients, const char*, address, int, port, int, ssl_connection, const char*, path, const char*, host, const char*, origin, const char*, protocol, int, ietf_version_or_minus_one)
MOCK_FUNCTION_END(TEST_LIBWEBSOCKET)
MOCK_FUNCTION_WITH_CODE(, struct lws_extension*, lws_get_internal_extensions)
MOCK_FUNCTION_END(TEST_INTERNAL_EXTENSIONS)

// openssl mocks
MOCK_FUNCTION_WITH_CODE(, BIO*, BIO_new, BIO_METHOD*, type)
MOCK_FUNCTION_END(TEST_BIO)
MOCK_FUNCTION_WITH_CODE(, int, BIO_free, BIO*, a)
MOCK_FUNCTION_END(0)
MOCK_FUNCTION_WITH_CODE(, int, BIO_puts, BIO*, bp, const char*, buf)
MOCK_FUNCTION_END(0)
MOCK_FUNCTION_WITH_CODE(, BIO_METHOD*, BIO_s_mem);
MOCK_FUNCTION_END(TEST_BIO_METHOD)
MOCK_FUNCTION_WITH_CODE(, X509_STORE*, SSL_CTX_get_cert_store, const SSL_CTX*, ctx);
MOCK_FUNCTION_END(TEST_CERT_STORE)
MOCK_FUNCTION_WITH_CODE(, int, X509_STORE_add_cert, X509_STORE*, ctx, X509*, x);
MOCK_FUNCTION_END(1)
MOCK_FUNCTION_WITH_CODE(, X509*, PEM_read_bio_X509, BIO*, bp, X509**, x, pem_password_cb*, cb, void*, u);
MOCK_FUNCTION_END(TEST_X509_CERT_1)
MOCK_FUNCTION_WITH_CODE(, void, X509_free, X509*, a);
MOCK_FUNCTION_END()

// consumer mocks
MOCK_FUNCTION_WITH_CODE(, void, test_on_io_open_complete, void*, context, IO_OPEN_RESULT, io_open_result);
MOCK_FUNCTION_END()
MOCK_FUNCTION_WITH_CODE(, void, test_on_bytes_received, void*, context, const unsigned char*, buffer, size_t, size);
MOCK_FUNCTION_END()
MOCK_FUNCTION_WITH_CODE(, void, test_on_io_error, void*, context);
MOCK_FUNCTION_END()
MOCK_FUNCTION_WITH_CODE(, void, test_on_io_close_complete, void*, context);
MOCK_FUNCTION_END()
MOCK_FUNCTION_WITH_CODE(, void, test_on_send_complete, void*, context, IO_SEND_RESULT, send_result)
MOCK_FUNCTION_END()

static TEST_MUTEX_HANDLE g_testByTest;
static TEST_MUTEX_HANDLE g_dllByDll;

DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    char temp_str[256];
    (void)snprintf(temp_str, sizeof(temp_str), "umock_c reported error :%s", ENUM_TO_STRING(UMOCK_C_ERROR_CODE, error_code));
    ASSERT_FAIL(temp_str);
}

BEGIN_TEST_SUITE(wsio_ut)

TEST_SUITE_INITIALIZE(suite_init)
{
    int result;
    TEST_INITIALIZE_MEMORY_DEBUG(g_dllByDll);
    g_testByTest = TEST_MUTEX_CREATE();
    ASSERT_IS_NOT_NULL(g_testByTest);

    umock_c_init(on_umock_c_error);

    result = umocktypes_charptr_register_types();
    ASSERT_ARE_EQUAL(int, 0, result);

    REGISTER_GLOBAL_MOCK_HOOK(gballoc_malloc, my_gballoc_malloc);
    REGISTER_GLOBAL_MOCK_HOOK(gballoc_free, my_gballoc_free);
    REGISTER_GLOBAL_MOCK_RETURN(singlylinkedlist_create, TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE);
    REGISTER_GLOBAL_MOCK_HOOK(singlylinkedlist_remove, my_singlylinkedlist_remove);
    REGISTER_GLOBAL_MOCK_HOOK(singlylinkedlist_get_head_item, my_singlylinkedlist_get_head_item);
    REGISTER_GLOBAL_MOCK_HOOK(singlylinkedlist_add, my_singlylinkedlist_add);
    REGISTER_GLOBAL_MOCK_HOOK(singlylinkedlist_item_get_value, my_singlylinkedlist_item_get_value);
    REGISTER_GLOBAL_MOCK_HOOK(singlylinkedlist_find, my_singlylinkedlist_find);
    REGISTER_GLOBAL_MOCK_HOOK(mallocAndStrcpy_s, my_mallocAndStrcpy_s);
    REGISTER_GLOBAL_MOCK_HOOK(OptionHandler_Create, my_OptionHandler_Create);
    REGISTER_GLOBAL_MOCK_HOOK(OptionHandler_Destroy, my_OptionHandler_Destroy);
    REGISTER_GLOBAL_MOCK_RETURN(OptionHandler_AddOption, OPTIONHANDLER_OK);
    REGISTER_TYPE(IO_OPEN_RESULT, IO_OPEN_RESULT);
    REGISTER_TYPE(IO_SEND_RESULT, IO_SEND_RESULT);
    REGISTER_TYPE(OPTIONHANDLER_RESULT, OPTIONHANDLER_RESULT);
    REGISTER_TYPE(my_lws_write_protocol_enum, my_lws_write_protocol_enum);
    REGISTER_TYPE(const struct lws_context_creation_info*, const_struct_lws_context_creation_info_ptr);

    REGISTER_UMOCK_ALIAS_TYPE(struct lws_context_creation_info*, const struct lws_context_creation_info*);
    REGISTER_UMOCK_ALIAS_TYPE(SINGLYLINKEDLIST_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(pfCloneOption, void*);
    REGISTER_UMOCK_ALIAS_TYPE(pfDestroyOption, void*);
    REGISTER_UMOCK_ALIAS_TYPE(pfSetOption, void*);
    REGISTER_UMOCK_ALIAS_TYPE(OPTIONHANDLER_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(LIST_ITEM_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(enum lws_write_protocol, my_lws_write_protocol_enum);
    REGISTER_UMOCK_ALIAS_TYPE(lws_write_protocol, my_lws_write_protocol_enum);
}

TEST_SUITE_CLEANUP(suite_cleanup)
{
    umock_c_deinit();

    TEST_MUTEX_DESTROY(g_testByTest);
    TEST_DEINITIALIZE_MEMORY_DEBUG(g_dllByDll);
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
    singlylinkedlist_remove_result = 0;
}

TEST_FUNCTION_CLEANUP(method_cleanup)
{
    TEST_MUTEX_RELEASE(g_testByTest);
}

/* wsio_create */

/* Tests_SRS_WSIO_01_001: [wsio_create shall create an instance of a wsio and return a non-NULL handle to it.] */
/* Tests_SRS_WSIO_01_098: [wsio_create shall create a pending IO list that is to be used when sending buffers over the libwebsockets IO by calling singlylinkedlist_create.] */
/* Tests_SRS_WSIO_01_003: [io_create_parameters shall be used as a WSIO_CONFIG*.] */
/* Tests_SRS_WSIO_01_006: [The members host, protocol_name, relative_path and trusted_ca shall be copied for later use (they are needed when the IO is opened).] */
TEST_FUNCTION(wsio_create_with_valid_args_succeeds)
{
	// arrange
	EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_create());
    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));

	// act
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);

	// assert
	ASSERT_IS_NOT_NULL(wsio);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_002: [If the argument io_create_parameters is NULL then wsio_create shall return NULL.] */
TEST_FUNCTION(wsio_create_with_NULL_io_create_parameters_fails)
{
    // arrange

    // act
    CONCRETE_IO_HANDLE wsio = wsio_create(NULL);

    // assert
    ASSERT_IS_NULL(wsio);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_WSIO_01_004: [If any of the WSIO_CONFIG fields host, protocol_name or relative_path is NULL then wsio_create shall return NULL.] */
TEST_FUNCTION(wsio_create_with_NULL_hostname_fails)
{
    // arrange
    static WSIO_CONFIG test_wsio_config =
    {
        NULL,
        443,
        "test_ws_protocol",
        "a/b/c",
        false
    };

    // act
    CONCRETE_IO_HANDLE wsio = wsio_create(&test_wsio_config);

    // assert
    ASSERT_IS_NULL(wsio);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_WSIO_01_004: [If any of the WSIO_CONFIG fields host, protocol_name or relative_path is NULL then wsio_create shall return NULL.] */
TEST_FUNCTION(wsio_create_with_NULL_protocol_name_fails)
{
    // arrange
    static WSIO_CONFIG test_wsio_config =
    {
        "testhost",
        443,
        NULL,
        "a/b/c",
        false
    };

    // act
    CONCRETE_IO_HANDLE wsio = wsio_create(&test_wsio_config);

    // assert
    ASSERT_IS_NULL(wsio);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_WSIO_01_004: [If any of the WSIO_CONFIG fields host, protocol_name or relative_path is NULL then wsio_create shall return NULL.] */
TEST_FUNCTION(wsio_create_with_NULL_relative_path_fails)
{
    // arrange
    static WSIO_CONFIG test_wsio_config =
    {
        "testhost",
        443,
        "test_ws_protocol",
        NULL,
        false
    };

    // act
    CONCRETE_IO_HANDLE wsio = wsio_create(&test_wsio_config);

    // assert
    ASSERT_IS_NULL(wsio);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_WSIO_01_005: [If allocating memory for the new wsio instance fails then wsio_create shall return NULL.] */
TEST_FUNCTION(when_allocating_memory_for_the_instance_fails_wsio_create_fails)
{
    // arrange
    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG))
        .SetReturn((void*)NULL);

    // act
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);

    // assert
    ASSERT_IS_NULL(wsio);
}

/* Tests_SRS_WSIO_01_099: [If singlylinkedlist_create fails then wsio_create shall fail and return NULL.] */
TEST_FUNCTION(when_creating_the_pending_io_list_fails_wsio_create_fails)
{
    // arrange
    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_create())
        .SetReturn((SINGLYLINKEDLIST_HANDLE)NULL);
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);

    // assert
    ASSERT_IS_NULL(wsio);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_WSIO_01_005: [If allocating memory for the new wsio instance fails then wsio_create shall return NULL.] */
TEST_FUNCTION(when_allocating_memory_for_the_host_name_fails_wsio_create_fails)
{
    // arrange
    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_create());
    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG))
        .SetReturn((SINGLYLINKEDLIST_HANDLE)NULL);
    STRICT_EXPECTED_CALL(singlylinkedlist_destroy(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);

    // assert
    ASSERT_IS_NULL(wsio);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_WSIO_01_005: [If allocating memory for the new wsio instance fails then wsio_create shall return NULL.] */
TEST_FUNCTION(when_allocating_memory_for_the_protocol_name_fails_wsio_create_fails)
{
    // arrange
    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_create());
    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG))
        .SetReturn((SINGLYLINKEDLIST_HANDLE)NULL);
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_destroy(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);

    // assert
    ASSERT_IS_NULL(wsio);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_WSIO_01_005: [If allocating memory for the new wsio instance fails then wsio_create shall return NULL.] */
TEST_FUNCTION(when_allocating_memory_for_the_relative_path_fails_wsio_create_fails)
{
    // arrange
    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_create());
    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG))
        .SetReturn((SINGLYLINKEDLIST_HANDLE)NULL);
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_destroy(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);

    // assert
    ASSERT_IS_NULL(wsio);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_WSIO_01_005: [If allocating memory for the new wsio instance fails then wsio_create shall return NULL.] */
TEST_FUNCTION(when_allocating_memory_for_the_protocols_fails_wsio_create_fails)
{
    // arrange
    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_create());
    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG))
        .SetReturn((SINGLYLINKEDLIST_HANDLE)NULL);
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_destroy(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);

    // assert
    ASSERT_IS_NULL(wsio);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* wsio_destroy */

/* Tests_SRS_WSIO_01_007: [wsio_destroy shall free all resources associated with the wsio instance.] */
TEST_FUNCTION(when_wsio_destroy_is_called_all_resources_are_freed)
{
    // arrange
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG)); // host_name
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG)); // relative_path
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG)); // protocol_name
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG)); // protocols
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG)); // trusted_ca
    STRICT_EXPECTED_CALL(singlylinkedlist_destroy(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG)); // instance

    // act
    wsio_destroy(wsio);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_WSIO_01_008: [If ws_io is NULL, wsio_destroy shall do nothing.] */
TEST_FUNCTION(wsio_destroy_with_NULL_handle_does_nothing)
{
    // arrange

    // act
    wsio_destroy(NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_WSIO_01_009: [wsio_destroy shall execute a close action if the IO has already been open or an open action is already pending.] */
TEST_FUNCTION(wsio_destroy_closes_the_underlying_lws_before_destroying_all_resources)
{
    // arrange
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_on_io_open_complete((void*)0x4242, IO_OPEN_CANCELLED));

    STRICT_EXPECTED_CALL(lws_context_destroy(TEST_LIBWEBSOCKET_CONTEXT));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG)); // host_name
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG)); // relative_path
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG)); // protocol_name
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG)); // protocols
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG)); // trusted_ca
    STRICT_EXPECTED_CALL(singlylinkedlist_destroy(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG)); // instance

    // act
    wsio_destroy(wsio);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* wsio_open */

/* Tests_SRS_WSIO_01_010: [wsio_open shall create a context for the libwebsockets connection by calling lws_create_context.] */
/* Tests_SRS_WSIO_01_011: [The port member of the info argument shall be set to CONTEXT_PORT_NO_LISTEN.] */
/* Tests_SRS_WSIO_01_091: [The extensions field shall be set to the internal extensions obtained by calling lws_get_internal_extensions.] */
/* Tests_SRS_WSIO_01_092: [gid and uid shall be set to -1.] */
/* Tests_SRS_WSIO_01_093: [The members iface, token_limits, ssl_cert_filepath, ssl_private_key_filepath, ssl_private_key_password, ssl_ca_filepath, ssl_cipher_list and provided_client_ssl_ctx shall be set to NULL.] */
/* Tests_SRS_WSIO_01_172: [ If no proxy was configured, http_proxy_address shall be set to NULL. ] */
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
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    struct lws_context_creation_info lws_context_info;
    struct lws_protocols protocols[2];
    umock_c_reset_all_calls();

    protocols[0].name = default_wsio_config.protocol_name;
    protocols[0].callback = NULL;
    protocols[0].per_session_data_size = 0;
    protocols[0].rx_buffer_size = 0;
    protocols[0].id = 0;
    protocols[0].user = NULL;
    protocols[1].name = NULL;
    protocols[1].callback = NULL;
    protocols[1].per_session_data_size = 0;
    protocols[1].rx_buffer_size = 0;
    protocols[1].id = 0;
    protocols[1].user = NULL;

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
    lws_context_info.http_proxy_port = 0;
    lws_context_info.options = 0;
    lws_context_info.ka_time = 0;
    lws_context_info.ka_probes = 0;
    lws_context_info.ka_interval = 0;

    lws_context_info.protocols = protocols;

    STRICT_EXPECTED_CALL(lws_get_internal_extensions());
    STRICT_EXPECTED_CALL(lws_create_context(&lws_context_info));
    STRICT_EXPECTED_CALL(lws_client_connect(TEST_LIBWEBSOCKET_CONTEXT, default_wsio_config.host, default_wsio_config.port, 0, default_wsio_config.relative_path, default_wsio_config.host, default_wsio_config.host, default_wsio_config.protocol_name, -1));

    // act
    int result = wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_169: [ If any proxy was configured by using the proxy data option, then http_proxy_address shall be set to the address, port, username and password specified in the proxy options, in the format {username}:{password}@{address}:{port}. ] */
/* Tests_SRS_WSIO_01_171: [ If any proxy was configured by using the proxy data option, the http_proxy_port shall be set to the proxy port. ]*/
TEST_FUNCTION(wsio_open_with_proxy_option_with_username_and_pwd_succeeds)
{
    // arrange
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    struct lws_context_creation_info lws_context_info;
    struct lws_protocols protocols[2];

    HTTP_PROXY_OPTIONS proxy_options;
    proxy_options.host_address = "test_proxy";
    proxy_options.port = 8080;
    proxy_options.username = "user_name";
    proxy_options.password = "secret";

    (void)wsio_setoption(wsio, OPTION_HTTP_PROXY, &proxy_options);

    umock_c_reset_all_calls();

    protocols[0].name = default_wsio_config.protocol_name;
    protocols[0].callback = NULL;
    protocols[0].per_session_data_size = 0;
    protocols[0].rx_buffer_size = 0;
    protocols[0].id = 0;
    protocols[0].user = NULL;
    protocols[1].name = NULL;
    protocols[1].callback = NULL;
    protocols[1].per_session_data_size = 0;
    protocols[1].rx_buffer_size = 0;
    protocols[1].id = 0;
    protocols[1].user = NULL;

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
    lws_context_info.http_proxy_address = "user_name:secret@test_proxy:8080";
    lws_context_info.http_proxy_port = 8080;
    lws_context_info.options = 0;
    lws_context_info.ka_time = 0;
    lws_context_info.ka_probes = 0;
    lws_context_info.ka_interval = 0;

    lws_context_info.protocols = protocols;

    STRICT_EXPECTED_CALL(lws_get_internal_extensions());
    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(lws_create_context(&lws_context_info));
    STRICT_EXPECTED_CALL(lws_client_connect(TEST_LIBWEBSOCKET_CONTEXT, default_wsio_config.host, default_wsio_config.port, 0, default_wsio_config.relative_path, default_wsio_config.host, default_wsio_config.host, default_wsio_config.protocol_name, -1));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    int result = wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_169: [ If any proxy was configured by using the proxy data option, then http_proxy_address shall be set to the address, port, username and password specified in the proxy options, in the format {username}:{password}@{address}:{port}. ] */
/* Tests_SRS_WSIO_01_171: [ If any proxy was configured by using the proxy data option, the http_proxy_port shall be set to the proxy port. ]*/
TEST_FUNCTION(wsio_open_with_proxy_option_with_username_and_pwd_and_5digit_port_succeeds)
{
    // arrange
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    struct lws_context_creation_info lws_context_info;
    struct lws_protocols protocols[2];

    HTTP_PROXY_OPTIONS proxy_options;
    proxy_options.host_address = "test_proxy";
    proxy_options.port = 22222;
    proxy_options.username = "user_name";
    proxy_options.password = "secret";

    (void)wsio_setoption(wsio, OPTION_HTTP_PROXY, &proxy_options);

    umock_c_reset_all_calls();

    protocols[0].name = default_wsio_config.protocol_name;
    protocols[0].callback = NULL;
    protocols[0].per_session_data_size = 0;
    protocols[0].rx_buffer_size = 0;
    protocols[0].id = 0;
    protocols[0].user = NULL;
    protocols[1].name = NULL;
    protocols[1].callback = NULL;
    protocols[1].per_session_data_size = 0;
    protocols[1].rx_buffer_size = 0;
    protocols[1].id = 0;
    protocols[1].user = NULL;

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
    lws_context_info.http_proxy_address = "user_name:secret@test_proxy:22222";
    lws_context_info.http_proxy_port = 22222;
    lws_context_info.options = 0;
    lws_context_info.ka_time = 0;
    lws_context_info.ka_probes = 0;
    lws_context_info.ka_interval = 0;

    lws_context_info.protocols = protocols;

    STRICT_EXPECTED_CALL(lws_get_internal_extensions());
    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(lws_create_context(&lws_context_info));
    STRICT_EXPECTED_CALL(lws_client_connect(TEST_LIBWEBSOCKET_CONTEXT, default_wsio_config.host, default_wsio_config.port, 0, default_wsio_config.relative_path, default_wsio_config.host, default_wsio_config.host, default_wsio_config.protocol_name, -1));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    int result = wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_169: [ If any proxy was configured by using the proxy data option, then http_proxy_address shall be set to the address, port, username and password specified in the proxy options, in the format {username}:{password}@{address}:{port}. ] */
/* Tests_SRS_WSIO_01_170: [ If no username/password was specified for the proxy settings then http_proxy_address shall be set to the address and port specified in the proxy options, in the format {address}:{port}. ]*/
TEST_FUNCTION(wsio_open_with_proxy_option_without_username_and_pwd_succeeds)
{
    // arrange
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    struct lws_context_creation_info lws_context_info;
    struct lws_protocols protocols[2];

    HTTP_PROXY_OPTIONS proxy_options;
    proxy_options.host_address = "test_proxy";
    proxy_options.port = 8080;
    proxy_options.username = "user_name";
    proxy_options.password = "secret";

    (void)wsio_setoption(wsio, OPTION_HTTP_PROXY, &proxy_options);

    umock_c_reset_all_calls();

    protocols[0].name = default_wsio_config.protocol_name;
    protocols[0].callback = NULL;
    protocols[0].per_session_data_size = 0;
    protocols[0].rx_buffer_size = 0;
    protocols[0].id = 0;
    protocols[0].user = NULL;
    protocols[1].name = NULL;
    protocols[1].callback = NULL;
    protocols[1].per_session_data_size = 0;
    protocols[1].rx_buffer_size = 0;
    protocols[1].id = 0;
    protocols[1].user = NULL;

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
    lws_context_info.http_proxy_address = "user_name:secret@test_proxy:8080";
    lws_context_info.http_proxy_port = 8080;
    lws_context_info.options = 0;
    lws_context_info.ka_time = 0;
    lws_context_info.ka_probes = 0;
    lws_context_info.ka_interval = 0;

    lws_context_info.protocols = protocols;

    STRICT_EXPECTED_CALL(lws_get_internal_extensions());
    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(lws_create_context(&lws_context_info));
    STRICT_EXPECTED_CALL(lws_client_connect(TEST_LIBWEBSOCKET_CONTEXT, default_wsio_config.host, default_wsio_config.port, 0, default_wsio_config.relative_path, default_wsio_config.host, default_wsio_config.host, default_wsio_config.protocol_name, -1));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    int result = wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

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
    static WSIO_CONFIG wsio_config =
    {
        "hagauaga",
        1234,
        "another_proto",
        "d1/e2/f3",
        true
    };
    CONCRETE_IO_HANDLE wsio = wsio_create(&wsio_config);
    struct lws_context_creation_info lws_context_info;
    struct lws_protocols protocols[2];
    umock_c_reset_all_calls();

    protocols[0].name = "another_proto";
    protocols[0].callback = NULL;
    protocols[0].per_session_data_size = 0;
    protocols[0].rx_buffer_size = 0;
    protocols[0].id = 0;
    protocols[0].user = NULL;
    protocols[1].name = NULL;
    protocols[1].callback = NULL;
    protocols[1].per_session_data_size = 0;
    protocols[1].rx_buffer_size = 0;
    protocols[1].id = 0;
    protocols[1].user = NULL;

    lws_context_info.port = CONTEXT_PORT_NO_LISTEN;
    lws_context_info.extensions = (struct lws_extension*)NULL;
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

    STRICT_EXPECTED_CALL(lws_get_internal_extensions())
        .SetReturn((struct lws_extension*)NULL);
    STRICT_EXPECTED_CALL(lws_create_context(&lws_context_info));
    STRICT_EXPECTED_CALL(lws_client_connect(TEST_LIBWEBSOCKET_CONTEXT, wsio_config.host, wsio_config.port, 1, wsio_config.relative_path, wsio_config.host, wsio_config.host, wsio_config.protocol_name, -1));

    // act
    int result = wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_169: [ If any proxy was configured by using the proxy data option, then http_proxy_address shall be set to the address, port, username and password specified in the proxy options, in the format {username}:{password}@{address}:{port}. ] */
/* Tests_SRS_WSIO_01_170: [ If no username/password was specified for the proxy settings then http_proxy_address shall be set to the address and port specified in the proxy options, in the format {address}:{port}. ]*/
TEST_FUNCTION(wsio_open_with_proxy_config_with_username_NULL_and_non_NULL_password_succeeds)
{
    // arrange
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    struct lws_context_creation_info lws_context_info;
    struct lws_protocols protocols[2];

    HTTP_PROXY_OPTIONS proxy_options;
    proxy_options.host_address = "ha";
    proxy_options.port = 8080;
    proxy_options.username = NULL;
    proxy_options.password = "pwd";
    (void)wsio_setoption(wsio, OPTION_HTTP_PROXY, &proxy_options);

    umock_c_reset_all_calls();

    protocols[0].name = default_wsio_config.protocol_name;
    protocols[0].callback = NULL;
    protocols[0].per_session_data_size = 0;
    protocols[0].rx_buffer_size = 0;
    protocols[0].id = 0;
    protocols[0].user = NULL;
    protocols[1].name = NULL;
    protocols[1].callback = NULL;
    protocols[1].per_session_data_size = 0;
    protocols[1].rx_buffer_size = 0;
    protocols[1].id = 0;
    protocols[1].user = NULL;

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
    lws_context_info.options = 0;
    lws_context_info.ka_time = 0;
    lws_context_info.http_proxy_address = "ha:8080";
    lws_context_info.http_proxy_port = 8080;
    lws_context_info.protocols = protocols;

    STRICT_EXPECTED_CALL(lws_get_internal_extensions());
    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(lws_create_context(&lws_context_info));
    STRICT_EXPECTED_CALL(lws_client_connect(TEST_LIBWEBSOCKET_CONTEXT, default_wsio_config.host, default_wsio_config.port, 0, default_wsio_config.relative_path, default_wsio_config.host, default_wsio_config.host, default_wsio_config.protocol_name, -1));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    int result = wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_022: [If creating the context fails then wsio_open shall fail and return a non-zero value.] */
TEST_FUNCTION(when_creating_the_libwebsockets_context_fails_then_wsio_open_fails)
{
    // arrange
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    struct lws_context_creation_info lws_context_info;
    struct lws_protocols protocols[2];
    umock_c_reset_all_calls();

    protocols[0].name = default_wsio_config.protocol_name;
    protocols[0].callback = NULL;
    protocols[0].per_session_data_size = 0;
    protocols[0].rx_buffer_size = 0;
    protocols[0].id = 0;
    protocols[0].user = NULL;
    protocols[1].name = NULL;
    protocols[1].callback = NULL;
    protocols[1].per_session_data_size = 0;
    protocols[1].rx_buffer_size = 0;
    protocols[1].id = 0;
    protocols[1].user = NULL;

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

    STRICT_EXPECTED_CALL(lws_get_internal_extensions());
    STRICT_EXPECTED_CALL(lws_create_context(&lws_context_info))
        .SetReturn((struct lws_context*)NULL);

    // act
    int result = wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    ///cleanup 
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_033: [If lws_client_connect fails then wsio_open shall fail and return a non-zero value.] */
TEST_FUNCTION(when_lws_client_connect_fails_then_wsio_open_fails)
{
    // arrange
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    struct lws_context_creation_info lws_context_info;
    struct lws_protocols protocols[2];
    umock_c_reset_all_calls();

    protocols[0].name = default_wsio_config.protocol_name;
    protocols[0].callback = NULL;
    protocols[0].per_session_data_size = 0;
    protocols[0].rx_buffer_size = 0;
    protocols[0].id = 0;
    protocols[0].user = NULL;
    protocols[1].name = NULL;
    protocols[1].callback = NULL;
    protocols[1].per_session_data_size = 0;
    protocols[1].rx_buffer_size = 0;
    protocols[1].id = 0;
    protocols[1].user = NULL;

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

    STRICT_EXPECTED_CALL(lws_get_internal_extensions());
    STRICT_EXPECTED_CALL(lws_create_context(&lws_context_info));
    STRICT_EXPECTED_CALL(lws_client_connect(TEST_LIBWEBSOCKET_CONTEXT, default_wsio_config.host, default_wsio_config.port, 0, default_wsio_config.relative_path, default_wsio_config.host, default_wsio_config.host, default_wsio_config.protocol_name, -1))
        .SetReturn((struct lws*)NULL);
    STRICT_EXPECTED_CALL(lws_context_destroy(TEST_LIBWEBSOCKET_CONTEXT));

    // act
    int result = wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);

    ///cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_034: [If another open is in progress or has completed successfully (the IO is open), wsio_open shall fail and return a non-zero value without performing any connection related activities.] */
TEST_FUNCTION(a_second_wsio_open_while_opening_fails)
{
    // arrange
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    umock_c_reset_all_calls();

    // act
    int result = wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_036: [The callback on_io_open_complete shall be called with io_open_result being set to IO_OPEN_OK when the open action is succesfull.] */
/* Tests_SRS_WSIO_01_039: [The callback_context argument shall be passed to on_io_open_complete as is.] */
TEST_FUNCTION(when_ws_callback_indicates_a_connect_complete_then_the_on_open_complete_callback_is_triggered)
{
    // arrange
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    STRICT_EXPECTED_CALL(test_on_io_open_complete((void*)0x4242, IO_OPEN_OK));

    // act
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_ESTABLISHED, NULL, NULL, 0);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_037: [If any error occurs while the open action is in progress, the callback on_io_open_complete shall be called with io_open_result being set to IO_OPEN_ERROR.] */
/* Tests_SRS_WSIO_01_039: [The callback_context argument shall be passed to on_io_open_complete as is.] */
TEST_FUNCTION(when_ws_callback_indicates_a_connection_error_then_the_on_open_complete_callback_is_triggered_with_error)
{
    // arrange
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    STRICT_EXPECTED_CALL(test_on_io_open_complete((void*)0x4242, IO_OPEN_ERROR));
    STRICT_EXPECTED_CALL(lws_context_destroy(TEST_LIBWEBSOCKET_CONTEXT));

    // act
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_CONNECTION_ERROR, NULL, NULL, 0);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_038: [If wsio_close is called while the open action is in progress, the callback on_io_open_complete shall be called with io_open_result being set to IO_OPEN_CANCELLED and then the wsio_close shall proceed to close the IO.] */
/* Tests_SRS_WSIO_01_039: [The callback_context argument shall be passed to on_io_open_complete as is.] */
TEST_FUNCTION(when_wsio_close_is_called_while_an_open_action_is_in_progress_the_on_io_open_complete_callback_is_triggered_with_cancelled)
{
    // arrange
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_on_io_open_complete((void*)0x4242, IO_OPEN_CANCELLED));
    STRICT_EXPECTED_CALL(lws_context_destroy(TEST_LIBWEBSOCKET_CONTEXT));

    // act
    wsio_close(wsio, NULL, NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_039: [The callback_context argument shall be passed to on_io_open_complete as is.] */
TEST_FUNCTION(when_ws_callback_indicates_a_connection_error_then_the_on_open_complete_callback_is_triggered_with_error_and_NULL_callback_context)
{
    // arrange
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, NULL, test_on_bytes_received, NULL, test_on_io_error, NULL);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    STRICT_EXPECTED_CALL(test_on_io_open_complete(NULL, IO_OPEN_ERROR));
    STRICT_EXPECTED_CALL(lws_context_destroy(TEST_LIBWEBSOCKET_CONTEXT));

    // act
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_CONNECTION_ERROR, NULL, NULL, 0);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_039: [The callback_context argument shall be passed to on_io_open_complete as is.] */
TEST_FUNCTION(when_ws_callback_indicates_a_connect_complete_then_the_on_open_complete_callback_is_triggered_with_NULL_callback_context)
{
    // arrange
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, NULL, test_on_bytes_received, NULL, test_on_io_error, NULL);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    STRICT_EXPECTED_CALL(test_on_io_open_complete(NULL, IO_OPEN_OK));

    // act
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_ESTABLISHED, NULL, NULL, 0);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_040: [The argument on_io_open_complete shall be optional, if NULL is passed by the caller then no open complete callback shall be triggered.] */
TEST_FUNCTION(when_ws_callback_indicates_a_connect_complete_and_no_on_open_complete_callback_was_supplied_no_callback_is_triggered)
{
    // arrange
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, NULL, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);

    // act
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_ESTABLISHED, NULL, NULL, 0);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_040: [The argument on_io_open_complete shall be optional, if NULL is passed by the caller then no open complete callback shall be triggered.] */
TEST_FUNCTION(when_ws_callback_indicates_a_connect_error_and_no_on_open_complete_callback_was_supplied_no_callback_is_triggered)
{
    // arrange
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, NULL, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    STRICT_EXPECTED_CALL(lws_context_destroy(TEST_LIBWEBSOCKET_CONTEXT));

    // act
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_CONNECTION_ERROR, NULL, NULL, 0);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

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
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_on_io_open_complete((void*)0x4242, IO_OPEN_CANCELLED));
    STRICT_EXPECTED_CALL(lws_context_destroy(TEST_LIBWEBSOCKET_CONTEXT));

    // act
    int result = wsio_close(wsio, NULL, NULL);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_047: [The callback on_io_close_complete shall be called after the close action has been completed in the context of wsio_close (wsio_close is effectively blocking).] */
/* Tests_SRS_WSIO_01_048: [The callback_context argument shall be passed to on_io_close_complete as is.] */
TEST_FUNCTION(wsio_close_destroys_the_ws_context_and_calls_the_io_close_complete_callback)
{
    // arrange
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_on_io_open_complete((void*)0x4242, IO_OPEN_CANCELLED));
    STRICT_EXPECTED_CALL(lws_context_destroy(TEST_LIBWEBSOCKET_CONTEXT));
    STRICT_EXPECTED_CALL(test_on_io_close_complete((void*)0x4243));

    // act
    int result = wsio_close(wsio, test_on_io_close_complete, (void*)0x4243);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_047: [The callback on_io_close_complete shall be called after the close action has been completed in the context of wsio_close (wsio_close is effectively blocking).] */
/* Tests_SRS_WSIO_01_048: [The callback_context argument shall be passed to on_io_close_complete as is.] */
/* Tests_SRS_WSIO_01_108: [wsio_close shall obtain all the pending IO items by repetitively querying for the head of the pending IO list and freeing that head item.] */
/* Tests_SRS_WSIO_01_111: [Obtaining the head of the pending IO list shall be done by calling singlylinkedlist_get_head_item.] */
TEST_FUNCTION(wsio_close_after_ws_connected_calls_the_io_close_complete_callback)
{
    // arrange
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    STRICT_EXPECTED_CALL(lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_ESTABLISHED, saved_ws_callback_context, NULL, 0);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(singlylinkedlist_get_head_item(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE));
    STRICT_EXPECTED_CALL(lws_context_destroy(TEST_LIBWEBSOCKET_CONTEXT));
    STRICT_EXPECTED_CALL(test_on_io_close_complete((void*)0x4243));

    // act
    int result = wsio_close(wsio, test_on_io_close_complete, (void*)0x4243);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_042: [if ws_io is NULL, wsio_close shall return a non-zero value.] */
TEST_FUNCTION(wsio_close_with_NULL_handle_fails)
{
    // arrange

    // act
    int result = wsio_close(NULL, test_on_io_close_complete, (void*)0x4242);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
}

/* Tests_SRS_WSIO_01_045: [wsio_close when no open action has been issued shall fail and return a non-zero value.] */
TEST_FUNCTION(wsio_close_when_not_open_fails)
{
    // arrange
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    umock_c_reset_all_calls();

    // act
    int result = wsio_close(wsio, test_on_io_close_complete, (void*)0x4242);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_046: [wsio_close after a wsio_close shall fail and return a non-zero value.]  */
TEST_FUNCTION(wsio_close_when_already_closed_fails)
{
    // arrange
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    (void)wsio_close(wsio, test_on_io_close_complete, (void*)0x4242);
    umock_c_reset_all_calls();

    // act
    int result = wsio_close(wsio, test_on_io_close_complete, (void*)0x4242);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);

    // cleanup
    wsio_destroy(wsio);
}

/* wsio_send */

/* Tests_SRS_WSIO_01_050: [wsio_send shall send the buffer bytes through the websockets connection.] */
/* Tests_SRS_WSIO_01_054: [wsio_send shall queue the buffer and size until the libwebsockets callback is invoked with the event LWS_CALLBACK_CLIENT_WRITEABLE.] */
/* Tests_SRS_WSIO_01_105: [The data and callback shall be queued by calling singlylinkedlist_add on the list created in wsio_create.] */
/* Tests_SRS_WSIO_01_056: [After queueing the data, wsio_send shall call lws_callback_on_writable, while passing as arguments the websockets instance previously obtained in wsio_open from lws_client_connect.] */
/* Tests_SRS_WSIO_01_107: [On success, wsio_send shall return 0.] */
TEST_FUNCTION(wsio_send_adds_the_bytes_to_the_list_and_triggers_on_writable_when_the_io_is_open)
{
    // arrange
    unsigned char test_buffer[] = { 0x42 };
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    STRICT_EXPECTED_CALL(lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_ESTABLISHED, saved_ws_callback_context, NULL, 0);
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_add(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE, IGNORED_PTR_ARG))
        .IgnoreArgument(2);
    STRICT_EXPECTED_CALL(lws_callback_on_writable(TEST_LIBWEBSOCKET));

    // act
    int result = wsio_send(wsio, test_buffer, sizeof(test_buffer), test_on_send_complete, (void*)0x4243);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_055: [If queueing the data fails (i.e. due to insufficient memory), wsio_send shall fail and return a non-zero value.] */
TEST_FUNCTION(when_allocating_memory_for_the_list_item_fails_wsio_send_fails)
{
    // arrange
    unsigned char test_buffer[] = { 0x42 };
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    STRICT_EXPECTED_CALL(lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_ESTABLISHED, saved_ws_callback_context, NULL, 0);
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG))
        .SetReturn((void*)NULL);

    // act
    int result = wsio_send(wsio, test_buffer, sizeof(test_buffer), test_on_send_complete, (void*)0x4243);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_055: [If queueing the data fails (i.e. due to insufficient memory), wsio_send shall fail and return a non-zero value.] */
TEST_FUNCTION(when_allocating_memory_for_the_pending_bytes_fails_wsio_send_fails)
{
    // arrange
    unsigned char test_buffer[] = { 0x42 };
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    STRICT_EXPECTED_CALL(lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_ESTABLISHED, saved_ws_callback_context, NULL, 0);
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG))
        .SetReturn((void*)NULL);
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    int result = wsio_send(wsio, test_buffer, sizeof(test_buffer), test_on_send_complete, (void*)0x4243);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_055: [If queueing the data fails (i.e. due to insufficient memory), wsio_send shall fail and return a non-zero value.] */
TEST_FUNCTION(when_singlylinkedlist_add_fails_wsio_send_fails)
{
    // arrange
    unsigned char test_buffer[] = { 0x42 };
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    STRICT_EXPECTED_CALL(lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_ESTABLISHED, saved_ws_callback_context, NULL, 0);
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_add(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE, IGNORED_PTR_ARG))
        .IgnoreArgument(2)
        .SetReturn((LIST_ITEM_HANDLE)NULL);
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    int result = wsio_send(wsio, test_buffer, sizeof(test_buffer), test_on_send_complete, (void*)0x4243);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_106: [If lws_callback_on_writable returns a negative value, wsio_send shall fail and return a non-zero value.] */
TEST_FUNCTION(when_callback_on_writable_fails_then_wsio_send_fails)
{
    // arrange
    unsigned char test_buffer[] = { 0x42 };
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    STRICT_EXPECTED_CALL(lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_ESTABLISHED, saved_ws_callback_context, NULL, 0);
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_add(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE, IGNORED_PTR_ARG))
        .IgnoreArgument(2);
    STRICT_EXPECTED_CALL(lws_callback_on_writable(TEST_LIBWEBSOCKET))
        .SetReturn(-1);

    // act
    int result = wsio_send(wsio, test_buffer, sizeof(test_buffer), test_on_send_complete, (void*)0x4243);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_050: [wsio_send shall send the buffer bytes through the websockets connection.] */
/* Tests_SRS_WSIO_01_054: [wsio_send shall queue the buffer and size until the libwebsockets callback is invoked with the event LWS_CALLBACK_CLIENT_WRITEABLE.] */
/* Tests_SRS_WSIO_01_057: [The callback on_send_complete shall be called with SEND_RESULT_OK when the send is indicated as complete.] */
TEST_FUNCTION(when_lws_wants_to_send_bytes_they_are_pushed_to_lws)
{
    // arrange
    unsigned char test_buffer[] = { 0x42 };
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    STRICT_EXPECTED_CALL(lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_ESTABLISHED, saved_ws_callback_context, NULL, 0);
    (void)wsio_send(wsio, test_buffer, sizeof(test_buffer), test_on_send_complete, (void*)0x4243);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    STRICT_EXPECTED_CALL(singlylinkedlist_get_head_item(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE));
    EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(lws_write(TEST_LIBWEBSOCKET, IGNORED_PTR_ARG, sizeof(test_buffer), LWS_WRITE_BINARY))
        .ValidateArgumentBuffer(2, test_buffer, sizeof(test_buffer))
        .SetReturn((int)sizeof(test_buffer));
    STRICT_EXPECTED_CALL(test_on_send_complete((void*)0x4243, IO_SEND_OK));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(singlylinkedlist_remove(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_get_head_item(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_WRITEABLE, saved_ws_callback_context, NULL, 0);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_060: [The argument on_send_complete shall be optional, if NULL is passed by the caller then no send complete callback shall be triggered.] */
TEST_FUNCTION(when_lws_wants_to_send_bytes_they_are_pushed_to_lws_but_no_callback_is_called_if_none_was_specified)
{
    // arrange
    unsigned char test_buffer[] = { 0x42 };
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    STRICT_EXPECTED_CALL(lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_ESTABLISHED, saved_ws_callback_context, NULL, 0);
    (void)wsio_send(wsio, test_buffer, sizeof(test_buffer), NULL, (void*)0x4243);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    STRICT_EXPECTED_CALL(singlylinkedlist_get_head_item(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE));
    EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(lws_write(TEST_LIBWEBSOCKET, IGNORED_PTR_ARG, sizeof(test_buffer), LWS_WRITE_BINARY))
        .ValidateArgumentBuffer(2, test_buffer, sizeof(test_buffer))
        .SetReturn((int)sizeof(test_buffer));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(singlylinkedlist_remove(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_get_head_item(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_WRITEABLE, saved_ws_callback_context, NULL, 0);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_051: [If the wsio is not OPEN (open has not been called or is still in progress) then wsio_send shall fail and return a non-zero value.] */
TEST_FUNCTION(when_ws_io_is_not_opened_yet_wsio_send_fails)
{
    // arrange
    unsigned char test_buffer[] = { 0x42 };
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    umock_c_reset_all_calls();

    // act
    int result = wsio_send(wsio, test_buffer, sizeof(test_buffer), test_on_send_complete, (void*)0x4243);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_051: [If the wsio is not OPEN (open has not been called or is still in progress) then wsio_send shall fail and return a non-zero value.] */
TEST_FUNCTION(when_ws_io_is_opening_wsio_send_fails)
{
    // arrange
    unsigned char test_buffer[] = { 0x42 };
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    umock_c_reset_all_calls();

    // act
    int result = wsio_send(wsio, test_buffer, sizeof(test_buffer), test_on_send_complete, (void*)0x4243);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_052: [If any of the arguments ws_io or buffer are NULL, wsio_send shall fail and return a non-zero value.] */
TEST_FUNCTION(when_wsio_is_NULL_wsio_send_fails)
{
    // arrange
    unsigned char test_buffer[] = { 0x42 };
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    STRICT_EXPECTED_CALL(lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_ESTABLISHED, saved_ws_callback_context, NULL, 0);
    umock_c_reset_all_calls();

    // act
    int result = wsio_send(NULL, test_buffer, sizeof(test_buffer), test_on_send_complete, (void*)0x4243);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_052: [If any of the arguments ws_io or buffer are NULL, wsio_send shall fail and return a non-zero value.] */
TEST_FUNCTION(when_buffer_is_NULL_wsio_send_fails)
{
    // arrange
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    STRICT_EXPECTED_CALL(lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_ESTABLISHED, saved_ws_callback_context, NULL, 0);
    umock_c_reset_all_calls();

    // act
    int result = wsio_send(wsio, NULL, 1, test_on_send_complete, (void*)0x4243);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_053: [If size is zero then wsio_send shall fail and return a non-zero value.] */
TEST_FUNCTION(when_size_is_zero_wsio_send_fails)
{
    // arrange
    unsigned char test_buffer[] = { 0x42 };
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    STRICT_EXPECTED_CALL(lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_ESTABLISHED, saved_ws_callback_context, NULL, 0);
    umock_c_reset_all_calls();

    // act
    int result = wsio_send(wsio, test_buffer, 0, test_on_send_complete, (void*)0x4243);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_108: [wsio_close shall obtain all the pending IO items by repetitively querying for the head of the pending IO list and freeing that head item.] */
/* Tests_SRS_WSIO_01_111: [Obtaining the head of the pending IO list shall be done by calling singlylinkedlist_get_head_item.] */
/* Tests_SRS_WSIO_01_109: [For each pending item the send complete callback shall be called with IO_SEND_CANCELLED.] */
/* Tests_SRS_WSIO_01_110: [The callback context passed to the on_send_complete callback shall be the context given to wsio_send.] */
TEST_FUNCTION(wsio_close_with_a_pending_send_cancels_the_send)
{
    // arrange
    unsigned char test_buffer[] = { 0x42 };
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    STRICT_EXPECTED_CALL(lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_ESTABLISHED, saved_ws_callback_context, NULL, 0);
    (void)wsio_send(wsio, test_buffer, sizeof(test_buffer), test_on_send_complete, (void*)0x4243);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(singlylinkedlist_get_head_item(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE));
    EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(test_on_send_complete((void*)0x4243, IO_SEND_CANCELLED));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(singlylinkedlist_remove(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_get_head_item(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE));
    STRICT_EXPECTED_CALL(lws_context_destroy(TEST_LIBWEBSOCKET_CONTEXT));

    // act
    (void)wsio_close(wsio, NULL, NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_110: [The callback context passed to the on_send_complete callback shall be the context given to wsio_send.] */
/* Tests_SRS_WSIO_01_059: [The callback_context argument shall be passed to on_send_complete as is.] */
TEST_FUNCTION(wsio_close_with_a_pending_send_cancels_the_send_and_passes_the_appropriate_callback_context)
{
    // arrange
    unsigned char test_buffer[] = { 0x42 };
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    STRICT_EXPECTED_CALL(lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_ESTABLISHED, saved_ws_callback_context, NULL, 0);
    (void)wsio_send(wsio, test_buffer, sizeof(test_buffer), test_on_send_complete, (void*)0x4243);
    (void)wsio_send(wsio, test_buffer, sizeof(test_buffer), test_on_send_complete, (void*)0x4244);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(singlylinkedlist_get_head_item(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE));
    EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(test_on_send_complete((void*)0x4243, IO_SEND_CANCELLED));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(singlylinkedlist_remove(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_get_head_item(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE));
    EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(test_on_send_complete((void*)0x4244, IO_SEND_CANCELLED));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(singlylinkedlist_remove(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_get_head_item(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE));
    STRICT_EXPECTED_CALL(lws_context_destroy(TEST_LIBWEBSOCKET_CONTEXT));

    // act
    (void)wsio_close(wsio, NULL, NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_060: [The argument on_send_complete shall be optional, if NULL is passed by the caller then no send complete callback shall be triggered.] */
TEST_FUNCTION(wsio_close_with_a_pending_send_cancels_the_send_but_doen_not_call_calback_if_no_callback_was_specified)
{
    // arrange
    unsigned char test_buffer[] = { 0x42 };
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    STRICT_EXPECTED_CALL(lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_ESTABLISHED, saved_ws_callback_context, NULL, 0);
    (void)wsio_send(wsio, test_buffer, sizeof(test_buffer), NULL, (void*)0x4243);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(singlylinkedlist_get_head_item(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE));
    EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(singlylinkedlist_remove(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_get_head_item(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE));
    STRICT_EXPECTED_CALL(lws_context_destroy(TEST_LIBWEBSOCKET_CONTEXT));

    // act
    (void)wsio_close(wsio, NULL, NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

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
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    STRICT_EXPECTED_CALL(lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_ESTABLISHED, saved_ws_callback_context, NULL, 0);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(lws_service(TEST_LIBWEBSOCKET_CONTEXT, 0));

    // act
    wsio_dowork(wsio);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_063: [If the ws_io argument is NULL, wsio_dowork shall do nothing.]  */
TEST_FUNCTION(wsio_dowork_with_NULL_handles_does_not_call_lws_service)
{
    // arrange

    // act
    wsio_dowork(NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_WSIO_01_061: [wsio_dowork shall service the libwebsockets context by calling lws_service and passing as argument the context obtained in wsio_open.] */
/* Tests_SRS_WSIO_01_112: [The timeout for lws_service shall be 0.] */
/* Tests_SRS_WSIO_01_062: [This shall be done if the IO is not closed.] */
TEST_FUNCTION(wsio_dowork_services_lws_when_still_opening)
{
    // arrange
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    STRICT_EXPECTED_CALL(lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(lws_service(TEST_LIBWEBSOCKET_CONTEXT, 0));

    // act
    wsio_dowork(wsio);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_062: [This shall be done if the IO is not closed.] */
TEST_FUNCTION(wsio_dowork_does_not_service_lws_when_not_yet_open)
{
    // arrange
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    umock_c_reset_all_calls();

    // act
    wsio_dowork(wsio);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_062: [This shall be done if the IO is not closed.] */
TEST_FUNCTION(wsio_dowork_does_not_service_lws_when_closed)
{
    // arrange
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    STRICT_EXPECTED_CALL(lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_ESTABLISHED, saved_ws_callback_context, NULL, 0);
    wsio_close(wsio, NULL, NULL);
    umock_c_reset_all_calls();

    // act
    wsio_dowork(wsio);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_destroy(wsio);
}

/* wsio_get_interface_description */

/* Tests_SRS_WSIO_01_064: [wsio_get_interface_description shall return a pointer to an IO_INTERFACE_DESCRIPTION structure that contains pointers to the functions: wsio_create, wsio_destroy, wsio_open, wsio_close, wsio_send and wsio_dowork.] */
TEST_FUNCTION(wsio_get_interface_description_fills_the_interface_structure)
{
    // arrange

    // act
    const IO_INTERFACE_DESCRIPTION* if_description = wsio_get_interface_description();

    // assert
    ASSERT_IS_TRUE(wsio_create == if_description->concrete_io_create);
    ASSERT_IS_TRUE(wsio_destroy == if_description->concrete_io_destroy);
    ASSERT_IS_TRUE(wsio_open == if_description->concrete_io_open);
    ASSERT_IS_TRUE(wsio_close == if_description->concrete_io_close);
    ASSERT_IS_TRUE(wsio_send == if_description->concrete_io_send);
    ASSERT_IS_TRUE(wsio_dowork == if_description->concrete_io_dowork);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* on_ws_callback */

/* Tests_SRS_WSIO_01_066: [If an open action is pending, the on_io_open_complete callback shall be triggered with IO_OPEN_OK and from now on it shall be possible to send/receive data.] */
TEST_FUNCTION(CLIENT_CONNECTED_when_opening_triggers_an_open_complete_callback)
{
    // arrange
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    STRICT_EXPECTED_CALL(test_on_io_open_complete((void*)0x4242, IO_OPEN_OK));

    // act
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_ESTABLISHED, saved_ws_callback_context, NULL, 0);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_068: [If the IO is already open, the on_io_error callback shall be triggered.] */
TEST_FUNCTION(CLIENT_CONNECTED_when_already_open_triggers_an_error)
{
    // arrange
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_ESTABLISHED, saved_ws_callback_context, NULL, 0);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    STRICT_EXPECTED_CALL(test_on_io_error((void*)0x4242));

    // act
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_ESTABLISHED, saved_ws_callback_context, NULL, 0);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_069: [If an open action is pending, the on_io_open_complete callback shall be triggered with IO_OPEN_ERROR.] */
TEST_FUNCTION(CLIENT_CONNECTION_ERROR_when_opening_yields_open_complete_with_error)
{
    // arrange
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    STRICT_EXPECTED_CALL(test_on_io_open_complete((void*)0x4242, IO_OPEN_ERROR));
    STRICT_EXPECTED_CALL(lws_context_destroy(TEST_LIBWEBSOCKET_CONTEXT));

    // act
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_CONNECTION_ERROR, saved_ws_callback_context, NULL, 0);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_069: [If an open action is pending, the on_io_open_complete callback shall be triggered with IO_OPEN_ERROR.] */
TEST_FUNCTION(CLIENT_CONNECTION_ERROR_when_opening_with_NULL_open_complete_callback_still_frees_the_lws_context)
{
    // arrange
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, NULL, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    STRICT_EXPECTED_CALL(lws_context_destroy(TEST_LIBWEBSOCKET_CONTEXT));

    // act
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_CONNECTION_ERROR, saved_ws_callback_context, NULL, 0);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_070: [If the IO is already open, the on_io_error callback shall be triggered.] */
TEST_FUNCTION(CLIENT_CONNECTION_ERROR_when_already_open_indicates_an_error_to_the_consumer)
{
    // arrange
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_ESTABLISHED, saved_ws_callback_context, NULL, 0);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    STRICT_EXPECTED_CALL(test_on_io_error((void*)0x4242));

    // act
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_CONNECTION_ERROR, saved_ws_callback_context, NULL, 0);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

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
    unsigned char test_buffer[] = { 0x42 };
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_ESTABLISHED, saved_ws_callback_context, NULL, 0);
    (void)wsio_send(wsio, test_buffer, sizeof(test_buffer), NULL, (void*)0x4243);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    STRICT_EXPECTED_CALL(singlylinkedlist_get_head_item(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE));
    EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(lws_write(TEST_LIBWEBSOCKET, IGNORED_PTR_ARG, sizeof(test_buffer), LWS_WRITE_BINARY))
        .ValidateArgumentBuffer(2, test_buffer, sizeof(test_buffer))
        .SetReturn((int)sizeof(test_buffer));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(singlylinkedlist_remove(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_get_head_item(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_WRITEABLE, saved_ws_callback_context, NULL, 0);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_121: [If this event is received in while an open action is incomplete, the open_complete callback shall be called with IO_OPEN_ERROR.] */
TEST_FUNCTION(CLIENT_WRITABLE_when_opening_yields_an_open_complete_error)
{
    // arrange
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    STRICT_EXPECTED_CALL(test_on_io_open_complete((void*)0x4242, IO_OPEN_ERROR));
    STRICT_EXPECTED_CALL(lws_context_destroy(TEST_LIBWEBSOCKET_CONTEXT));

    // act
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_WRITEABLE, saved_ws_callback_context, NULL, 0);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_071: [If any pending IO chunks queued in wsio_send are to be sent, then the first one shall be retrieved from the queue.] */
TEST_FUNCTION(when_no_items_are_pending_in_CLIENT_WRITABLE_nothing_happens)
{
    // arrange
    unsigned char test_buffer[] = { 0x42 };
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_ESTABLISHED, saved_ws_callback_context, NULL, 0);
    (void)wsio_send(wsio, test_buffer, sizeof(test_buffer), NULL, (void*)0x4243);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    STRICT_EXPECTED_CALL(singlylinkedlist_get_head_item(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE))
        .SetReturn((LIST_ITEM_HANDLE)NULL);

    // act
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_WRITEABLE, saved_ws_callback_context, NULL, 0);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_071: [If any pending IO chunks queued in wsio_send are to be sent, then the first one shall be retrieved from the queue.] */
TEST_FUNCTION(when_getting_the_pending_io_data_in_CLIENT_WRITABLE_fails_then_an_error_is_indicated)
{
    // arrange
    unsigned char test_buffer[] = { 0x42 };
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_ESTABLISHED, saved_ws_callback_context, NULL, 0);
    (void)wsio_send(wsio, test_buffer, sizeof(test_buffer), NULL, (void*)0x4243);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    STRICT_EXPECTED_CALL(singlylinkedlist_get_head_item(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE));
    EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG))
        .SetReturn((void*)NULL);
    STRICT_EXPECTED_CALL(test_on_io_error((void*)0x4242));

    // act
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_WRITEABLE, saved_ws_callback_context, NULL, 0);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_073: [If allocating the memory fails then the send_result callback callback shall be triggered with IO_SEND_ERROR.] */
/* Tests_SRS_WSIO_01_081: [If no errors prohibiting further processing of other pending IO chunks have happened, then lws_callback_on_writable shall be called, while passing the websockets context and interface obtained in wsio_open as arguments.] */
TEST_FUNCTION(when_allocating_memory_for_lws_in_CLIENT_WRITABLE_fails_then_an_error_is_indicated)
{
    // arrange
    unsigned char test_buffer[] = { 0x42 };
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_ESTABLISHED, saved_ws_callback_context, NULL, 0);
    (void)wsio_send(wsio, test_buffer, sizeof(test_buffer), test_on_send_complete, (void*)0x4243);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    STRICT_EXPECTED_CALL(singlylinkedlist_get_head_item(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE));
    EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG))
        .SetReturn((void*)NULL);
    STRICT_EXPECTED_CALL(test_on_send_complete((void*)0x4243, IO_SEND_ERROR));
    STRICT_EXPECTED_CALL(singlylinkedlist_get_head_item(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE))
        .SetReturn((LIST_ITEM_HANDLE)NULL);
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(singlylinkedlist_remove(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE, IGNORED_PTR_ARG));

    // act
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_WRITEABLE, saved_ws_callback_context, NULL, 0);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_076: [If lws_write fails (result is less than 0) then the send_complete callback shall be triggered with IO_SEND_ERROR.] */
TEST_FUNCTION(when_lws_write_fails_in_CLIENT_WRITABLE_then_an_error_is_indicated)
{
    // arrange
    unsigned char test_buffer[] = { 0x42 };
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_ESTABLISHED, saved_ws_callback_context, NULL, 0);
    (void)wsio_send(wsio, test_buffer, sizeof(test_buffer), test_on_send_complete, (void*)0x4243);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    STRICT_EXPECTED_CALL(singlylinkedlist_get_head_item(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE));
    EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(lws_write(TEST_LIBWEBSOCKET, IGNORED_PTR_ARG, sizeof(test_buffer), LWS_WRITE_BINARY))
        .ValidateArgumentBuffer(2, test_buffer, sizeof(test_buffer))
        .SetReturn(-1);
    STRICT_EXPECTED_CALL(test_on_send_complete((void*)0x4243, IO_SEND_ERROR));
    STRICT_EXPECTED_CALL(singlylinkedlist_get_head_item(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE))
        .SetReturn((LIST_ITEM_HANDLE)NULL);
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(singlylinkedlist_remove(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE, IGNORED_PTR_ARG));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_WRITEABLE, saved_ws_callback_context, NULL, 0);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_113: [If allocating the memory fails for a pending IO that has been partially sent already then the on_io_error callback shall also be triggered.] */
TEST_FUNCTION(when_allocating_memory_for_the_lws_write_fails_in_CLIENT_WRITABLE_for_a_pending_io_that_was_partially_sent_an_error_is_indicated)
{
    // arrange
    unsigned char test_buffer[] = { 0x42, 0x43 };
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_ESTABLISHED, saved_ws_callback_context, NULL, 0);
    (void)wsio_send(wsio, test_buffer, sizeof(test_buffer), test_on_send_complete, (void*)0x4243);

    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    STRICT_EXPECTED_CALL(lws_write(TEST_LIBWEBSOCKET, IGNORED_PTR_ARG, sizeof(test_buffer), LWS_WRITE_BINARY))
        .ValidateArgumentBuffer(2, test_buffer, sizeof(test_buffer))
        .SetReturn(1);
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_WRITEABLE, saved_ws_callback_context, NULL, 0);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    STRICT_EXPECTED_CALL(singlylinkedlist_get_head_item(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE));
    EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG))
        .SetReturn((void*)NULL);
    STRICT_EXPECTED_CALL(test_on_send_complete((void*)0x4243, IO_SEND_ERROR));
    STRICT_EXPECTED_CALL(test_on_io_error((void*)0x4242));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(singlylinkedlist_remove(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE, IGNORED_PTR_ARG));

    // act
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_WRITEABLE, saved_ws_callback_context, NULL, 0);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_114: [Additionally, if the failure is for a pending IO that has been partially sent already then the on_io_error callback shall also be triggered.] */
TEST_FUNCTION(when_lws_write_fails_in_CLIENT_WRITABLE_for_a_pending_io_that_was_partially_sent_an_error_is_indicated)
{
    // arrange
    unsigned char test_buffer[] = { 0x42, 0x43 };
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_ESTABLISHED, saved_ws_callback_context, NULL, 0);
    (void)wsio_send(wsio, test_buffer, sizeof(test_buffer), test_on_send_complete, (void*)0x4243);

    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    STRICT_EXPECTED_CALL(lws_write(TEST_LIBWEBSOCKET, IGNORED_PTR_ARG, sizeof(test_buffer), LWS_WRITE_BINARY))
        .ValidateArgumentBuffer(2, test_buffer, sizeof(test_buffer))
        .SetReturn(1);
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_WRITEABLE, saved_ws_callback_context, NULL, 0);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    STRICT_EXPECTED_CALL(singlylinkedlist_get_head_item(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE));
    EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(lws_write(TEST_LIBWEBSOCKET, IGNORED_PTR_ARG, 1, LWS_WRITE_BINARY))
        .ValidateArgumentBuffer(2, test_buffer + 1, 1)
        .SetReturn(-1);
    STRICT_EXPECTED_CALL(test_on_send_complete((void*)0x4243, IO_SEND_ERROR));
    STRICT_EXPECTED_CALL(test_on_io_error((void*)0x4242));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(singlylinkedlist_remove(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE, IGNORED_PTR_ARG));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_WRITEABLE, saved_ws_callback_context, NULL, 0);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_079: [If the send was successful and any error occurs during removing the pending IO from the list then the on_io_error callback shall be triggered.]  */
TEST_FUNCTION(when_removing_the_pending_IO_after_a_succesfull_write_lws_write_fails_in_CLIENT_WRITABLE_then_an_error_is_indicated)
{
    // arrange
    unsigned char test_buffer[] = { 0x42, 0x43 };
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_ESTABLISHED, saved_ws_callback_context, NULL, 0);
    (void)wsio_send(wsio, test_buffer, sizeof(test_buffer), test_on_send_complete, (void*)0x4243);
    umock_c_reset_all_calls();

    singlylinkedlist_remove_result = 1;

    STRICT_EXPECTED_CALL(lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    STRICT_EXPECTED_CALL(singlylinkedlist_get_head_item(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE));
    EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(lws_write(TEST_LIBWEBSOCKET, IGNORED_PTR_ARG, sizeof(test_buffer), LWS_WRITE_BINARY))
        .ValidateArgumentBuffer(2, test_buffer, sizeof(test_buffer))
        .SetReturn(2);
    STRICT_EXPECTED_CALL(test_on_send_complete((void*)0x4243, IO_SEND_OK));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(singlylinkedlist_remove(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(test_on_io_error((void*)0x4242));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_WRITEABLE, saved_ws_callback_context, NULL, 0);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_081: [If any pending IOs are in the list, lws_callback_on_writable shall be called, while passing the websockets instance obtained in wsio_open as arguments if:] */
/* Tests_SRS_WSIO_01_116: [The send failed writing to lws or allocating memory for the data to be passed to lws and no partial data has been sent previously for the pending IO.] */
TEST_FUNCTION(when_allocating_memory_for_lws_in_CLIENT_WRITABLE_and_another_pending_IO_exists_then_callback_on_writable_is_called)
{
    // arrange
    unsigned char test_buffer[] = { 0x42 };
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_ESTABLISHED, saved_ws_callback_context, NULL, 0);
    (void)wsio_send(wsio, test_buffer, sizeof(test_buffer), test_on_send_complete, (void*)0x4243);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    STRICT_EXPECTED_CALL(singlylinkedlist_get_head_item(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE));
    EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG))
        .SetReturn((void*)NULL);
    STRICT_EXPECTED_CALL(test_on_send_complete((void*)0x4243, IO_SEND_ERROR));
    STRICT_EXPECTED_CALL(singlylinkedlist_get_head_item(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE));
    STRICT_EXPECTED_CALL(lws_callback_on_writable(TEST_LIBWEBSOCKET));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(singlylinkedlist_remove(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE, IGNORED_PTR_ARG));

    // act
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_WRITEABLE, saved_ws_callback_context, NULL, 0);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_081: [If any pending IOs are in the list, lws_callback_on_writable shall be called, while passing the websockets instance obtained in wsio_open as arguments if:] */
/* Tests_SRS_WSIO_01_116: [The send failed writing to lws or allocating memory for the data to be passed to lws and no partial data has been sent previously for the pending IO.] */
TEST_FUNCTION(when_lws_write_in_CLIENT_WRITABLE_fails_and_another_pending_IO_exists_then_callback_on_writable_is_called)
{
    // arrange
    unsigned char test_buffer[] = { 0x42 };
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_ESTABLISHED, saved_ws_callback_context, NULL, 0);
    (void)wsio_send(wsio, test_buffer, sizeof(test_buffer), test_on_send_complete, (void*)0x4243);
    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    STRICT_EXPECTED_CALL(singlylinkedlist_get_head_item(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE));
    EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(lws_write(TEST_LIBWEBSOCKET, IGNORED_PTR_ARG, sizeof(test_buffer), LWS_WRITE_BINARY))
        .ValidateArgumentBuffer(2, test_buffer, sizeof(test_buffer))
        .SetReturn(-1);
    STRICT_EXPECTED_CALL(test_on_send_complete((void*)0x4243, IO_SEND_ERROR));
    STRICT_EXPECTED_CALL(singlylinkedlist_get_head_item(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE));
    STRICT_EXPECTED_CALL(lws_callback_on_writable(TEST_LIBWEBSOCKET));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(singlylinkedlist_remove(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE, IGNORED_PTR_ARG));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_WRITEABLE, saved_ws_callback_context, NULL, 0);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_081: [If any pending IOs are in the list, lws_callback_on_writable shall be called, while passing the websockets instance obtained in wsio_open as arguments if:] */
/* Tests_SRS_WSIO_01_116: [The send failed writing to lws or allocating memory for the data to be passed to lws and no partial data has been sent previously for the pending IO.] */
TEST_FUNCTION(when_allocating_memory_in_CLIENT_WRITABLE_fails_after_a_partial_write_and_another_pending_IO_exists_then_callback_on_writable_is_not_called)
{
    // arrange
    unsigned char test_buffer[] = { 0x42, 0x43 };
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_ESTABLISHED, saved_ws_callback_context, NULL, 0);
    (void)wsio_send(wsio, test_buffer, sizeof(test_buffer), test_on_send_complete, (void*)0x4243);
    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    STRICT_EXPECTED_CALL(lws_write(TEST_LIBWEBSOCKET, IGNORED_PTR_ARG, sizeof(test_buffer), LWS_WRITE_BINARY))
        .ValidateArgumentBuffer(2, test_buffer, sizeof(test_buffer))
        .SetReturn(1);
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_WRITEABLE, saved_ws_callback_context, NULL, 0);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    STRICT_EXPECTED_CALL(singlylinkedlist_get_head_item(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE));
    EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG))
        .SetReturn((void*)NULL);
    STRICT_EXPECTED_CALL(test_on_send_complete((void*)0x4243, IO_SEND_ERROR));
    STRICT_EXPECTED_CALL(test_on_io_error((void*)0x4242));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(singlylinkedlist_remove(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE, IGNORED_PTR_ARG));

    // act
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_WRITEABLE, saved_ws_callback_context, NULL, 0);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_081: [If any pending IOs are in the list, lws_callback_on_writable shall be called, while passing the websockets instance obtained in wsio_open as arguments if:] */
/* Tests_SRS_WSIO_01_116: [The send failed writing to lws or allocating memory for the data to be passed to lws and no partial data has been sent previously for the pending IO.] */
TEST_FUNCTION(when_lws_write_in_CLIENT_WRITABLE_fails_after_a_partial_write_and_another_pending_IO_exists_then_callback_on_writable_is_not_called)
{
    // arrange
    unsigned char test_buffer[] = { 0x42, 0x43 };
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_ESTABLISHED, saved_ws_callback_context, NULL, 0);
    (void)wsio_send(wsio, test_buffer, sizeof(test_buffer), test_on_send_complete, (void*)0x4243);
    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    STRICT_EXPECTED_CALL(lws_write(TEST_LIBWEBSOCKET, IGNORED_PTR_ARG, sizeof(test_buffer), LWS_WRITE_BINARY))
        .ValidateArgumentBuffer(2, test_buffer, sizeof(test_buffer))
        .SetReturn(1);
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_WRITEABLE, saved_ws_callback_context, NULL, 0);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    STRICT_EXPECTED_CALL(singlylinkedlist_get_head_item(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE));
    EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(lws_write(TEST_LIBWEBSOCKET, IGNORED_PTR_ARG, 1, LWS_WRITE_BINARY))
        .ValidateArgumentBuffer(2, test_buffer + 1, 1)
        .SetReturn(-1);
    STRICT_EXPECTED_CALL(test_on_send_complete((void*)0x4243, IO_SEND_ERROR));
    STRICT_EXPECTED_CALL(test_on_io_error((void*)0x4242));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(singlylinkedlist_remove(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE, IGNORED_PTR_ARG));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_WRITEABLE, saved_ws_callback_context, NULL, 0);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_081: [If any pending IOs are in the list, lws_callback_on_writable shall be called, while passing the websockets instance obtained in wsio_open as arguments if:] */
/* Tests_SRS_WSIO_01_115: [The send over websockets was successful] */
TEST_FUNCTION(when_send_is_succesfull_and_there_is_another_pending_IO_in_CLIENT_WRITABLE_then_callback_on_writable_is_called)
{
    // arrange
    unsigned char test_buffer[] = { 0x42, 0x43 };
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_ESTABLISHED, saved_ws_callback_context, NULL, 0);
    (void)wsio_send(wsio, test_buffer, sizeof(test_buffer), test_on_send_complete, (void*)0x4243);
    (void)wsio_send(wsio, test_buffer, sizeof(test_buffer), test_on_send_complete, (void*)0x4243);
    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    STRICT_EXPECTED_CALL(lws_write(TEST_LIBWEBSOCKET, IGNORED_PTR_ARG, sizeof(test_buffer), LWS_WRITE_BINARY))
        .ValidateArgumentBuffer(2, test_buffer, sizeof(test_buffer))
        .SetReturn(1);
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_WRITEABLE, saved_ws_callback_context, NULL, 0);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    STRICT_EXPECTED_CALL(singlylinkedlist_get_head_item(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE));
    EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(lws_write(TEST_LIBWEBSOCKET, IGNORED_PTR_ARG, 1, LWS_WRITE_BINARY))
        .ValidateArgumentBuffer(2, test_buffer + 1, 1)
        .SetReturn(1);
    STRICT_EXPECTED_CALL(test_on_send_complete((void*)0x4243, IO_SEND_OK));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(singlylinkedlist_remove(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_get_head_item(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE));
    STRICT_EXPECTED_CALL(lws_callback_on_writable(TEST_LIBWEBSOCKET));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_WRITEABLE, saved_ws_callback_context, NULL, 0);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_117: [on_io_error should not be triggered twice when removing a pending IO that failed and a partial send for it has already been done.]  */
TEST_FUNCTION(when_removing_the_pending_IO_due_to_lws_write_failing_in_CLIENT_WRITABLE_fails_then_on_io_error_is_called_only_once)
{
    // arrange
    unsigned char test_buffer[] = { 0x42, 0x43 };
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_ESTABLISHED, saved_ws_callback_context, NULL, 0);
    (void)wsio_send(wsio, test_buffer, sizeof(test_buffer), test_on_send_complete, (void*)0x4243);
    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    STRICT_EXPECTED_CALL(lws_write(TEST_LIBWEBSOCKET, IGNORED_PTR_ARG, sizeof(test_buffer), LWS_WRITE_BINARY))
        .ValidateArgumentBuffer(2, test_buffer, sizeof(test_buffer))
        .SetReturn(1);
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_WRITEABLE, saved_ws_callback_context, NULL, 0);
    umock_c_reset_all_calls();

    singlylinkedlist_remove_result = 1;

    STRICT_EXPECTED_CALL(lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    STRICT_EXPECTED_CALL(singlylinkedlist_get_head_item(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE));
    EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(lws_write(TEST_LIBWEBSOCKET, IGNORED_PTR_ARG, 1, LWS_WRITE_BINARY))
        .ValidateArgumentBuffer(2, test_buffer + 1, 1)
        .SetReturn(-1);
    STRICT_EXPECTED_CALL(test_on_send_complete((void*)0x4243, IO_SEND_ERROR));
    STRICT_EXPECTED_CALL(test_on_io_error((void*)0x4242));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(singlylinkedlist_remove(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE, IGNORED_PTR_ARG));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_WRITEABLE, saved_ws_callback_context, NULL, 0);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_117: [on_io_error should not be triggered twice when removing a pending IO that failed and a partial send for it has already been done.]  */
TEST_FUNCTION(when_removing_the_pending_IO_due_to_allocating_memory_failing_in_CLIENT_WRITABLE_fails_then_on_io_error_is_called_only_once)
{
    // arrange
    unsigned char test_buffer[] = { 0x42, 0x43 };
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_ESTABLISHED, saved_ws_callback_context, NULL, 0);
    (void)wsio_send(wsio, test_buffer, sizeof(test_buffer), test_on_send_complete, (void*)0x4243);
    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    STRICT_EXPECTED_CALL(lws_write(TEST_LIBWEBSOCKET, IGNORED_PTR_ARG, sizeof(test_buffer), LWS_WRITE_BINARY))
        .ValidateArgumentBuffer(2, test_buffer, sizeof(test_buffer))
        .SetReturn(1);
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_WRITEABLE, saved_ws_callback_context, NULL, 0);
    umock_c_reset_all_calls();

    singlylinkedlist_remove_result = 1;

    STRICT_EXPECTED_CALL(lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    STRICT_EXPECTED_CALL(singlylinkedlist_get_head_item(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE));
    EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG))
        .SetReturn((void*)NULL);
    STRICT_EXPECTED_CALL(test_on_send_complete((void*)0x4243, IO_SEND_ERROR));
    STRICT_EXPECTED_CALL(test_on_io_error((void*)0x4242));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(singlylinkedlist_remove(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE, IGNORED_PTR_ARG));

    // act
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_WRITEABLE, saved_ws_callback_context, NULL, 0);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_080: [If lws_write succeeds and less bytes than the complete payload have been sent, then the sent bytes shall be removed from the pending IO and only the leftover bytes shall be left as pending and sent upon subsequent events.] */
TEST_FUNCTION(sending_partial_content_leaves_the_bytes_for_the_next_writable_event)
{
    // arrange
    unsigned char test_buffer[] = { 0x42, 0x43 };
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_ESTABLISHED, saved_ws_callback_context, NULL, 0);
    (void)wsio_send(wsio, test_buffer, sizeof(test_buffer), test_on_send_complete, (void*)0x4243);
    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    STRICT_EXPECTED_CALL(lws_write(TEST_LIBWEBSOCKET, IGNORED_PTR_ARG, sizeof(test_buffer), LWS_WRITE_BINARY))
        .ValidateArgumentBuffer(2, test_buffer, sizeof(test_buffer))
        .SetReturn(1);
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_WRITEABLE, saved_ws_callback_context, NULL, 0);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    STRICT_EXPECTED_CALL(singlylinkedlist_get_head_item(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE));
    EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(lws_write(TEST_LIBWEBSOCKET, IGNORED_PTR_ARG, 1, LWS_WRITE_BINARY))
        .ValidateArgumentBuffer(2, test_buffer + 1, 1)
        .SetReturn(1);
    STRICT_EXPECTED_CALL(test_on_send_complete((void*)0x4243, IO_SEND_OK));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(singlylinkedlist_remove(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_get_head_item(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_WRITEABLE, saved_ws_callback_context, NULL, 0);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_080: [If lws_write succeeds and less bytes than the complete payload have been sent, then the sent bytes shall be removed from the pending IO and only the leftover bytes shall be left as pending and sent upon subsequent events.] */
TEST_FUNCTION(sending_partial_content_of_2_bytes_works_and_leaves_the_bytes_for_the_next_writable_event)
{
    // arrange
    unsigned char test_buffer[] = { 0x42, 0x43, 0x44 };
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_ESTABLISHED, saved_ws_callback_context, NULL, 0);
    (void)wsio_send(wsio, test_buffer, sizeof(test_buffer), test_on_send_complete, (void*)0x4243);
    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    STRICT_EXPECTED_CALL(lws_write(TEST_LIBWEBSOCKET, IGNORED_PTR_ARG, sizeof(test_buffer), LWS_WRITE_BINARY))
        .ValidateArgumentBuffer(2, test_buffer, sizeof(test_buffer))
        .SetReturn(2);
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_WRITEABLE, saved_ws_callback_context, NULL, 0);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    STRICT_EXPECTED_CALL(singlylinkedlist_get_head_item(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE));
    EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(lws_write(TEST_LIBWEBSOCKET, IGNORED_PTR_ARG, 1, LWS_WRITE_BINARY))
        .ValidateArgumentBuffer(2, test_buffer + 2, 1)
        .SetReturn(1);
    STRICT_EXPECTED_CALL(test_on_send_complete((void*)0x4243, IO_SEND_OK));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(singlylinkedlist_remove(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_get_head_item(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_WRITEABLE, saved_ws_callback_context, NULL, 0);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_118: [If lws_write indicates more bytes sent than were passed to it an error shall be indicated via on_io_error.] */
TEST_FUNCTION(when_more_bytes_than_requested_are_indicated_by_lws_write_on_send_complete_indicates_an_error)
{
    // arrange
    unsigned char test_buffer[] = { 0x42 };
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_ESTABLISHED, saved_ws_callback_context, NULL, 0);
    (void)wsio_send(wsio, test_buffer, sizeof(test_buffer), test_on_send_complete, (void*)0x4243);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    STRICT_EXPECTED_CALL(singlylinkedlist_get_head_item(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE));
    EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(lws_write(TEST_LIBWEBSOCKET, IGNORED_PTR_ARG, sizeof(test_buffer), LWS_WRITE_BINARY))
        .ValidateArgumentBuffer(2, test_buffer, sizeof(test_buffer))
        .SetReturn(2);
    STRICT_EXPECTED_CALL(test_on_send_complete((void*)0x4243, IO_SEND_ERROR));
    STRICT_EXPECTED_CALL(singlylinkedlist_get_head_item(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE))
        .SetReturn((LIST_ITEM_HANDLE)NULL);
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(singlylinkedlist_remove(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE, IGNORED_PTR_ARG));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_WRITEABLE, saved_ws_callback_context, NULL, 0);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_119: [If this error happens after the pending IO being partially sent, the on_io_error shall also be indicated.] */
TEST_FUNCTION(when_more_bytes_than_requested_are_indicated_by_lws_write_and_a_partial_send_has_been_done_then_on_io_error_is_triggered)
{
    // arrange
    unsigned char test_buffer[] = { 0x42, 0x43 };
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_ESTABLISHED, saved_ws_callback_context, NULL, 0);
    (void)wsio_send(wsio, test_buffer, sizeof(test_buffer), test_on_send_complete, (void*)0x4243);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    STRICT_EXPECTED_CALL(lws_write(TEST_LIBWEBSOCKET, IGNORED_PTR_ARG, sizeof(test_buffer), LWS_WRITE_BINARY))
        .ValidateArgumentBuffer(2, test_buffer, sizeof(test_buffer))
        .SetReturn(1);
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_WRITEABLE, saved_ws_callback_context, NULL, 0);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    STRICT_EXPECTED_CALL(singlylinkedlist_get_head_item(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE));
    EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(lws_write(TEST_LIBWEBSOCKET, IGNORED_PTR_ARG, 1, LWS_WRITE_BINARY))
        .ValidateArgumentBuffer(2, test_buffer + 1, 1)
        .SetReturn(2);
    STRICT_EXPECTED_CALL(test_on_send_complete((void*)0x4243, IO_SEND_ERROR));
    STRICT_EXPECTED_CALL(test_on_io_error((void*)0x4242));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(singlylinkedlist_remove(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE, IGNORED_PTR_ARG));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_WRITEABLE, saved_ws_callback_context, NULL, 0);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

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
    unsigned char test_buffer[] = { 0x42 };
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    STRICT_EXPECTED_CALL(lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_ESTABLISHED, saved_ws_callback_context, NULL, 0);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    STRICT_EXPECTED_CALL(test_on_bytes_received((void*)0x4242, IGNORED_PTR_ARG, sizeof(test_buffer)))
        .ValidateArgumentBuffer(2, test_buffer, sizeof(test_buffer));

    // act
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_RECEIVE, saved_ws_callback_context, test_buffer, sizeof(test_buffer));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

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
    unsigned char test_buffer[] = { 0x42, 0x43 };
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    STRICT_EXPECTED_CALL(lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_ESTABLISHED, saved_ws_callback_context, NULL, 0);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    STRICT_EXPECTED_CALL(test_on_bytes_received((void*)0x4242, IGNORED_PTR_ARG, sizeof(test_buffer)))
        .ValidateArgumentBuffer(2, test_buffer, sizeof(test_buffer));

    // act
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_RECEIVE, saved_ws_callback_context, test_buffer, sizeof(test_buffer));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_122: [If an open action is in progress then the on_open_complete callback shall be invoked with IO_OPEN_ERROR.] */
TEST_FUNCTION(CLIENT_RECEIVE_while_opening_triggers_an_open_complete_with_IO_OPEN_ERROR)
{
    // arrange
    unsigned char test_buffer[] = { 0x42 };
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    STRICT_EXPECTED_CALL(test_on_io_open_complete((void*)0x4242, IO_OPEN_ERROR));
    STRICT_EXPECTED_CALL(lws_context_destroy(TEST_LIBWEBSOCKET_CONTEXT));

    // act
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_RECEIVE, saved_ws_callback_context, test_buffer, sizeof(test_buffer));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_086: [The callback_context shall be set to the callback_context that was passed in wsio_open.] */
TEST_FUNCTION(CLIENT_RECEIVE_passes_the_proper_context_to_on_bytes_received)
{
    // arrange
    unsigned char test_buffer[] = { 0x42, 0x43 };
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4546, test_on_bytes_received, (void*)0x4546, test_on_io_error, (void*)0x4546);
    STRICT_EXPECTED_CALL(lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_ESTABLISHED, saved_ws_callback_context, NULL, 0);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    STRICT_EXPECTED_CALL(test_on_bytes_received((void*)0x4546, IGNORED_PTR_ARG, sizeof(test_buffer)))
        .ValidateArgumentBuffer(2, test_buffer, sizeof(test_buffer));

    // act
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_RECEIVE, saved_ws_callback_context, test_buffer, sizeof(test_buffer));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_087: [If the number of bytes is 0 then the on_io_error callback shall be called.] */
TEST_FUNCTION(CLIENT_RECEIVE_with_0_bytes_yields_an_error)
{
    // arrange
    unsigned char test_buffer[] = { 0x42, 0x43 };
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4546, test_on_bytes_received, (void*)0x4546, test_on_io_error, (void*)0x4546);
    STRICT_EXPECTED_CALL(lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_ESTABLISHED, saved_ws_callback_context, NULL, 0);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    STRICT_EXPECTED_CALL(test_on_io_error((void*)0x4546));

    // act
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_RECEIVE, saved_ws_callback_context, test_buffer, 0);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_088: [If the number of bytes received is positive, but the buffer indicated by the in parameter is NULL, then the on_io_error callback shall be called.] */
TEST_FUNCTION(CLIENT_RECEIVE_with_NULL_input_buffer_yields_an_error)
{
    // arrange
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4546, test_on_bytes_received, (void*)0x4546, test_on_io_error, (void*)0x4546);
    STRICT_EXPECTED_CALL(lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_ESTABLISHED, saved_ws_callback_context, NULL, 0);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    STRICT_EXPECTED_CALL(test_on_io_error((void*)0x4546));

    // act
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_RECEIVE, saved_ws_callback_context, NULL, 1);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

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
    unsigned char test_buffer1[] = { 0x42, 0x43 };
    unsigned char test_buffer2[] = { 0x44 };
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    STRICT_EXPECTED_CALL(lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_ESTABLISHED, saved_ws_callback_context, NULL, 0);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    STRICT_EXPECTED_CALL(test_on_bytes_received((void*)0x4242, IGNORED_PTR_ARG, sizeof(test_buffer1)))
        .ValidateArgumentBuffer(2, test_buffer1, sizeof(test_buffer1));
    STRICT_EXPECTED_CALL(lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    STRICT_EXPECTED_CALL(test_on_bytes_received((void*)0x4242, IGNORED_PTR_ARG, sizeof(test_buffer2)))
        .ValidateArgumentBuffer(2, test_buffer2, sizeof(test_buffer2));

    // act
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_RECEIVE, saved_ws_callback_context, test_buffer1, sizeof(test_buffer1));
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_RECEIVE, saved_ws_callback_context, test_buffer2, sizeof(test_buffer2));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

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
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    (void)wsio_setoption(wsio, "TrustedCerts", "boohoo");
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    STRICT_EXPECTED_CALL(SSL_CTX_get_cert_store(TEST_SSL_CONTEXT));
    STRICT_EXPECTED_CALL(BIO_s_mem());
    STRICT_EXPECTED_CALL(BIO_new(TEST_BIO_METHOD));
    STRICT_EXPECTED_CALL(BIO_puts(TEST_BIO, "boohoo"))
        .SetReturn((int)strlen("boohoo"));
    STRICT_EXPECTED_CALL(PEM_read_bio_X509(TEST_BIO, NULL, NULL, NULL))
        .SetReturn((X509*)NULL);
    STRICT_EXPECTED_CALL(BIO_free(TEST_BIO));

    // act
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_OPENSSL_LOAD_EXTRA_CLIENT_VERIFY_CERTS, (void*)TEST_SSL_CONTEXT, NULL, 0);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_126: [Reading every certificate by calling PEM_read_bio_X509] */
/* Tests_SRS_WSIO_01_127: [Adding the read certificate to the store by calling X509_STORE_add_cert]  */
TEST_FUNCTION(LOAD_EXTRA_CLIENT_VERIFY_CERTS_with_a_string_that_has_1_cert_loads_that_cert)
{
    // arrange
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    (void)wsio_setoption(wsio, "TrustedCerts", "boohoo");
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    STRICT_EXPECTED_CALL(SSL_CTX_get_cert_store(TEST_SSL_CONTEXT));
    STRICT_EXPECTED_CALL(BIO_s_mem());
    STRICT_EXPECTED_CALL(BIO_new(TEST_BIO_METHOD));
    STRICT_EXPECTED_CALL(BIO_puts(TEST_BIO, "boohoo"))
        .SetReturn((int)strlen("boohoo"));
    STRICT_EXPECTED_CALL(PEM_read_bio_X509(TEST_BIO, NULL, NULL, NULL))
        .SetReturn(TEST_X509_CERT_1);
    STRICT_EXPECTED_CALL(X509_STORE_add_cert(TEST_CERT_STORE, TEST_X509_CERT_1));
    STRICT_EXPECTED_CALL(PEM_read_bio_X509(TEST_BIO, NULL, NULL, NULL))
        .SetReturn((X509*)NULL);
    STRICT_EXPECTED_CALL(BIO_free(TEST_BIO));

    // act
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_OPENSSL_LOAD_EXTRA_CLIENT_VERIFY_CERTS, (void*)TEST_SSL_CONTEXT, NULL, 0);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_126: [Reading every certificate by calling PEM_read_bio_X509] */
/* Tests_SRS_WSIO_01_127: [Adding the read certificate to the store by calling X509_STORE_add_cert] */
TEST_FUNCTION(LOAD_EXTRA_CLIENT_VERIFY_CERTS_with_a_string_that_has_2_certs_loads_that_certs)
{
    // arrange
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    (void)wsio_setoption(wsio, "TrustedCerts", "boohoo");
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    STRICT_EXPECTED_CALL(SSL_CTX_get_cert_store(TEST_SSL_CONTEXT));
    STRICT_EXPECTED_CALL(BIO_s_mem());
    STRICT_EXPECTED_CALL(BIO_new(TEST_BIO_METHOD));
    STRICT_EXPECTED_CALL(BIO_puts(TEST_BIO, "boohoo"))
        .SetReturn((int)strlen("boohoo"));
    STRICT_EXPECTED_CALL(PEM_read_bio_X509(TEST_BIO, NULL, NULL, NULL))
        .SetReturn(TEST_X509_CERT_1);
    STRICT_EXPECTED_CALL(X509_STORE_add_cert(TEST_CERT_STORE, TEST_X509_CERT_1));
    STRICT_EXPECTED_CALL(PEM_read_bio_X509(TEST_BIO, NULL, NULL, NULL))
        .SetReturn(TEST_X509_CERT_2);
    STRICT_EXPECTED_CALL(X509_STORE_add_cert(TEST_CERT_STORE, TEST_X509_CERT_2));
    STRICT_EXPECTED_CALL(PEM_read_bio_X509(TEST_BIO, NULL, NULL, NULL))
        .SetReturn((X509*)NULL);
    STRICT_EXPECTED_CALL(BIO_free(TEST_BIO));

    // act
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_OPENSSL_LOAD_EXTRA_CLIENT_VERIFY_CERTS, (void*)TEST_SSL_CONTEXT, NULL, 0);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_130: [If the event is received when the IO is already open the on_io_error callback shall be triggered.] */
TEST_FUNCTION(LOAD_EXTRA_CLIENT_VERIFY_CERTS_when_already_open_yields_an_error)
{
    // arrange
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    STRICT_EXPECTED_CALL(lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_CLIENT_ESTABLISHED, saved_ws_callback_context, NULL, 0);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    STRICT_EXPECTED_CALL(test_on_io_error((void*)0x4242));

    // act
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_OPENSSL_LOAD_EXTRA_CLIENT_VERIFY_CERTS, (void*)TEST_SSL_CONTEXT, NULL, 0);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_129: [If any of the APIs fails and an open call is pending the on_open_complete callback shall be triggered with IO_OPEN_ERROR.] */
TEST_FUNCTION(when_getting_the_cert_store_fails_in_LOAD_EXTRA_CLIENT_VERIFY_CERTS_an_on_open_complete_is_triggered_with_error)
{
    // arrange
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    (void)wsio_setoption(wsio, "TrustedCerts", "boohoo");
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    STRICT_EXPECTED_CALL(SSL_CTX_get_cert_store(TEST_SSL_CONTEXT))
        .SetReturn((X509_STORE*)NULL);
    STRICT_EXPECTED_CALL(test_on_io_open_complete((void*)0x4242, IO_OPEN_ERROR));

    // act
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_OPENSSL_LOAD_EXTRA_CLIENT_VERIFY_CERTS, (void*)TEST_SSL_CONTEXT, NULL, 0);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_129: [If any of the APIs fails and an open call is pending the on_open_complete callback shall be triggered with IO_OPEN_ERROR.] */
TEST_FUNCTION(when_getting_the_memory_BIO_METHOD_fails_in_LOAD_EXTRA_CLIENT_VERIFY_CERTS_an_on_open_complete_is_triggered_with_error)
{
    // arrange
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    (void)wsio_setoption(wsio, "TrustedCerts", "boohoo");
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    STRICT_EXPECTED_CALL(SSL_CTX_get_cert_store(TEST_SSL_CONTEXT));
    STRICT_EXPECTED_CALL(BIO_s_mem())
        .SetReturn((BIO_METHOD*)NULL);
    STRICT_EXPECTED_CALL(test_on_io_open_complete((void*)0x4242, IO_OPEN_ERROR));

    // act
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_OPENSSL_LOAD_EXTRA_CLIENT_VERIFY_CERTS, (void*)TEST_SSL_CONTEXT, NULL, 0);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_129: [If any of the APIs fails and an open call is pending the on_open_complete callback shall be triggered with IO_OPEN_ERROR.] */
TEST_FUNCTION(when_creating_the_BIO_fails_in_LOAD_EXTRA_CLIENT_VERIFY_CERTS_an_on_open_complete_is_triggered_with_error)
{
    // arrange
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    (void)wsio_setoption(wsio, "TrustedCerts", "boohoo");
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    STRICT_EXPECTED_CALL(SSL_CTX_get_cert_store(TEST_SSL_CONTEXT));
    STRICT_EXPECTED_CALL(BIO_s_mem());
    STRICT_EXPECTED_CALL(BIO_new(TEST_BIO_METHOD))
        .SetReturn((BIO*)NULL);
    STRICT_EXPECTED_CALL(test_on_io_open_complete((void*)0x4242, IO_OPEN_ERROR));

    // act
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_OPENSSL_LOAD_EXTRA_CLIENT_VERIFY_CERTS, (void*)TEST_SSL_CONTEXT, NULL, 0);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_129: [If any of the APIs fails and an open call is pending the on_open_complete callback shall be triggered with IO_OPEN_ERROR.] */
TEST_FUNCTION(when_setting_the_input_string_for_the_memory_BIO_fails_with_0_in_LOAD_EXTRA_CLIENT_VERIFY_CERTS_an_on_open_complete_is_triggered_with_error)
{
    // arrange
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    (void)wsio_setoption(wsio, "TrustedCerts", "boohoo");
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    STRICT_EXPECTED_CALL(SSL_CTX_get_cert_store(TEST_SSL_CONTEXT));
    STRICT_EXPECTED_CALL(BIO_s_mem());
    STRICT_EXPECTED_CALL(BIO_new(TEST_BIO_METHOD));
    STRICT_EXPECTED_CALL(BIO_puts(TEST_BIO, "boohoo"))
        .SetReturn(0);
    STRICT_EXPECTED_CALL(BIO_free(TEST_BIO));
    STRICT_EXPECTED_CALL(test_on_io_open_complete((void*)0x4242, IO_OPEN_ERROR));

    // act
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_OPENSSL_LOAD_EXTRA_CLIENT_VERIFY_CERTS, (void*)TEST_SSL_CONTEXT, NULL, 0);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_129: [If any of the APIs fails and an open call is pending the on_open_complete callback shall be triggered with IO_OPEN_ERROR.] */
TEST_FUNCTION(when_setting_the_input_string_for_the_memory_BIO_fails_with_len_minus_1_in_LOAD_EXTRA_CLIENT_VERIFY_CERTS_an_on_open_complete_is_triggered_with_error)
{
    // arrange
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    (void)wsio_setoption(wsio, "TrustedCerts", "boohoo");
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    STRICT_EXPECTED_CALL(SSL_CTX_get_cert_store(TEST_SSL_CONTEXT));
    STRICT_EXPECTED_CALL(BIO_s_mem());
    STRICT_EXPECTED_CALL(BIO_new(TEST_BIO_METHOD));
    STRICT_EXPECTED_CALL(BIO_puts(TEST_BIO, "boohoo"))
        .SetReturn((int)strlen("boohoo") - 1);
    STRICT_EXPECTED_CALL(BIO_free(TEST_BIO));
    STRICT_EXPECTED_CALL(test_on_io_open_complete((void*)0x4242, IO_OPEN_ERROR));

    // act
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_OPENSSL_LOAD_EXTRA_CLIENT_VERIFY_CERTS, (void*)TEST_SSL_CONTEXT, NULL, 0);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_129: [If any of the APIs fails and an open call is pending the on_open_complete callback shall be triggered with IO_OPEN_ERROR.] */
TEST_FUNCTION(when_setting_the_input_string_for_the_memory_BIO_fails_with_len_plus_1_in_LOAD_EXTRA_CLIENT_VERIFY_CERTS_an_on_open_complete_is_triggered_with_error)
{
    // arrange
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    (void)wsio_setoption(wsio, "TrustedCerts", "boohoo");
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    STRICT_EXPECTED_CALL(SSL_CTX_get_cert_store(TEST_SSL_CONTEXT));
    STRICT_EXPECTED_CALL(BIO_s_mem());
    STRICT_EXPECTED_CALL(BIO_new(TEST_BIO_METHOD));
    STRICT_EXPECTED_CALL(BIO_puts(TEST_BIO, "boohoo"))
        .SetReturn((int)strlen("boohoo") + 1);
    STRICT_EXPECTED_CALL(BIO_free(TEST_BIO));
    STRICT_EXPECTED_CALL(test_on_io_open_complete((void*)0x4242, IO_OPEN_ERROR));

    // act
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_OPENSSL_LOAD_EXTRA_CLIENT_VERIFY_CERTS, (void*)TEST_SSL_CONTEXT, NULL, 0);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_129: [If any of the APIs fails and an open call is pending the on_open_complete callback shall be triggered with IO_OPEN_ERROR.] */
/* Tests_SRS_WSIO_01_133: [If X509_STORE_add_cert fails then the certificate obtained by calling PEM_read_bio_X509 shall be freed with X509_free.] */
TEST_FUNCTION(when_adding_the_cert_to_the_store_fails_in_LOAD_EXTRA_CLIENT_VERIFY_CERTS_the_cert_is_freed_and_on_open_complete_is_triggered_with_error)
{
    // arrange
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);
    (void)wsio_setoption(wsio, "TrustedCerts", "boohoo");
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(lws_get_context(TEST_LIBWEBSOCKET));
    STRICT_EXPECTED_CALL(lws_context_user(TEST_LIBWEBSOCKET_CONTEXT))
        .SetReturn(saved_ws_callback_context);
    STRICT_EXPECTED_CALL(SSL_CTX_get_cert_store(TEST_SSL_CONTEXT));
    STRICT_EXPECTED_CALL(BIO_s_mem());
    STRICT_EXPECTED_CALL(BIO_new(TEST_BIO_METHOD));
    STRICT_EXPECTED_CALL(BIO_puts(TEST_BIO, "boohoo"))
        .SetReturn((int)strlen("boohoo"));
    STRICT_EXPECTED_CALL(PEM_read_bio_X509(TEST_BIO, NULL, NULL, NULL))
        .SetReturn(TEST_X509_CERT_1);
    STRICT_EXPECTED_CALL(X509_STORE_add_cert(TEST_CERT_STORE, TEST_X509_CERT_1))
        .SetReturn(0);
    STRICT_EXPECTED_CALL(X509_free(TEST_X509_CERT_1));
    STRICT_EXPECTED_CALL(BIO_free(TEST_BIO));
    STRICT_EXPECTED_CALL(test_on_io_open_complete((void*)0x4242, IO_OPEN_ERROR));

    // act
    (void)saved_ws_callback(TEST_LIBWEBSOCKET, LWS_CALLBACK_OPENSSL_LOAD_EXTRA_CLIENT_VERIFY_CERTS, (void*)TEST_SSL_CONTEXT, NULL, 0);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_destroy(wsio);
}

/* wsio_setoption */

/* Tests_SRS_WSIO_01_136: [ If any of the arguments ws_io or option_name is NULL wsio_setoption shall return a non-zero value. ] ]*/
TEST_FUNCTION(wsio_setoption_with_NULL_io_handle_fails)
{
    //arrange
    HTTP_PROXY_OPTIONS proxy_options;
    proxy_options.host_address = NULL;
    proxy_options.port = 8080;
    proxy_options.username = NULL;
    proxy_options.password = NULL;

    // act
    int option_result = wsio_setoption(NULL, OPTION_HTTP_PROXY, &proxy_options);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, option_result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_WSIO_01_136: [ If any of the arguments ws_io or option_name is NULL wsio_setoption shall return a non-zero value. ] ]*/
TEST_FUNCTION(wsio_setoption_with_NULL_option_name_fails)
{
    //arrange
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    HTTP_PROXY_OPTIONS proxy_options;
    proxy_options.host_address = NULL;
    proxy_options.port = 8080;
    proxy_options.username = NULL;
    proxy_options.password = NULL;
    umock_c_reset_all_calls();

    // act
    int option_result = wsio_setoption(wsio, NULL, &proxy_options);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, option_result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_137: [ If the option_name argument indicates an option that is not handled by wsio, then wsio_setoption shall return a non-zero value. ]*/
TEST_FUNCTION(wsio_setoption_optionName_invalid_fails)
{
    //arrange
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    umock_c_reset_all_calls();

    HTTP_PROXY_OPTIONS proxy_options;
    proxy_options.host_address = NULL;
    proxy_options.port = 8080;
    proxy_options.username = NULL;
    proxy_options.password = NULL;

    // act
    int option_result = wsio_setoption(wsio, "Invalid_options", &proxy_options);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, option_result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_134: [ - "TrustedCerts" - a char\* that shall be saved by wsio as it shall be used to validate the server cert in the LWS_CALLBACK_OPENSSL_LOAD_EXTRA_CLIENT_VERIFY_CERTS callback. ]*/
/* Tests_SRS_WSIO_01_138: [ If the option was handled by wsio, then wsio_setoption shall return 0. ]*/
TEST_FUNCTION(wsio_setoption_with_trustedcerts_succeeds)
{
    //arrange
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_NUM_ARG, "pupu"))
        .IgnoreArgument_destination();

    // act
    int option_result = wsio_setoption(wsio, "TrustedCerts", "pupu");

    // assert
    ASSERT_ARE_EQUAL(int, 0, option_result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_139: [ If a previous TrustedCerts option was saved, then the previous value shall be freed. ]*/
TEST_FUNCTION(wsio_setoption_with_a_previous_trustedcerts_frees_the_previous_string)
{
    //arrange
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_setoption(wsio, "TrustedCerts", "boohoo");
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_free(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_NUM_ARG, "pupu"))
        .IgnoreArgument_destination();

    // act
    int option_result = wsio_setoption(wsio, "TrustedCerts", "pupu");

    // assert
    ASSERT_ARE_EQUAL(int, 0, option_result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_148: [ A NULL value shall be allowed for TrustedCerts, in which case the previously stored TrustedCerts option value shall be cleared. ]*/
TEST_FUNCTION(wsio_setoption_with_trustedcerts_and_NULL_value_clears_the_TrustedCerts_option)
{
    //arrange
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_setoption(wsio, "TrustedCerts", "boohoo");
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_free(IGNORED_NUM_ARG));

    // act
    int option_result = wsio_setoption(wsio, "TrustedCerts", NULL);

    // assert
    ASSERT_ARE_EQUAL(int, 0, option_result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_149: [  - "proxy_data" - a HTTP_PROXY_OPTIONS structure that defines the HTTP proxy to be used. ]*/
/* Tests_SRS_WSIO_01_138: [ If the option was handled by wsio, then wsio_setoption shall return 0. ]*/
/* Tests_SRS_WSIO_01_163: [ The fields hostname, username and password shall be copied for later use by using mallocAndStrcpy_s. ]*/
TEST_FUNCTION(wsio_setoption_with_proxy_data_succeeds)
{
    //arrange
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    umock_c_reset_all_calls();

    HTTP_PROXY_OPTIONS proxy_options;
    proxy_options.host_address = TEST_HOST_ADDRESS;
    proxy_options.port = 8080;
    proxy_options.username = TEST_USERNAME;
    proxy_options.password = TEST_PASSWORD;

    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, TEST_HOST_ADDRESS))
        .IgnoreArgument_destination();
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, TEST_USERNAME))
        .IgnoreArgument_destination();
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, TEST_PASSWORD))
        .IgnoreArgument_destination();

    // act
    int option_result = wsio_setoption(wsio, OPTION_HTTP_PROXY, &proxy_options);

    // assert
    ASSERT_ARE_EQUAL(int, 0, option_result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_160: [ If the hostname field is NULL then wsio_setoption shall return a non-zero value. ]*/
TEST_FUNCTION(wsio_setoption_with_proxy_data_with_NULL_hostname_fails)
{
    //arrange
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    umock_c_reset_all_calls();

    HTTP_PROXY_OPTIONS proxy_options;
    proxy_options.host_address = NULL;
    proxy_options.port = 8080;
    proxy_options.username = TEST_USERNAME;
    proxy_options.password = TEST_PASSWORD;

    // act
    int option_result = wsio_setoption(wsio, OPTION_HTTP_PROXY, &proxy_options);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, option_result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_150: [ The username and password fields are optional (can be NULL). ]*/
TEST_FUNCTION(wsio_setoption_no_username_password_succeeds)
{
    //arrange
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    umock_c_reset_all_calls();

    HTTP_PROXY_OPTIONS proxy_options;
    proxy_options.host_address = TEST_HOST_ADDRESS;
    proxy_options.port = 8080;
    proxy_options.username = NULL;
    proxy_options.password = NULL;

    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, TEST_HOST_ADDRESS))
        .IgnoreArgument_destination();

    // act
    int option_result = wsio_setoption(wsio, OPTION_HTTP_PROXY, &proxy_options);

    // assert
    ASSERT_ARE_EQUAL(int, 0, option_result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_150: [ The username and password fields are optional (can be NULL). ]*/
TEST_FUNCTION(wsio_setoption_username_NULL_password_Valid_succeed)
{
    //arrange
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    umock_c_reset_all_calls();

    HTTP_PROXY_OPTIONS proxy_options;
    proxy_options.host_address = TEST_HOST_ADDRESS;
    proxy_options.port = 8080;
    proxy_options.username = NULL;
    proxy_options.password = TEST_PASSWORD;

    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, TEST_HOST_ADDRESS))
        .IgnoreArgument_destination();
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, TEST_PASSWORD))
        .IgnoreArgument_destination();

    // act
    int option_result = wsio_setoption(wsio, OPTION_HTTP_PROXY, &proxy_options);

    // assert
    ASSERT_ARE_EQUAL(int, 0, option_result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_159: [ If a username has been specified then a password shall also be specified. ]*/
TEST_FUNCTION(wsio_setoption_username_valid_password_NULL_fail)
{
    //arrange
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    umock_c_reset_all_calls();

    HTTP_PROXY_OPTIONS proxy_options;
    proxy_options.host_address = TEST_HOST_ADDRESS;
    proxy_options.port = 8080;
    proxy_options.username = TEST_USERNAME;
    proxy_options.password = NULL;

    // act
    int option_result = wsio_setoption(wsio, OPTION_HTTP_PROXY, &proxy_options);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, option_result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_151: [ If copying of any of the fields host_address, username or password fails then wsio_setoption shall return a non-zero value. ]*/
TEST_FUNCTION(when_copying_the_hostname_fails_wsio_setoption_with_proxy_data_fails)
{
    //arrange
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    umock_c_reset_all_calls();

    HTTP_PROXY_OPTIONS proxy_options;
    proxy_options.host_address = TEST_HOST_ADDRESS;
    proxy_options.port = 8080;
    proxy_options.username = TEST_USERNAME;
    proxy_options.password = TEST_PASSWORD;

    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, TEST_HOST_ADDRESS))
        .IgnoreArgument_destination()
        .SetReturn(1);

    // act
    int option_result = wsio_setoption(wsio, OPTION_HTTP_PROXY, &proxy_options);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, option_result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_151: [ If copying of any of the fields host_address, username or password fails then wsio_setoption shall return a non-zero value. ]*/
TEST_FUNCTION(when_copying_the_username_fails_wsio_setoption_with_proxy_data_fails)
{
    //arrange
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    umock_c_reset_all_calls();

    HTTP_PROXY_OPTIONS proxy_options;
    proxy_options.host_address = TEST_HOST_ADDRESS;
    proxy_options.port = 8080;
    proxy_options.username = TEST_USERNAME;
    proxy_options.password = TEST_PASSWORD;

    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, TEST_HOST_ADDRESS))
        .IgnoreArgument_destination();
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, TEST_USERNAME))
        .IgnoreArgument_destination()
        .SetReturn(1);

    // act
    int option_result = wsio_setoption(wsio, OPTION_HTTP_PROXY, &proxy_options);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, option_result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_151: [ If copying of any of the fields host_address, username or password fails then wsio_setoption shall return a non-zero value. ]*/
TEST_FUNCTION(when_copying_the_password_fails_wsio_setoption_with_proxy_data_fails)
{
    //arrange
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    umock_c_reset_all_calls();

    HTTP_PROXY_OPTIONS proxy_options;
    proxy_options.host_address = TEST_HOST_ADDRESS;
    proxy_options.port = 8080;
    proxy_options.username = TEST_USERNAME;
    proxy_options.password = TEST_PASSWORD;

    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, TEST_HOST_ADDRESS))
        .IgnoreArgument_destination();
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, TEST_USERNAME))
        .IgnoreArgument_destination();
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, TEST_PASSWORD))
        .IgnoreArgument_destination()
        .SetReturn(1);

    // act
    int option_result = wsio_setoption(wsio, OPTION_HTTP_PROXY, &proxy_options);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, option_result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_161: [ If a previous proxy_data option was saved, then the previous value shall be freed. ]*/
TEST_FUNCTION(the_http_proxy_data_set_via_a_previous_wsio_setoption_shall_be_free_upon_a_new_setoption)
{
    //arrange
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);

    HTTP_PROXY_OPTIONS proxy_options;
    proxy_options.host_address = TEST_HOST_ADDRESS;
    proxy_options.port = 8080;
    proxy_options.username = TEST_USERNAME;
    proxy_options.password = TEST_PASSWORD;

    (void)wsio_setoption(wsio, OPTION_HTTP_PROXY, &proxy_options);
    umock_c_reset_all_calls();

    proxy_options.host_address = TEST_HOST_ADDRESS;
    proxy_options.port = 8080;
    proxy_options.username = NULL;
    proxy_options.password = NULL;

    EXPECTED_CALL(gballoc_free(IGNORED_NUM_ARG));
    EXPECTED_CALL(gballoc_free(IGNORED_NUM_ARG));
    EXPECTED_CALL(gballoc_free(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, TEST_HOST_ADDRESS))
        .IgnoreArgument_destination();

    // act
    int option_result = wsio_setoption(wsio, OPTION_HTTP_PROXY, &proxy_options);

    // assert
    ASSERT_ARE_EQUAL(int, 0, option_result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_162: [ A NULL value shall be allowed for proxy_data, in which case the previously stored proxy_data option value shall be cleared. ]*/
TEST_FUNCTION(wsio_setoption_with_http_proxy_data_with_NULL_clears_previous_proxy_data)
{
    //arrange
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);

    HTTP_PROXY_OPTIONS proxy_options;
    proxy_options.host_address = TEST_HOST_ADDRESS;
    proxy_options.port = 8080;
    proxy_options.username = TEST_USERNAME;
    proxy_options.password = TEST_PASSWORD;

    (void)wsio_setoption(wsio, OPTION_HTTP_PROXY, &proxy_options);
    umock_c_reset_all_calls();

    proxy_options.host_address = TEST_HOST_ADDRESS;
    proxy_options.port = 8080;
    proxy_options.username = NULL;
    proxy_options.password = NULL;

    EXPECTED_CALL(gballoc_free(IGNORED_NUM_ARG));
    EXPECTED_CALL(gballoc_free(IGNORED_NUM_ARG));
    EXPECTED_CALL(gballoc_free(IGNORED_NUM_ARG));

    // act
    int option_result = wsio_setoption(wsio, OPTION_HTTP_PROXY, NULL);

    // assert
    ASSERT_ARE_EQUAL(int, 0, option_result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_destroy(wsio);
}

/* wsio_clone_option */

/* Tests_SRS_WSIO_01_140: [ If the name or value arguments are NULL, wsio_clone_option shall return NULL. ]*/
TEST_FUNCTION(wsio_cloneoption_name_NULL_fail)
{
    //arrange
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);

    umock_c_reset_all_calls();

    HTTP_PROXY_OPTIONS proxy_options;
    proxy_options.host_address = TEST_HOST_ADDRESS;
    proxy_options.port = 8080;
    proxy_options.username = NULL;
    proxy_options.password = TEST_PASSWORD;

    // act
    void* option_result = wsio_clone_option(NULL, &proxy_options);

    // assert
    ASSERT_IS_NULL(option_result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_140: [ If the name or value arguments are NULL, wsio_clone_option shall return NULL. ]*/
TEST_FUNCTION(wsio_cloneoption_value_NULL_fail)
{
    //arrange
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);

    umock_c_reset_all_calls();

    // act
    void* option_result = wsio_clone_option(OPTION_HTTP_PROXY, NULL);

    // assert
    ASSERT_IS_NULL(option_result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_destroy_option(OPTION_HTTP_PROXY, option_result);
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_141: [ wsio_clone_option shall clone the option named `TrustedCerts` by calling mallocAndStrcpy_s. ]*/
/* Tests_SRS_WSIO_01_143: [ On success it shall return a non-NULL pointer to the cloned option. ]*/
TEST_FUNCTION(wsio_cloneoption_clones_TrustedCerts)
{
    //arrange
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_setoption(wsio, "TrustedCerts", "boohoo");
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_NUM_ARG, "boohoo"))
        .IgnoreArgument_destination();

    // act
    void* option_result = wsio_clone_option("TrustedCerts", "boohoo");

    // assert
    ASSERT_IS_NOT_NULL(option_result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_destroy_option("TrustedCerts", option_result);
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_142: [ If mallocAndStrcpy_s for `TrustedCerts` fails, wsio_clone_option shall return NULL. ]*/
TEST_FUNCTION(when_cloning_the_TrustedCerts_option_fails_then_wsio_cloneoption_fails)
{
    //arrange
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    (void)wsio_setoption(wsio, "TrustedCerts", "boohoo");
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_NUM_ARG, "boohoo"))
        .IgnoreArgument_destination()
        .SetReturn(1);

    // act
    void* option_result = wsio_clone_option("TrustedCerts", "boohoo");

    // assert
    ASSERT_IS_NULL(option_result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_152: [ wsio_clone_option shall clone the option named `proxy_data` by allocating a new HTTP_PROXY_OPTIONS structure. ]*/
/* Tests_SRS_WSIO_01_154: [ Then each of the fields host_address, username and password shall be cloned by using mallocAndStrcpy_s. ]*/
TEST_FUNCTION(wsio_cloneoption_clones_proxy_data)
{
    //arrange
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    umock_c_reset_all_calls();

    HTTP_PROXY_OPTIONS proxy_options;
    proxy_options.host_address = TEST_HOST_ADDRESS;
    proxy_options.port = 8080;
    proxy_options.username = TEST_USERNAME;
    proxy_options.password = TEST_PASSWORD;

    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, TEST_HOST_ADDRESS))
        .IgnoreArgument_destination();
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, TEST_USERNAME))
        .IgnoreArgument_destination();
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, TEST_PASSWORD))
        .IgnoreArgument_destination();

    // act
    void* option_result = wsio_clone_option("proxy_data", &proxy_options);

    // assert
    ASSERT_IS_NOT_NULL(option_result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_destroy_option("proxy_data", option_result);
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_152: [ wsio_clone_option shall clone the option named `proxy_data` by allocating a new HTTP_PROXY_OPTIONS structure. ]*/
/* Tests_SRS_WSIO_01_154: [ Then each of the fields host_address, username and password shall be cloned by using mallocAndStrcpy_s. ]*/
/* Tests_SRS_WSIO_01_165: [ If the field username in the structure pointed to by value is NULL nothing shall be copied to the cloned option. ]*/
/* Tests_SRS_WSIO_01_166: [ If the field password in the structure pointed to by value is NULL nothing shall be copied to the cloned option. ]*/
TEST_FUNCTION(wsio_cloneoption_with_proxy_data_without_username_and_password_succeeds)
{
    //arrange
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    umock_c_reset_all_calls();

    HTTP_PROXY_OPTIONS proxy_options;
    proxy_options.host_address = TEST_HOST_ADDRESS;
    proxy_options.port = 8080;
    proxy_options.username = NULL;
    proxy_options.password = NULL;

    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, TEST_HOST_ADDRESS))
        .IgnoreArgument_destination();

    // act
    void* option_result = wsio_clone_option("proxy_data", &proxy_options);

    // assert
    ASSERT_IS_NOT_NULL(option_result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_destroy_option("proxy_data", option_result);
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_153: [ If allocating memory for the structure fails fails, wsio_clone_option shall return NULL. ]*/
TEST_FUNCTION(when_allocatng_memory_for_proxy_data_fails_then_wsio_cloneoption_with_proxy_data_fails)
{
    //arrange
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    umock_c_reset_all_calls();

    HTTP_PROXY_OPTIONS proxy_options;
    proxy_options.host_address = TEST_HOST_ADDRESS;
    proxy_options.port = 8080;
    proxy_options.username = TEST_USERNAME;
    proxy_options.password = TEST_PASSWORD;

    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG))
        .SetReturn(NULL);

    // act
    void* option_result = wsio_clone_option("proxy_data", &proxy_options);

    // assert
    ASSERT_IS_NULL(option_result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_155: [ If mallocAndStrcpy_s fails, wsio_clone_option shall return NULL. ]*/
TEST_FUNCTION(when_copying_the_host_address_fails_then_wsio_cloneoption_with_proxy_data_fails)
{
    //arrange
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    umock_c_reset_all_calls();

    HTTP_PROXY_OPTIONS proxy_options;
    proxy_options.host_address = TEST_HOST_ADDRESS;
    proxy_options.port = 8080;
    proxy_options.username = TEST_USERNAME;
    proxy_options.password = TEST_PASSWORD;

    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, TEST_HOST_ADDRESS))
        .IgnoreArgument_destination()
        .SetReturn(1);
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    void* option_result = wsio_clone_option("proxy_data", &proxy_options);

    // assert
    ASSERT_IS_NULL(option_result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_155: [ If mallocAndStrcpy_s fails, wsio_clone_option shall return NULL. ]*/
TEST_FUNCTION(when_copying_the_username_fails_then_wsio_cloneoption_with_proxy_data_fails)
{
    //arrange
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    umock_c_reset_all_calls();

    HTTP_PROXY_OPTIONS proxy_options;
    proxy_options.host_address = TEST_HOST_ADDRESS;
    proxy_options.port = 8080;
    proxy_options.username = TEST_USERNAME;
    proxy_options.password = TEST_PASSWORD;

    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, TEST_HOST_ADDRESS))
        .IgnoreArgument_destination();
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, TEST_USERNAME))
        .IgnoreArgument_destination()
        .SetReturn(1);
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    void* option_result = wsio_clone_option("proxy_data", &proxy_options);

    // assert
    ASSERT_IS_NULL(option_result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_155: [ If mallocAndStrcpy_s fails, wsio_clone_option shall return NULL. ]*/
TEST_FUNCTION(when_copying_the_password_fails_then_wsio_cloneoption_with_proxy_data_fails)
{
    //arrange
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    umock_c_reset_all_calls();

    HTTP_PROXY_OPTIONS proxy_options;
    proxy_options.host_address = TEST_HOST_ADDRESS;
    proxy_options.port = 8080;
    proxy_options.username = TEST_USERNAME;
    proxy_options.password = TEST_PASSWORD;

    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, TEST_HOST_ADDRESS))
        .IgnoreArgument_destination();
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, TEST_USERNAME))
        .IgnoreArgument_destination();
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, TEST_PASSWORD))
        .IgnoreArgument_destination()
        .SetReturn(1);
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    void* option_result = wsio_clone_option("proxy_data", &proxy_options);

    // assert
    ASSERT_IS_NULL(option_result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_destroy(wsio);
}

/* wsio_destroy_option */

/* Tests_SRS_WSIO_01_147: [ If any of the arguments is NULL, wsio_destroy_option shall do nothing. ]*/
TEST_FUNCTION(wsio_destroy_option_name_NULL_fail)
{
    //arrange
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);

    umock_c_reset_all_calls();

    // act
    int port = 8080;
    wsio_destroy_option(NULL, &port);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_147: [ If any of the arguments is NULL, wsio_destroy_option shall do nothing. ]*/
TEST_FUNCTION(wsio_destroy_option_value_NULL_fail)
{
    //arrange
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);

    umock_c_reset_all_calls();

    // act
    wsio_destroy_option(OPTION_HTTP_PROXY, NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_144: [ If the option name is `TrustedCerts`, wsio_destroy_option shall free the char\* option indicated by value. ]*/
TEST_FUNCTION(wsio_destroy_option_with_TrustedCerts_frees_the_string)
{
    //arrange
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    void* option_result = wsio_clone_option("TrustedCerts", "boohoo");
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    wsio_destroy_option("TrustedCerts", option_result);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_157: [ If the option name is `proxy_data`, wsio_destroy_option shall free the strings for the fields host_address, username and password. ]*/
/* Tests_SRS_WSIO_01_156: [ Also the memory for the HTTP_PROXY_OPTIONS shall be freed. ]*/
TEST_FUNCTION(wsio_destroy_option_with_proxy_data_frees_the_strings)
{
    //arrange
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);

    HTTP_PROXY_OPTIONS proxy_options;
    proxy_options.host_address = TEST_HOST_ADDRESS;
    proxy_options.port = 8080;
    proxy_options.username = TEST_USERNAME;
    proxy_options.password = TEST_PASSWORD;

    void* option_result = wsio_clone_option("proxy_data", &proxy_options);
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    wsio_destroy_option("proxy_data", option_result);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_01_157: [ If the option name is `proxy_data`, wsio_destroy_option shall free the strings for the fields host_address, username and password. ]*/
/* Tests_SRS_WSIO_01_156: [ Also the memory for the HTTP_PROXY_OPTIONS shall be freed. ]*/
/* Tests_SRS_WSIO_01_167: [ No free shal be done for a NULL username. ]*/
/* Tests_SRS_WSIO_01_168: [ No free shal be done for a NULL password. ]*/
TEST_FUNCTION(wsio_destroy_option_with_proxy_data_with_NULL_username_frees_the_hostname_and_structure)
{
    //arrange
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);

    HTTP_PROXY_OPTIONS proxy_options;
    proxy_options.host_address = TEST_HOST_ADDRESS;
    proxy_options.port = 8080;
    proxy_options.username = NULL;
    proxy_options.password = NULL;

    void* option_result = wsio_clone_option("proxy_data", &proxy_options);
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    wsio_destroy_option("proxy_data", option_result);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_destroy(wsio);
}

/* wsio_retrieveoptions */

/*Tests_SRS_WSIO_02_001: [ If parameter handle is NULL then wsio_retrieveoptions shall fail and return NULL. */
TEST_FUNCTION(wsio_retrieveoptions_with_NULL_handle_returns_NULL)
{
    //arrange
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);

    umock_c_reset_all_calls();

    // act
    OPTIONHANDLER_HANDLE handle = wsio_retrieveoptions(NULL);

    // assert
    ASSERT_IS_NULL(handle);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_destroy(wsio);
}

/*Tests_SRS_WSIO_02_002: [** `wsio_retrieveoptions` shall produce an OPTIONHANDLER_HANDLE. ]*/
TEST_FUNCTION(wsio_retrieveoptions_returns_non_NULL_empty)
{
    ///arrange
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(OptionHandler_Create(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreAllArguments();

    ///act
    OPTIONHANDLER_HANDLE h = wsio_get_interface_description()->concrete_io_retrieveoptions(wsio);

    ///assert
    ASSERT_IS_NOT_NULL(h);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    ///cleanup
    wsio_destroy(wsio);
    free(h);
}

/*Tests_SRS_WSIO_02_003: [ If producing the OPTIONHANDLER_HANDLE fails then wsio_retrieveoptions shall fail and return NULL. ]*/
TEST_FUNCTION(wsio_retrieveoptions_when_OptionHandler_Create_fails_it_fails)
{
    ///arrange
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(OptionHandler_Create(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreAllArguments()
        .SetReturn((OPTIONHANDLER_HANDLE)NULL);

    ///act
    OPTIONHANDLER_HANDLE h = wsio_get_interface_description()->concrete_io_retrieveoptions(wsio);

    ///assert
    ASSERT_IS_NULL(h);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    ///cleanup
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_02_002: [ `wsio_retrieveoptions` shall produce an OPTIONHANDLER_HANDLE. ]*/
/* Tests_SRS_WSIO_01_145: [ `wsio_retrieveoptions` shall add to it the options: ]*/
/* Tests_SRS_WSIO_01_158: [ - proxy_data ]*/
TEST_FUNCTION(wsio_retrieveoptions_returns_previously_set_proxy_options)
{
    //arrange
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    OPTIONHANDLER_HANDLE created_option_handle;

    HTTP_PROXY_OPTIONS proxy_options;
    proxy_options.host_address = TEST_HOST_ADDRESS;
    proxy_options.port = 8080;
    proxy_options.username = TEST_USERNAME;
    proxy_options.password = TEST_PASSWORD;

    (void)wsio_setoption(wsio, OPTION_HTTP_PROXY, &proxy_options);

    umock_c_reset_all_calls();

    EXPECTED_CALL(OptionHandler_Create(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .CaptureReturn(&created_option_handle);
    STRICT_EXPECTED_CALL(OptionHandler_AddOption(IGNORED_PTR_ARG, "proxy_data", IGNORED_PTR_ARG))
        .ValidateArgumentValue_handle(&created_option_handle)
        .IgnoreArgument_value();

    // act
    OPTIONHANDLER_HANDLE handle = wsio_retrieveoptions(wsio);

    // assert
    ASSERT_IS_NOT_NULL(handle);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    OptionHandler_Destroy(handle);
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_02_002: [ `wsio_retrieveoptions` shall produce an OPTIONHANDLER_HANDLE. ]*/
/* Tests_SRS_WSIO_01_145: [ `wsio_retrieveoptions` shall add to it the options: ]*/
/* Tests_SRS_WSIO_01_146: [ - TrustedCerts ]*/
TEST_FUNCTION(wsio_retrieveoptions_returns_previously_set_trusted_certs_option)
{
    //arrange
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    OPTIONHANDLER_HANDLE optionhandler_handle;
    (void)wsio_setoption(wsio, "TrustedCerts", "boohoo");

    umock_c_reset_all_calls();

    EXPECTED_CALL(OptionHandler_Create(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .CaptureReturn(&optionhandler_handle);
    STRICT_EXPECTED_CALL(OptionHandler_AddOption(IGNORED_PTR_ARG, "TrustedCerts", IGNORED_PTR_ARG))
        .ValidateArgumentValue_handle(&optionhandler_handle)
        .ValidateArgumentBuffer(3, "boohoo", sizeof("boohoo"));

    // act
    OPTIONHANDLER_HANDLE handle = wsio_retrieveoptions(wsio);

    // assert
    ASSERT_IS_NOT_NULL(handle);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    OptionHandler_Destroy(handle);
    wsio_destroy(wsio);
}

/* Tests_SRS_WSIO_02_003: [ If producing the OPTIONHANDLER_HANDLE fails then wsio_retrieveoptions shall fail and return NULL. ]*/
TEST_FUNCTION(when_AddOption_fails_wsio_retrieveoptions_fails)
{
    //arrange
    CONCRETE_IO_HANDLE wsio = wsio_create(&default_wsio_config);
    OPTIONHANDLER_HANDLE optionhandler_handle;
    (void)wsio_setoption(wsio, "TrustedCerts", "boohoo");

    umock_c_reset_all_calls();

    EXPECTED_CALL(OptionHandler_Create(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .CaptureReturn(&optionhandler_handle);
    STRICT_EXPECTED_CALL(OptionHandler_AddOption(IGNORED_PTR_ARG, "TrustedCerts", IGNORED_PTR_ARG))
        .IgnoreArgument_handle().IgnoreArgument_value()
        .SetReturn(OPTIONHANDLER_ERROR);
    STRICT_EXPECTED_CALL(OptionHandler_Destroy(IGNORED_PTR_ARG))
        .ValidateArgumentValue_handle(&optionhandler_handle);

    // act
    OPTIONHANDLER_HANDLE handle = wsio_retrieveoptions(wsio);

    // assert
    ASSERT_IS_NULL(handle);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_destroy(wsio);
}

END_TEST_SUITE(wsio_ut)
