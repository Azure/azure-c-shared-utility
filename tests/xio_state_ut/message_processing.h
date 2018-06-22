// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

// This file is made an integral part of xio_filter_ut.c with a #include. It
// is broken out for readability. 

#ifndef MESSAGE_PROCESSING_H
#define MESSAGE_PROCESSING_H

// This file contains support for verifying proper send queue behavior

static const char* g_message_1 = "Some message here";
static const size_t g_message_1_size = 18;

static const char* g_message_2 = "Some other message here";
static const size_t g_message_2_size = 24;

static const char* g_message_doomed = "I'm surely going to fail";
static const size_t g_message_doomed_size = 25;

// Track whether a message is g_message_1
static bool last_message_was_g_message_1_content = false;
static bool last_message_was_g_message_1_pointer = false;
static const char* last_message_pointer_copy = NULL;

static const char* get_message_1()
{
    return g_message_1;
}

static const char* get_message_2()
{
    return g_message_2;
}

static const char* get_message_doomed()
{
    return g_message_doomed;
}

static void EXPECT_MESSAGE_CREATION()
{
    // Message 1 malloc
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
}

static void EXPECT_2_MESSAGE_DELETIONS()
{
    // Message 1 delete
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    // Message 2 delete
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
}

#endif // MESSAGE_PROCESSING_H
