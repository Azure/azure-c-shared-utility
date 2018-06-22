// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

// This file is made an integral part of xio_adapter_ut.c with a #include. It
// is broken out for readability. 

#ifndef FAKE_ADAPTER_H
#define FAKE_ADAPTER_H

typedef struct XIO_ADAPTER_INSTANCE_TAG
{
    int dummy;
} XIO_ADAPTER_INSTANCE;

static XIO_ADAPTER_INSTANCE adapter_instance = { 0 };

static XIO_ADAPTER_INTERFACE adapter_interface =
{
    xio_adapter_create,
    xio_adapter_destroy,
    xio_adapter_open,
    xio_adapter_close,
    xio_adapter_read,
    xio_adapter_write,
    xio_adapter_setoption,
    xio_adapter_retrieveoptions
};

// 
static uint8_t* SSL_send_buffer = (uint8_t*)"111111112222222233333333";
static size_t SSL_send_message_size = 25;

#define SSL_TEST_MESSAGE_SIZE 64
#define SSL_WRITE_MAX_TEST_SIZE 60
#define SSL_SHORT_SENT_MESSAGE_SIZE 30
#define SSL_FAIL_ME_SENT_MESSAGE_SIZE 1700
#define SSL_SHORT_RECEIVED_MESSAGE_SIZE 15
#define SSL_LONG_RECEIVED_MESSAGE_SIZE 1500

static ON_BYTES_RECEIVED supplied_on_received_callback = NULL;
static void* supplied_on_received_callback_context = NULL;

static XIO_ASYNC_RESULT my_xio_adapter_open(XIO_ADAPTER_INSTANCE_HANDLE xio_adapter_instance,
    ON_BYTES_RECEIVED on_received, void* on_received_context)
{
    (void)xio_adapter_instance;
    supplied_on_received_callback = on_received;
    supplied_on_received_callback_context = on_received_context;
    return XIO_ASYNC_RESULT_SUCCESS;
}

static XIO_ASYNC_RESULT my_xio_adapter_read(XIO_ADAPTER_INSTANCE_HANDLE xio_adapter_instance)
{
    ASSERT_ARE_EQUAL(size_t, (size_t)xio_adapter_instance, (size_t)&adapter_instance);
    supplied_on_received_callback(supplied_on_received_callback_context, SSL_send_buffer, SSL_send_message_size);
    return XIO_ASYNC_RESULT_SUCCESS;
}

static int my_xio_adapter_write(XIO_ADAPTER_INSTANCE_HANDLE xio_adapter_instance, const uint8_t* buffer, uint32_t buffer_size)
{
    int result;
    (void)buffer; // not used

    // Track whether a message is g_message_1 and whether it was copied or not
    last_message_was_g_message_1_content = strcmp((const char*)buffer, g_message_1) == 0;
    last_message_was_g_message_1_pointer = (const char*)buffer == g_message_1;

    // "Send" no more than SSL_WRITE_MAX_TEST_SIZE bytes
    ASSERT_ARE_EQUAL(size_t, (size_t)xio_adapter_instance, (size_t)&adapter_instance);
    if (buffer_size == SSL_FAIL_ME_SENT_MESSAGE_SIZE)
    {
        result = XIO_ASYNC_RESULT_FAILURE;
    }
    else
    {
        if (buffer_size > SSL_WRITE_MAX_TEST_SIZE)
        {
            result = SSL_WRITE_MAX_TEST_SIZE;
        }
        else
        {
            result = (int)buffer_size;
        }
    }
    return result;
}

static void ASSERT_MESSAGE_1_SENT_BY_REFERENCE()
{
    ASSERT_ARE_EQUAL_WITH_MSG(int, (int)true, (int)last_message_was_g_message_1_pointer, "unexpected message pointer value");
}

static void ASSERT_MESSAGE_1_SENT_BY_COPY()
{
    ASSERT_ARE_EQUAL_WITH_MSG(int, (int)true, (int)last_message_was_g_message_1_content, "unexpected message content");
    ASSERT_ARE_EQUAL_WITH_MSG(int, (int)false, (int)last_message_was_g_message_1_pointer, "message was not copied");
}

#endif // FAKE_ADAPTER_H
