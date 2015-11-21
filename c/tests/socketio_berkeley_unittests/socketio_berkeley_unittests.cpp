// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <cstdint>
#include "testrunnerswitcher.h"
#include "micromock.h"
#include "micromockcharstararenullterminatedstrings.h"
#include "socketio.h"
#include "list.h"

#undef DECLSPEC_IMPORT

#include <sys/socket.h>

static const LIST_HANDLE TEST_LIST_HANDLE = (LIST_HANDLE)0x4242;
static const void** list_items = NULL;
static size_t list_item_count = 0;
static SOCKET test_socket = (SOCKET)0x4243;

TYPED_MOCK_CLASS(amqp_frame_codec_mocks, CGlobalMock)
{
public:
	/* list mocks */
	MOCK_STATIC_METHOD_0(, LIST_HANDLE, list_create)
	MOCK_METHOD_END(LIST_HANDLE, TEST_LIST_HANDLE);
	MOCK_STATIC_METHOD_1(, void, list_destroy, LIST_HANDLE, list)
	MOCK_VOID_METHOD_END();
	MOCK_STATIC_METHOD_2(, int, list_remove, LIST_HANDLE, list, LIST_ITEM_HANDLE, item)
	MOCK_METHOD_END(int, 0);
	MOCK_STATIC_METHOD_1(, LIST_ITEM_HANDLE, list_get_head_item, LIST_HANDLE, list)
	MOCK_METHOD_END(LIST_ITEM_HANDLE, (LIST_ITEM_HANDLE)1);
	MOCK_STATIC_METHOD_2(, LIST_ITEM_HANDLE, list_add, LIST_HANDLE, list, const void*, item)
		const void** items = (const void**)realloc(list_items, (list_item_count + 1) * sizeof(item));
		if (items != NULL)
		{
			list_items = items;
			list_items[list_item_count++] = item;
		}
	MOCK_METHOD_END(LIST_ITEM_HANDLE, (LIST_ITEM_HANDLE)list_item_count);
	MOCK_STATIC_METHOD_1(, const void*, list_item_get_value, LIST_ITEM_HANDLE, item_handle)
		MOCK_METHOD_END(const void*, (const void*)item_handle);
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

	/* ws2 mocks */
	MOCK_STATIC_METHOD_3(, SOCKET, socket, int, af, int, type, int, protocol)
	MOCK_METHOD_END(SOCKET, test_socket);
	MOCK_STATIC_METHOD_1(, int, closesocket, SOCKET, s)
	MOCK_METHOD_END(int, 0);
	MOCK_STATIC_METHOD_3(, int, connect, SOCKET, s, const struct sockaddr FAR*, name, int, namelen)
	MOCK_METHOD_END(int, 0);
	MOCK_STATIC_METHOD_4(, int, recv, SOCKET, s, char FAR*, buf, int, len, int, flags)
	MOCK_METHOD_END(int, len);
	MOCK_STATIC_METHOD_4(, int, send, SOCKET, s, const char FAR*, buf, int, len, int, flags)
	MOCK_METHOD_END(int, len);
	MOCK_STATIC_METHOD_4(, INT, getaddrinfo, PCSTR, pNodeName, PCSTR, pServiceName, const ADDRINFOA*, pHints, PADDRINFOA*, ppResult)
	MOCK_METHOD_END(int, 0);
	MOCK_STATIC_METHOD_0(, int, WSAGetLastError)
	MOCK_METHOD_END(int, 0);
	MOCK_STATIC_METHOD_3(, int, ioctlsocket, SOCKET, s, long, cmd, u_long FAR*, argp)
	MOCK_METHOD_END(int, 0);
};

extern "C"
{
	DECLARE_GLOBAL_MOCK_METHOD_0(amqp_frame_codec_mocks, , LIST_HANDLE, list_create);
	DECLARE_GLOBAL_MOCK_METHOD_1(amqp_frame_codec_mocks, , void, list_destroy, LIST_HANDLE, list);
	DECLARE_GLOBAL_MOCK_METHOD_1(amqp_frame_codec_mocks, , LIST_ITEM_HANDLE, list_get_head_item, LIST_HANDLE, list);
	DECLARE_GLOBAL_MOCK_METHOD_2(amqp_frame_codec_mocks, , int, list_remove, LIST_HANDLE, list, LIST_ITEM_HANDLE, item);
	DECLARE_GLOBAL_MOCK_METHOD_2(amqp_frame_codec_mocks, , LIST_ITEM_HANDLE, list_add, LIST_HANDLE, list, const void*, item);
	DECLARE_GLOBAL_MOCK_METHOD_1(amqp_frame_codec_mocks, , const void*, list_item_get_value, LIST_ITEM_HANDLE, item_handle);
	DECLARE_GLOBAL_MOCK_METHOD_3(amqp_frame_codec_mocks, , LIST_ITEM_HANDLE, list_find, LIST_HANDLE, handle, LIST_MATCH_FUNCTION, match_function, const void*, match_context);
	DECLARE_GLOBAL_MOCK_METHOD_3(amqp_frame_codec_mocks, , int, list_remove_matching_item, LIST_HANDLE, handle, LIST_MATCH_FUNCTION, match_function, const void*, match_context);

	DECLARE_GLOBAL_MOCK_METHOD_3(amqp_frame_codec_mocks, , SOCKET WSAAPI, socket, int, af, int, type, int, protocol);
	DECLARE_GLOBAL_MOCK_METHOD_1(amqp_frame_codec_mocks, , int WSAAPI, closesocket, SOCKET, s);
	DECLARE_GLOBAL_MOCK_METHOD_3(amqp_frame_codec_mocks, , int WSAAPI, connect, SOCKET, s, const struct sockaddr FAR*, name, int, namelen);
	DECLARE_GLOBAL_MOCK_METHOD_4(amqp_frame_codec_mocks, , int WSAAPI, recv, SOCKET, s, char FAR*, buf, int, len, int, flags);
	DECLARE_GLOBAL_MOCK_METHOD_4(amqp_frame_codec_mocks, , int WSAAPI, send, SOCKET, s, const char FAR*, buf, int, len, int, flags);
	DECLARE_GLOBAL_MOCK_METHOD_4(amqp_frame_codec_mocks, , INT WSAAPI, getaddrinfo, PCSTR, pNodeName, PCSTR, pServiceName, const ADDRINFOA*, pHints, PADDRINFOA*, ppResult);
	DECLARE_GLOBAL_MOCK_METHOD_0(amqp_frame_codec_mocks, , int WSAAPI, WSAGetLastError);
	DECLARE_GLOBAL_MOCK_METHOD_3(amqp_frame_codec_mocks, , int WSAAPI, ioctlsocket, SOCKET, s, long, cmd, u_long FAR*, argp)
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
}

TEST_FUNCTION_CLEANUP(method_cleanup)
{
	if (!MicroMockReleaseMutex(test_serialize_mutex))
	{
		ASSERT_FAIL("Could not release test serialization mutex.");
	}
}

/* socketio_win32_create */

END_TEST_SUITE(socketio_win32_unittests)
