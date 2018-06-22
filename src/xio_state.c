// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdlib.h>

#include <stdint.h>
#include <stdbool.h>
#include <limits.h>
#include "azure_c_shared_utility/xio.h"
#include "azure_c_shared_utility/gballoc.h"
#include "azure_c_shared_utility/xlogging.h"
#include "azure_c_shared_utility/singlylinkedlist.h"
#include "azure_c_shared_utility/crt_abstractions.h"
#include "azure_c_shared_utility/xio_state.h"
#include "azure_c_shared_utility/xio_adapter.h"

typedef struct
{
    unsigned char* bytes;
    size_t size;
    size_t unsent_size;
    ON_SEND_COMPLETE on_send_complete;
    void* callback_context;
} PENDING_TRANSMISSION;

typedef enum XIO_STATE_ENUM_TAG
{
    XIO_STATE_INITIAL,
    XIO_STATE_CLOSED,
    XIO_STATE_CLOSING,
    XIO_STATE_OPENING,
    XIO_STATE_OPEN,
    XIO_STATE_ERROR,
} XIO_STATE_ENUM;

typedef struct XIO_STATE_TAG
{
    SINGLYLINKEDLIST_HANDLE pending_transmission_list;
    ON_BYTES_RECEIVED on_bytes_received;
    ON_IO_ERROR on_io_error;
    ON_IO_OPEN_COMPLETE on_open_complete;
    ON_IO_CLOSE_COMPLETE on_close_complete;
    void* on_bytes_received_context;
    void* on_io_error_context;
    void* on_open_complete_context;
    void* on_close_complete_context;
    XIO_STATE_ENUM xio_state_state;
    const XIO_ADAPTER_INTERFACE* xio_adapter_interface;
    XIO_ADAPTER_INSTANCE_HANDLE xio_adapter_instance;
} XIO_STATE;

/* Codes_SRS_XIO_STATE_30_005: [ The phrase "enter XIO_STATE_ERROR" means xio_state shall call 
the on_io_error function and pass the on_io_error_context that was supplied in xio_state_open_async. ]*/
/* Codes_SRS_XIO_STATE_30_082: [ If the xio_adapter returns XIO_ASYNC_RESULT_FAILURE, 
xio_state_dowork shall log an error, call on_open_complete with the on_open_complete_context 
parameter provided in xio_state_open_async and IO_OPEN_ERROR, and enter XIO_STATE_CLOSED . ] */
static void enter_xio_state_error_state(XIO_STATE* xio_state)
{
    if (xio_state->xio_state_state != XIO_STATE_ERROR)
    {
        xio_state->xio_state_state = XIO_STATE_ERROR;
        xio_state->on_io_error(xio_state->on_io_error_context);
    }
}

/* Codes_SRS_XIO_STATE_30_005: [ The phrase "enter XIO_STATE_ERROR" means xio_state shall call
the on_io_error function and pass the on_io_error_context that was supplied in xio_state_open_async. ]*/
static void enter_open_error_state(XIO_STATE* xio_state)
{
    // save instance variables in case the framework destroys this object before we exit
    ON_IO_OPEN_COMPLETE on_open_complete = xio_state->on_open_complete;
    void* on_open_complete_context = xio_state->on_open_complete_context;
    enter_xio_state_error_state(xio_state);
    on_open_complete(on_open_complete_context, IO_OPEN_ERROR);
}

// Return true if a message was available to remove
static bool process_and_destroy_head_message(XIO_STATE* xio_state, IO_SEND_RESULT send_result)
{
    bool result;
    LIST_ITEM_HANDLE head_pending_io;
    if (send_result == IO_SEND_ERROR)
    {
        /* Codes_SRS_XIO_STATE_30_095: [ If the send process fails before sending all 
        of the bytes in an enqueued message, xio_state_dowork shall deque the message 
        per Message Processing Requirements and enter XIO_STATE_ERROR. ]*/
        enter_xio_state_error_state(xio_state);
    }
    head_pending_io = singlylinkedlist_get_head_item(xio_state->pending_transmission_list);
    if (head_pending_io != NULL)
    {
        PENDING_TRANSMISSION* head_message = (PENDING_TRANSMISSION*)singlylinkedlist_item_get_value(head_pending_io);

        if (singlylinkedlist_remove(xio_state->pending_transmission_list, head_pending_io) != 0)
        {
            // This particular situation is a bizarre and unrecoverable internal error
            /* Codes_SRS_XIO_STATE_30_095: [ If the send process fails before sending all
            of the bytes in an enqueued message, xio_state_dowork shall deque the message
            per Message Processing Requirements and enter XIO_STATE_ERROR. ]*/
            enter_xio_state_error_state(xio_state);
            LogError("Failed to remove message from list");
        }

        // on_send_complete is checked for NULL during PENDING_TRANSMISSION creation
        /* Codes_SRS_XIO_STATE_30_044: [ If a message was sent successfully, then after 
        it is dequeued xio_state shall call the message's on_send_complete along with 
        its associated callback_context and IO_SEND_OK. ]*/
        /* Codes_SRS_XIO_STATE_30_045: [ If a message was not sent successfully, then after
        it is dequeued xio_state shall call the message's on_send_complete along with
        its associated callback_context and IO_SEND_ERROR. ]*/
        head_message->on_send_complete(head_message->callback_context, send_result);

        /* Codes_SRS_XIO_STATE_30_047: [ When xio_state dequeues a message
        it shall free the message's data. ]*/
        free(head_message->bytes);
        free(head_message);
        result = true;
    }
    else
    {
        result = false;
    }
    return result;
}

static XIO_ASYNC_RESULT dowork_close(XIO_STATE* xio_state, bool force_close)
{
    /* Codes_SRS_XIO_STATE_30_105: [ xio_state_dowork shall call xio_adapter_close 
    on the xio_adapter provided during xio_state_open_async. ]*/
    XIO_ASYNC_RESULT close_result = xio_state->xio_adapter_interface->close(xio_state->xio_adapter_instance);
    if (close_result != XIO_ASYNC_RESULT_WAITING || force_close)
    {
        /* Codes_SRS_XIO_STATE_30_107: [ If the xio_adapter returns XIO_ASYNC_RESULT_SUCCESS,
        xio_state_dowork shall enter XIO_STATE_CLOSED. ]*/
        xio_state->xio_state_state = XIO_STATE_CLOSED;
        /* Codes_SRS_XIO_STATE_30_106: [ If the xio_adapter returns XIO_ASYNC_RESULT_FAILURE,
        xio_state_dowork shall log an error and enter XIO_STATE_CLOSED. ]*/
        if (close_result == XIO_ASYNC_RESULT_FAILURE)
        {
            LogError("xio_adapter_close failed");
        }

        /* Codes_SRS_XIO_STATE_30_009: [ The phrase "enter XIO_STATE_CLOSED" means xio_state
        shall deuque any unsent messages per Message Processing Requirements, then call the
        on_close_complete function and pass the on_close_complete_context that was
        supplied in xio_state_close_async. ]*/
        while (process_and_destroy_head_message(xio_state, IO_SEND_CANCELLED));
        // singlylinkedlist_destroy gets called in the main destroy

        xio_state->on_bytes_received = NULL;
        xio_state->on_io_error = NULL;
        xio_state->on_bytes_received_context = NULL;
        xio_state->on_io_error_context = NULL;
        xio_state->on_open_complete = NULL;
        xio_state->on_open_complete_context = NULL;
        if (xio_state->on_close_complete != NULL)
        {
            /* Codes_SRS_XIO_STATE_30_009: [ The phrase "enter XIO_STATE_CLOSED" means xio_state
            shall deuque any unsent messages per Message Processing Requirements, then call the
            on_close_complete function and pass the on_close_complete_context that was
            supplied in xio_state_close_async. ]*/
            xio_state->on_close_complete(xio_state->on_close_complete_context);
        }
    }

    /* Codes_SRS_XIO_STATE_30_108: [ If the xio_adapter returns XIO_ASYNC_RESULT_WAITING,
    xio_state_dowork shall remain in the XIO_STATE_CLOSING state. ]*/
    return close_result;
}

void xio_state_destroy(CONCRETE_IO_HANDLE xio_state_in)
{
    if (xio_state_in == NULL)
    {
        /* Codes_SRS_XIO_STATE_30_020: [ If xio_state_in is NULL, xio_state_destroy shall do nothing. ]*/
        LogError("NULL xio_state_in");
    }
    else
    {
        XIO_STATE* xio_state = (XIO_STATE*)xio_state_in;
        if (xio_state->xio_state_state == XIO_STATE_OPENING ||
            xio_state->xio_state_state == XIO_STATE_OPEN ||
            xio_state->xio_state_state == XIO_STATE_CLOSING ||
            xio_state->xio_state_state == XIO_STATE_ERROR)
        {
            /* Codes_SRS_XIO_STATE_30_022: [ If xio_state is in XIO_STATE_OPENING, xio_state_destroy
            shall call xio_adapter_close then enter XIO_STATE_CLOSED before releasing resources. ]*/
            /* Codes_SRS_XIO_STATE_30_023: [ If xio_state is in XIO_STATE_OPEN, xio_state_destroy
            shall call xio_adapter_close then enter XIO_STATE_CLOSED before releasing resources. ]*/
            /* Codes_SRS_XIO_STATE_30_024: [ If xio_state is in XIO_STATE_CLOSING, xio_state_destroy
            shall call xio_adapter_close then enter XIO_STATE_CLOSED before releasing resources. ]*/
            /* Codes_SRS_XIO_STATE_30_025: [ If xio_state is in XIO_STATE_ERROR, xio_state_destroy
            shall call xio_adapter_close then enter XIO_STATE_CLOSED before releasing resources. ]*/
            dowork_close(xio_state, true);
        }
        /* Codes_SRS_XIO_STATE_30_021: [ The xio_state_destroy call shall release all allocated resources, 
        call xio_adapter_destroy, and then release xio_state_handle. ]*/
        xio_state->xio_adapter_interface->destroy(xio_state->xio_adapter_instance);
        // singlylinkedlist_destroy accepts NULL with no error
        singlylinkedlist_destroy(xio_state->pending_transmission_list);

        free(xio_state);
    }
}

/* Codes_SRS_XIO_STATE_30_010: [ The xio_state_create shall allocate and initialize 
all necessary resources, call xio_adapter_create, and return an instance of the 
xio_state in XIO_STATE_INITIAL. ]*/
CONCRETE_IO_HANDLE xio_state_create(const XIO_ADAPTER_INTERFACE* adapter_interface, void* io_create_parameters)
{
    XIO_STATE* result;

    if (adapter_interface == NULL)
    {
        /* Codes_SRS_XIO_STATE_30_013: [ If the adapter interface parameter is NULL, xio_state_create
        shall log an error and return NULL. ]*/
        LogError("bad parameter");
        result = NULL;
    }
    else if (NULL == (result = malloc(sizeof(XIO_STATE))))
    {
        /* Codes_SRS_XIO_STATE_30_011: [ If xio_adapter_create fails or any resource 
        allocation fails, xio_state_create shall return NULL. ]*/
        LogError("malloc failed");
    }
    else
    {
        memset(result, 0, sizeof(XIO_STATE));
        result->xio_state_state = XIO_STATE_INITIAL;
        result->xio_adapter_interface = adapter_interface;
        result->pending_transmission_list = NULL;
        // Create the message queue if necessary
        if ((result->pending_transmission_list = singlylinkedlist_create()) == NULL)
        {
            /* Codes_SRS_XIO_STATE_30_011: [ If xio_adapter_create fails or any resource
            allocation fails, xio_state_create shall return NULL. ]*/
            LogError("Failed singlylinkedlist_create");
            xio_state_destroy(result);
            result = NULL;
        }
        /* Codes_SRS_XIO_STATE_30_012: [ The xio_state_create shall pass io_create_parameters to xio_adapter_create. ]*/
        else if ((result->xio_adapter_instance = result->xio_adapter_interface->create(io_create_parameters)) == NULL)
        {
            /* Codes_SRS_XIO_STATE_30_011: [ If xio_adapter_create fails or any resource
            allocation fails, xio_state_create shall return NULL. ]*/
            LogError("Failed xio_adapter create");
            xio_state_destroy(result);
            result = NULL;
        }
    }

    return (CONCRETE_IO_HANDLE)result;
}


int xio_state_open_async(CONCRETE_IO_HANDLE xio_state_in,
    ON_IO_OPEN_COMPLETE on_io_open_complete, void* on_io_open_complete_context,
    ON_BYTES_RECEIVED on_bytes_received, void* on_bytes_received_context,
    ON_IO_ERROR on_io_error, void* on_io_error_context)
{
    int result;
    /* Codes_SRS_XIO_STATE_30_030: [ If any of the xio_state_handle, on_open_complete, 
    on_bytes_received, or on_io_error parameters is NULL, xio_state_open_async shall 
    log an error and return _FAILURE_. ]*/
    if (xio_state_in == NULL || on_io_open_complete == NULL || on_bytes_received == NULL || on_io_error == NULL)
    {
        LogError("Required parameter is NULL");
        result = __FAILURE__;
    }
    else
    {
        XIO_STATE* xio_state = (XIO_STATE*)xio_state_in;

        if (xio_state->xio_state_state != XIO_STATE_INITIAL)
        {
            /* Codes_SRS_XIO_STATE_30_037: [ If xio_state is in any state other than 
            XIO_STATE_INITIAL when xio_state_open_async is called, it shall log an 
            error, and return _FAILURE_. ]*/
            LogError("Invalid xio_state_state. Expected state is XIO_STATE_INITIAL.");
            result = __FAILURE__;
        }
        else
        {
            /* The xio_state_open_async function will need to store the provided 
            on_bytes_received, on_bytes_received_context, on_io_error, on_io_error_context, 
            on_open_complete, and on_open_complete_context parameters for later use. 
            The use of these stored parameters is specified in other sections of this document. */
            xio_state->on_bytes_received = on_bytes_received;
            xio_state->on_bytes_received_context = on_bytes_received_context;

            xio_state->on_io_error = on_io_error;
            xio_state->on_io_error_context = on_io_error_context;

            xio_state->on_open_complete = on_io_open_complete;
            xio_state->on_open_complete_context = on_io_open_complete_context;

            /* Codes_SRS_XIO_STATE_30_035: [ On xio_state_open success xio_state shall enter XIO_STATE_EX_OPENING and return 0. ]*/
            // All the real work happens in dowork
            xio_state->xio_state_state = XIO_STATE_OPENING;
            result = 0;
        }
    }
    /* Codes_SRS_XIO_STATE_30_039: [ On failure, xio_state_open_async shall not call on_io_open_complete. ]*/

    return result;
}

int xio_state_close_async(CONCRETE_IO_HANDLE xio_state_in, ON_IO_CLOSE_COMPLETE on_io_close_complete, void* callback_context)
{
    int result;
    XIO_STATE* xio_state = (XIO_STATE*)xio_state_in;

    if (xio_state_in == NULL || on_io_close_complete == NULL)
    {
        /* Codes_SRS_XIO_STATE_30_050: [ If the xio_state_handle or on_close_complete parameter is NULL,
        xio_state_close_async shall log an error and return _FAILURE_. ]*/
        LogError("bad parameters");
        result = __FAILURE__;
    }
    else if (xio_state->xio_state_state == XIO_STATE_INITIAL)
    {
        /* Codes_SRS_XIO_STATE_30_053: [ If xio_state is in XIO_STATE_INITIAL it shall log an error and return _FAILURE_. ]*/
        LogError("bad state");
        result = __FAILURE__;
    }
    else if (xio_state->xio_state_state == XIO_STATE_CLOSED ||
        xio_state->xio_state_state == XIO_STATE_CLOSING)
    {
        /* Codes_SRS_XIO_STATE_30_055: [ If xio_state is in XIO_STATE_CLOSING then 
        xio_state_close_async shall do nothing and return 0. ]*/
        /* Codes_SRS_XIO_STATE_30_059: [ If xio_state is in XIO_STATE_CLOSED then 
        xio_state_close_async shall do nothing and return 0. ]*/
        result = 0;
    }
    else
    {
        if (xio_state->xio_state_state == XIO_STATE_OPENING)
        {
            /* Codes_SRS_XIO_STATE_30_057: [ If xio_state is in XIO_STATE_OPENING, it shall call on_io_open_complete with the
            on_io_open_complete_context supplied in xio_state_open_async and IO_OPEN_CANCELLED. ]*/
            // Redundantly setting the state here prevents reentry problems
            xio_state->xio_state_state = XIO_STATE_CLOSING;
            xio_state->on_open_complete(xio_state->on_open_complete_context, IO_OPEN_CANCELLED);
        }

        xio_state->on_close_complete = on_io_close_complete;
        xio_state->on_close_complete_context = callback_context;

        /* Codes_SRS_XIO_STATE_30_006: [ The phrase "enter XIO_STATE_CLOSING" means 
        xio_state shall call xio_adapter_close during subsequent calls to xio_state_dowork. ]*/
        xio_state->xio_state_state = XIO_STATE_CLOSING;
        /* Codes_SRS_XIO_STATE_30_058: [ xio_state_close_async shall call xio_adapter_close on its adapter. ]*/
        /* Codes_SRS_XIO_STATE_30_056: [ If xio_adapter_close returns XIO_ASYNC_RESULT_WAITING, 
        xio_state_close_async shall enter XIO_STATE_CLOSING and return 0. ]*/
        /* Codes_SRS_XIO_STATE_30_051: [ If xio_adapter_close returns XIO_ASYNC_RESULT_SUCCESS, 
        xio_state_close_async shall enter XIO_STATE_CLOSED and return 0. ]*/
        /* Codes_SRS_XIO_STATE_30_052: [ If xio_adapter_close returns XIO_ASYNC_RESULT_FAILURE, 
        xio_state_close_async shall enter XIO_STATE_CLOSED and return 0. ]*/
        dowork_close(xio_state, false);
        result = 0;
    }
    /* Codes_SRS_XIO_STATE_30_054: [ On failure, xio_state shall not call on_io_close_complete. ]*/

    return result;
}

// Only called from dowork while in XIO_STATE_OPEN
static void dowork_read(XIO_STATE* xio_state)
{
    /* Codes_SRS_XIO_STATE_30_100: [ The xio_state_dowork shall repeatedly call xio_adapter_read on the
    underlying xio_adapter until the return value is not XIO_ASYNC_RESULT_SUCCESS. ]*/
    /* Codes_SRS_XIO_STATE_30_103: [ If the xio_adapter_read returns returns data, it shall do so
    with the on_received callback and on_received_context provided in xio_state_open. ]*/
    XIO_ASYNC_RESULT result = XIO_ASYNC_RESULT_SUCCESS;
    while ((result = xio_state->xio_adapter_interface->read(xio_state->xio_adapter_instance)) == XIO_ASYNC_RESULT_SUCCESS);

    /* Codes_SRS_XIO_STATE_30_101: [ If the xio_adapter returns XIO_ASYNC_RESULT_WAITING, 
    xio_state_dowork shall do nothing. ]*/
    if (result == XIO_ASYNC_RESULT_FAILURE)
    {
        /* Codes_SRS_XIO_STATE_30_102: [ If the xio_adapter returns XIO_ASYNC_RESULT_FAILURE
        then xio_state_dowork shall enter XIO_STATE_ERROR. ]*/
        LogInfo("Communications error while reading");
        enter_xio_state_error_state(xio_state);
    }
}

static void dowork_send(XIO_STATE* xio_state)
{
    LIST_ITEM_HANDLE first_pending_io = singlylinkedlist_get_head_item(xio_state->pending_transmission_list);
    if (first_pending_io != NULL)
    {
        PENDING_TRANSMISSION* pending_message = (PENDING_TRANSMISSION*)singlylinkedlist_item_get_value(first_pending_io);
        uint8_t* buffer = ((uint8_t*)pending_message->bytes) + pending_message->size - pending_message->unsent_size;

        /* Codes_SRS_XIO_STATE_30_090: [ If there are any unsent messages in the queue, xio_state_dowork shall call 
        xio_adapter_write on the xio_adapter and pass in the first message in the queue. ]*/
        int write_result = xio_state->xio_adapter_interface->write(xio_state->xio_adapter_instance, buffer, (uint32_t)pending_message->unsent_size);
        if (write_result > 0)
        {
            pending_message->unsent_size -= write_result;
            if (pending_message->unsent_size == 0)
            {
                /* Codes_SRS_XIO_STATE_30_091: [ If xio_adapter_write is able to send all the bytes in the 
                enqueued message, xio_state_dowork shall deque the message per Message Processing Requirements. ]*/
                // The whole message has been sent successfully
                process_and_destroy_head_message(xio_state, IO_SEND_OK);
            }
            else
            {
                /* Codes_SRS_XIO_STATE_30_093: [ If xio_adapter_write was not able to send an entire 
                enqueued message at once, subsequent calls to xio_state_dowork shall continue 
                to send the remaining bytes. ]*/
                // Repeat the send on the next pass with the rest of the message
                // This empty else compiles to nothing but helps readability
            }
        }
        else if (write_result < 0)
        {
            // This is an unexpected error, and we need to bail out. Probably lost internet connection.
            /* Codes_SRS_XIO_STATE_30_095: [ If the send process fails before sending all of the 
            bytes in an enqueued message, xio_state_dowork shall deque the message per 
            Message Processing Requirements and enter XIO_STATE_ERROR. ]*/
            LogInfo("Unrecoverable error from xio_adapter_write");
            process_and_destroy_head_message(xio_state, IO_SEND_ERROR);
        }
    }
    else
    {
        /* Codes_SRS_XIO_STATE_30_096: [ If there are no enqueued messages available, xio_state_dowork shall do nothing. ]*/
    }
}

static void dowork_poll_open(XIO_STATE* xio_state)
{
    /* Codes_SRS_XIO_STATE_30_080: [ The xio_state_dowork shall call xio_adapter_open on the underlying xio_adapter. ]*/
    XIO_ASYNC_RESULT result = xio_state->xio_adapter_interface->open(xio_state->xio_adapter_instance,
        xio_state->on_bytes_received, xio_state->on_bytes_received_context);
    if (result == XIO_ASYNC_RESULT_SUCCESS)
    {
        // Connect succeeded
        xio_state->xio_state_state = XIO_STATE_OPEN;
        /* Codes_SRS_XIO_STATE_30_007: [ The phrase "enter XIO_STATE_OPEN" means xio_state shall call the
        on_io_open_complete function and pass IO_OPEN_OK and the on_io_open_complete_context
        that was supplied in xio_state_open . ]*/
        /* Codes_SRS_XIO_STATE_30_083: [ If xio_adapter returns XIO_ASYNC_RESULT_SUCCESS,
        xio_state_dowork shall enter XIO_STATE_OPEN. ]*/
        xio_state->on_open_complete(xio_state->on_open_complete_context, IO_OPEN_OK);
    }
    else if (result == XIO_ASYNC_RESULT_FAILURE)
    {
        /* Codes_SRS_XIO_STATE_30_082: [ If the xio_adapter returns XIO_ASYNC_RESULT_FAILURE,
        xio_state_dowork shall log an error, call on_open_complete with the on_open_complete_context
        parameter provided in xio_state_open_async and IO_OPEN_ERROR, and enter XIO_STATE_CLOSED. ] */
        enter_open_error_state(xio_state);
    }
    /* Codes_SRS_XIO_STATE_30_081: [ If the xio_adapter returns XIO_ASYNC_RESULT_WAITING,
    xio_state_dowork shall remain in the XIO_STATE_OPENING state. ] */
}

void xio_state_dowork(CONCRETE_IO_HANDLE xio_state_in)
{
    if (xio_state_in == NULL)
    {
        /* Codes_SRS_XIO_STATE_30_070: [ If the xio_state_in parameter is NULL, xio_state_dowork shall do nothing except log an error. ]*/
        LogError("NULL xio_state_in");
    }
    else
    {
        XIO_STATE* xio_state = (XIO_STATE*)xio_state_in;

        // This switch statement handles all of the state transitions during the opening process
        switch (xio_state->xio_state_state)
        {

        case XIO_STATE_INITIAL:
            /* Codes_SRS_XIO_STATE_30_072: [If xio_state is in XIO_STATE_INITIAL then xio_state_dowork shall do nothing.]*/
            // Waiting to be opened, nothing to do
            break;
        case XIO_STATE_CLOSED:
            /* Codes_SRS_XIO_STATE_30_075: [ If xio_state is in XIO_STATE_CLOSED then  xio_state_dowork  shall do nothing. ]*/
            break;
        case XIO_STATE_CLOSING:
            /* Codes_SRS_XIO_STATE_30_078: [ If xio_state is in XIO_STATE_CLOSING then xio_state_dowork 
            shall perform only the XIO_STATE_CLOSING behaviors. ]*/
            dowork_close(xio_state, false);
            break;
        case XIO_STATE_OPENING:
            /* Codes_SRS_XIO_STATE_30_076: [ If xio_state is in XIO_STATE_OPENING then 
            xio_state_dowork shall perform only the XIO_STATE_OPENING behaviors. ]*/
            dowork_poll_open(xio_state);
            break;
        case XIO_STATE_OPEN:
            /* Codes_SRS_XIO_STATE_30_077: [ If xio_state is in XIO_STATE_OPEN then 
            xio_state_dowork shall perform only the Data transmission behaviors and 
            the Data reception behaviors. ]*/
            dowork_read(xio_state);
            dowork_send(xio_state);
            break;
        case XIO_STATE_ERROR:
            /* Codes_SRS_XIO_STATE_30_071: [ If xio_state is in XIO_STATE_ERROR then xio_state_dowork shall do nothing. ]*/
            // There's nothing valid to do here but wait to be closed and destroyed
            break;
        default:
            LogError("Unexpected internal state");
            break;
        }
    }
}

int xio_state_setoption(CONCRETE_IO_HANDLE xio_state_in, const char* optionName, const void* value)
{
    XIO_STATE* xio_state = (XIO_STATE*)xio_state_in;
    /* Codes_SRS_XIO_STATE_30_120: [ If any of the the xio_state_handle, optionName, or value parameters is NULL,
    xio_state_setoption shall do nothing except log an error and return _FAILURE_. ]*/
    int result;
    if (xio_state == NULL || optionName == NULL || value == NULL)
    {
        LogError("NULL required parameter");
        result = __FAILURE__;
    }
    else
    {
        /* Codes_SRS_XIO_STATE_30_121 [ xio_state shall delegate the behavior of xio_state_setoption
        to the xio_adapter_config supplied in xio_state_create. ]*/
        int options_result = xio_state->xio_adapter_interface->setoption(xio_state->xio_adapter_instance, optionName, value);
        if (options_result != 0)
        {
            LogError("Failed xio_state_setoption");
            result = __FAILURE__;
        }
        else
        {
            result = 0;
        }
    }
    return result;
}

int xio_state_send_async(CONCRETE_IO_HANDLE xio_state_in, const void* buffer, size_t size, ON_SEND_COMPLETE on_send_complete, void* callback_context)
{
    XIO_ASYNC_RESULT result;
    XIO_STATE* xio_state = (XIO_STATE*)xio_state_in;

    if (on_send_complete == NULL || xio_state_in == NULL || buffer == NULL || size == 0 || size >= INT_MAX || on_send_complete == NULL)
    {
        /* Codes_SRS_XIO_STATE_30_060: [ If any of the xio_state_handle, buffer, or on_send_complete
        parameters is NULL, xio_state_send_async shall log an error and return XIO_ASYNC_RESULT_FAILURE. ]*/
        /* Codes_SRS_XIO_STATE_30_067: [ If the size is 0, xio_state_send_async shall log an error 
        and return XIO_ASYNC_RESULT_FAILURE. ]*/
        result = XIO_ASYNC_RESULT_FAILURE;
        LogError("Invalid parameter");
    }
    else if (xio_state->xio_state_state != XIO_STATE_OPEN)
    {
        /* Codes_SRS_XIO_STATE_30_065: [ If xio_state state is not XIO_STATE_OPEN, 
        xio_state_send_async shall log an error and return XIO_ASYNC_RESULT_FAILURE. ]]*/
        result = XIO_ASYNC_RESULT_FAILURE;
        LogError("xio_state_send_async without a prior successful open");
    }
    else
    {
        PENDING_TRANSMISSION* pending_transmission = (PENDING_TRANSMISSION*)malloc(sizeof(PENDING_TRANSMISSION));
        if (pending_transmission == NULL)
        {
            /* Codes_SRS_XIO_STATE_30_064: [ If the supplied message cannot be enqueued for transmission,
            xio_state_send_async shall log an error and return FAILURE. ]*/
            result = XIO_ASYNC_RESULT_FAILURE;
            LogError("malloc failed");
        }
        /* Codes_SRS_XIO_STATE_30_040: [ When xio_state enqueues a message it shall make a copy 
        of the data supplied in xio_state_send_async. ]*/
        else if ((pending_transmission->bytes = (unsigned char*)malloc(size)) == NULL)
        {
            /* Codes_SRS_XIO_STATE_30_064: [ If the supplied message cannot be enqueued for
            transmission, xio_state_send_async shall log an error and return FAILURE. ]*/
            LogError("malloc failed");
            free(pending_transmission);
            result = XIO_ASYNC_RESULT_FAILURE;
        }
        else
        {
            /* Codes_SRS_XIO_STATE_30_040: [ When xio_state enqueues a message it shall make a copy
            of the data supplied in xio_state_send_async. ]*/
            (void)memcpy(pending_transmission->bytes, buffer, size);
            pending_transmission->size = size;
            pending_transmission->unsent_size = size;
            pending_transmission->on_send_complete = on_send_complete;
            pending_transmission->callback_context = callback_context;

            if (singlylinkedlist_add(xio_state->pending_transmission_list, pending_transmission) == NULL)
            {
                /* Codes_SRS_XIO_STATE_30_064: [ If the supplied message cannot be enqueued for transmission,
                xio_state_send_async shall log an error and return FAILURE. ]*/
                LogError("Unable to add socket to pending list.");
                free(pending_transmission->bytes);
                free(pending_transmission);
                result = XIO_ASYNC_RESULT_FAILURE;
            }
            else
            {
                /* Codes_SRS_XIO_STATE_30_063: [ On success,  xio_state_send_async  shall enqueue
                for transmission the on_send_complete, the callback_context, the size,
                and the contents of  buffer  and then return 0. ]*/
                result = XIO_ASYNC_RESULT_SUCCESS;
                /* Codes_SRS_XIO_STATE_30_061: [ On success, after enqueuing the message, 
                xio_state_send_async shall invoke the Data Transmission behavior of xio_state_dowork. ]*/
                dowork_send(xio_state);
            }
        }
        /* Codes_SRS_XIO_STATE_30_066: [ On failure, on_send_complete shall not be called. ]*/
    }
    return result;
}

OPTIONHANDLER_HANDLE xio_state_retrieveoptions(CONCRETE_IO_HANDLE xio_state_in)
{
    XIO_STATE* xio_state = (XIO_STATE*)xio_state_in;
    /* Codes_SRS_XIO_STATE_30_160: [ If the xio_state_in parameter is NULL, xio_state_retrieveoptions
    shall do nothing except log an error and return FAILURE. ]*/
    OPTIONHANDLER_HANDLE result;
    if (xio_state == NULL)
    {
        LogError("NULL xio_state_in");
        result = NULL;
    }
    else
    {
        /* Codes_RS_XIO_STATE_30_161: [ xio_state shall delegate the behavior of
        xio_state_retrieveoptions to the xio_adapter supplied during xio_state_create. ]*/
        result = xio_state->xio_adapter_interface->retrieveoptions(xio_state->xio_adapter_instance);
    }
    return result;
}

