// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "azure_c_shared_utility/gballoc.h"
#include "azure_c_shared_utility/tlsio_appleios.h"
#include "azure_c_shared_utility/socketio.h"
#include "azure_c_shared_utility/optimize_size.h"
#include "azure_c_shared_utility/xlogging.h"
#include "azure_c_shared_utility/platform.h"
#include "azure_c_shared_utility/threadapi.h"
#include "azure_c_shared_utility/optionhandler.h"

#include <CoreFoundation/CoreFoundation.h>
#include <CFNetwork/CFSocketStream.h>

#define TLSIO_APPLEIOS_STATE_VALUES  \
    TLSIO_APPLEIOS_STATE_CLOSED,     \
    TLSIO_APPLEIOS_STATE_OPENING,    \
    TLSIO_APPLEIOS_STATE_OPEN,       \
    TLSIO_APPLEIOS_STATE_CLOSING,    \
    TLSIO_APPLEIOS_STATE_ERROR
DEFINE_ENUM(TLSIO_APPLEIOS_STATE, TLSIO_APPLEIOS_STATE_VALUES);

/* Codes_SRS_TLSIO_APPLEIOS_32_001: [ The tlsio_appleios shall implement and export all the Concrete functions in the VTable IO_INTERFACE_DESCRIPTION defined in the `xio.h`. ]*/
/* Codes_SRS_TLSIO_APPLEIOS_32_002: [ The tlsio_appleios shall report the open operation status using the IO_OPEN_RESULT enumerator defined in the `xio.h`.]*/
/* Codes_SRS_TLSIO_APPLEIOS_32_003: [ The tlsio_appleios shall report the send operation status using the IO_SEND_RESULT enumerator defined in the `xio.h`. ]*/
/* Codes_SRS_TLSIO_APPLEIOS_32_004: [ The tlsio_appleios shall call the callbacks functions defined in the `xio.h`. ]*/
#include "azure_c_shared_utility/xio.h"

/* Codes_SRS_TLSIO_APPLEIOS_32_005: [ The tlsio_appleios shall received the connection information using the TLSIO_CONFIG structure defined in `tlsio.h`. ]*/
#include "azure_c_shared_utility/tlsio.h"


/* Codes_SRS_TLSIO_APPLEIOS_32_001: [ The tlsio_appleios shall implement and export all the Concrete functions in the VTable IO_INTERFACE_DESCRIPTION defined in the `xio.h`. ]*/
static CONCRETE_IO_HANDLE tlsio_appleios_create(void* io_create_parameters);
static void tlsio_appleios_destroy(CONCRETE_IO_HANDLE tls_io);
static int tlsio_appleios_open_async(CONCRETE_IO_HANDLE tls_io, ON_IO_OPEN_COMPLETE on_io_open_complete, void* on_io_open_complete_context, ON_BYTES_RECEIVED on_bytes_received, void* on_bytes_received_context, ON_IO_ERROR on_io_error, void* on_io_error_context);
static int tlsio_appleios_close(CONCRETE_IO_HANDLE tls_io, ON_IO_CLOSE_COMPLETE on_io_close_complete, void* callback_context);
static int tlsio_appleios_send_async(CONCRETE_IO_HANDLE tls_io, const void* buffer, size_t size, ON_SEND_COMPLETE on_send_complete, void* callback_context);
static void tlsio_appleios_dowork(CONCRETE_IO_HANDLE tls_io);
static int tlsio_appleios_setoption(CONCRETE_IO_HANDLE tls_io, const char* optionName, const void* value);
static OPTIONHANDLER_HANDLE tlsio_appleios_retrieve_options(CONCRETE_IO_HANDLE tls_io);

static const IO_INTERFACE_DESCRIPTION tlsio_handle_interface_description =
{
    tlsio_appleios_retrieve_options,
    tlsio_appleios_create,
    tlsio_appleios_destroy,
    tlsio_appleios_open_async,
    tlsio_appleios_close,
    tlsio_appleios_send_async,
    tlsio_appleios_dowork,
    tlsio_appleios_setoption
};

#define RECEIVE_BUFFER_SIZE    128

#define CallErrorCallback() do { if (tlsio_instance->on_io_error != NULL) (void)tlsio_instance->on_io_error(tlsio_instance->on_io_error_context); } while((void)0,0)
#define CallOpenCallback(status) do { if (tlsio_instance->on_io_open_complete != NULL) (void)tlsio_instance->on_io_open_complete(tlsio_instance->on_io_open_complete_context, status); } while((void)0,0)
#define CallCloseCallback() do { if (tlsio_instance->on_io_close_complete != NULL) (void)tlsio_instance->on_io_close_complete(tlsio_instance->on_io_close_complete_context); } while((void)0,0)


typedef struct TLS_IO_INSTANCE_TAG
{
    ON_IO_OPEN_COMPLETE on_io_open_complete;
    void* on_io_open_complete_context;

    ON_BYTES_RECEIVED on_bytes_received;
    void* on_bytes_received_context;

    ON_IO_ERROR on_io_error;
    void* on_io_error_context;

    ON_IO_CLOSE_COMPLETE on_io_close_complete;
    void* on_io_close_complete_context;

    CFStringRef remote_host;
    uint16_t port;
	
    CFReadStreamRef sockRead;
    CFWriteStreamRef sockWrite;
	
    TLSIO_APPLEIOS_STATE state;
} TLS_IO_INSTANCE;

/* Codes_SRS_TLSIO_APPLEIOS_32_008: [ The tlsio_appleios_get_interface_description shall return the VTable IO_INTERFACE_DESCRIPTION. ]*/
const IO_INTERFACE_DESCRIPTION* tlsio_appleios_get_interface_description(void)
{
    return &tlsio_handle_interface_description;
}

/* Codes_SRS_TLSIO_APPLEIOS_32_005: [ The tlsio_appleios shall received the connection information using the TLSIO_CONFIG structure defined in `tlsio.h`. ]*/
/* Codes_SRS_TLSIO_APPLEIOS_32_009: [ The tlsio_appleios_create shall create a new instance of the tlsio for iOS. ]*/
/* Codes_SRS_TLSIO_APPLEIOS_32_017: [ The tlsio_appleios_create shall receive the connection configuration (TLSIO_CONFIG). ]*/
CONCRETE_IO_HANDLE tlsio_appleios_create(void* io_create_parameters)
{
    TLS_IO_INSTANCE* tlsio_instance;
    if (io_create_parameters == NULL)
    {
        LogError("Invalid TLS parameters.");
        tlsio_instance = NULL;
    }
    else
    {
        /* Codes_SRS_TLSIO_APPLEIOS_32_011: [ The tlsio_appleios_create shall allocate memory to control the tlsio instance. ]*/
        tlsio_instance = (TLS_IO_INSTANCE*)malloc(sizeof(TLS_IO_INSTANCE));
        if (tlsio_instance == NULL)
        {
            /* Codes_SRS_TLSIO_APPLEIOS_32_012: [ If there is not enough memory to control the tlsio, the tlsio_appleios_create shall return NULL as the handle. ]*/
            LogError("There is not enough memory to create the TLS instance.");
            /* return as is */
        }
        else
        {
            /* Codes_SRS_TLSIO_APPLEIOS_32_005: [ The tlsio_appleios shall received the connection information using the TLSIO_CONFIG structure defined in `tlsio.h`. ]*/
            /* Codes_SRS_TLSIO_APPLEIOS_32_017: [ The tlsio_appleios_create shall receive the connection configuration (TLSIO_CONFIG). ]*/
            TLSIO_CONFIG* tlsio_config = (TLSIO_CONFIG*)io_create_parameters;
            
            if (tlsio_config->hostname == NULL)
            {
                LogError("Host name was not provided");
            }
            else {
                /* Codes_SRS_TLSIO_APPLEIOS_32_016: [ The tlsio_appleios_create shall initialize all callback pointers as NULL. ]*/
                tlsio_instance->on_io_open_complete = NULL;
                tlsio_instance->on_io_open_complete_context = NULL;
                tlsio_instance->on_bytes_received = NULL;
                tlsio_instance->on_bytes_received_context = NULL;
                tlsio_instance->on_io_error = NULL;
                tlsio_instance->on_io_error_context = NULL;
                tlsio_instance->on_io_close_complete = NULL;
                tlsio_instance->on_io_close_complete_context = NULL;

                /* Codes_SRS_TLSIO_APPLEIOS_32_020: [ If tlsio_appleios_create get success to create the tlsio instance, it shall set the tlsio state as TLSIO_APPLEIOS_STATE_CLOSED. ]*/
                tlsio_instance->state = TLSIO_APPLEIOS_STATE_CLOSED;

                tlsio_instance->port = (uint16_t)tlsio_config->port;
                tlsio_instance->sockRead = NULL;
                tlsio_instance->sockWrite = NULL;

                /* Codes_SRS_TLSIO_APPLEIOS_32_018: [ The tlsio_appleios_create shall store provided hostName. ]*/
                tlsio_instance->remote_host = CFStringCreateWithCString(NULL, tlsio_config->hostname, kCFStringEncodingUTF8);
                
                if (tlsio_instance->remote_host == NULL)
                {
                    LogError("Unable to allocate string for host name");
                    free(tlsio_instance);
                    tlsio_instance = NULL;
                }
            }
        }
    }

    /* Codes_SRS_TLSIO_APPLEIOS_32_010: [ The tlsio_appleios_create shall return a non-NULL handle on success. ]*/
    return (CONCRETE_IO_HANDLE)tlsio_instance;
}

/* Codes_SRS_TLSIO_APPLEIOS_32_021: [ The tlsio_appleios_destroy shall destroy a created instance of the tlsio for iOS identified by the CONCRETE_IO_HANDLE. ]*/
void tlsio_appleios_destroy(CONCRETE_IO_HANDLE tlsio_handle)
{
    TLS_IO_INSTANCE* tlsio_instance = (TLS_IO_INSTANCE*)tlsio_handle;

    if (tlsio_handle == NULL)
    {
        /* Codes_SRS_TLSIO_APPLEIOS_32_024: [ If the tlsio_handle is NULL, the tlsio_appleios_destroy shall not do anything. ]*/
        LogError("NULL TLS handle.");
    }
    else
    {
		if (tlsio_instance->state == TLSIO_APPLEIOS_STATE_CLOSING)
		{
			int i = 10;
			
			while (i--)
			{
				tlsio_appleios_dowork(tlsio_handle);
				
				if (tlsio_instance->state == TLSIO_APPLEIOS_STATE_CLOSED)
					break;
			}
		}
        if ((tlsio_instance->state == TLSIO_APPLEIOS_STATE_OPENING) ||
            (tlsio_instance->state == TLSIO_APPLEIOS_STATE_OPEN) ||
            (tlsio_instance->state == TLSIO_APPLEIOS_STATE_CLOSING))
        {
            /* Codes_SRS_TLSIO_APPLEIOS_32_026: [ If the tlsio state is TLSIO_APPLEIOS_STATE_OPENING, TLSIO_APPLEIOS_STATE_OPEN, or TLSIO_APPLEIOS_STATE_CLOSING, the tlsio_appleios_destroy shall close destroy the tlsio, but log an error. ]*/
            LogError("TLS destroyed with a SSL connection still active.");
        }

        /* Codes_SRS_TLSIO_APPLEIOS_32_022: [ The tlsio_appleios_destroy shall free the memory allocated for tlsio_instance. ]*/
		CFRelease(tlsio_instance->remote_host);
		CFRelease(tlsio_instance->sockRead);
		CFRelease(tlsio_instance->sockWrite);
        free(tlsio_instance);
    }
}

/* Codes_SRS_TLSIO_APPLEIOS_32_026: [ The tlsio_appleios_open shall star the process to open the ssl connection with the host provided in the tlsio_appleios_create. ]*/
int tlsio_appleios_open_async(
    CONCRETE_IO_HANDLE tlsio_handle,
    ON_IO_OPEN_COMPLETE on_io_open_complete,
    void* on_io_open_complete_context,
    ON_BYTES_RECEIVED on_bytes_received,
    void* on_bytes_received_context,
    ON_IO_ERROR on_io_error,
    void* on_io_error_context)
{
    int result;
    TLS_IO_INSTANCE* tlsio_instance = (TLS_IO_INSTANCE*)tlsio_handle;

    if (tlsio_handle == NULL)
    {
        /* Codes_SRS_TLSIO_APPLEIOS_32_036: [ If the tlsio_handle is NULL, the tlsio_appleios_open shall not do anything, and return _LINE_. ]*/
        LogError("NULL TLS handle.");
        result = __FAILURE__;
    }
	else if (on_io_open_complete == NULL)
	{
		LogError("NULL open complete function");
		result = __FAILURE__;
	}
	else if (on_bytes_received == NULL)
	{
		LogError("NULL byte received function");
		result = __FAILURE__;
	}
	else if (on_io_error == NULL)
	{
		LogError("NULL error function");
		result = __FAILURE__;
	}
    else
    {
        /* Codes_SRS_TLSIO_APPLEIOS_32_004: [ The tlsio_appleios shall call the callbacks functions defined in the `xio.h`. ]*/
        /* Codes_SRS_TLSIO_APPLEIOS_32_006: [ The tlsio_appleios shall return the status of all async operations using the callbacks. ]*/
        /* Codes_SRS_TLSIO_APPLEIOS_32_007: [ If the callback function is set as NULL. The tlsio_appleios shall not call anything. ]*/
        /* Codes_SRS_TLSIO_APPLEIOS_32_028: [ The tlsio_appleios_open shall store the provided on_io_open_complete callback function address. ]*/
        tlsio_instance->on_io_open_complete = on_io_open_complete;
        /* Codes_SRS_TLSIO_APPLEIOS_32_029: [ The tlsio_appleios_open shall store the provided on_io_open_complete_context handle. ]*/
        tlsio_instance->on_io_open_complete_context = on_io_open_complete_context;

        /* Codes_SRS_TLSIO_APPLEIOS_32_030: [ The tlsio_appleios_open shall store the provided on_bytes_received callback function address. ]*/
        tlsio_instance->on_bytes_received = on_bytes_received;
        /* Codes_SRS_TLSIO_APPLEIOS_32_031: [ The tlsio_appleios_open shall store the provided on_bytes_received_context handle. ]*/
        tlsio_instance->on_bytes_received_context = on_bytes_received_context;

        /* Codes_SRS_TLSIO_APPLEIOS_32_032: [ The tlsio_appleios_open shall store the provided on_io_error callback function address. ]*/
        tlsio_instance->on_io_error = on_io_error;
        /* Codes_SRS_TLSIO_APPLEIOS_32_033: [ The tlsio_appleios_open shall store the provided on_io_error_context handle. ]*/
        tlsio_instance->on_io_error_context = on_io_error_context;

        if ((tlsio_instance->state == TLSIO_APPLEIOS_STATE_ERROR) ||
            (tlsio_instance->state == TLSIO_APPLEIOS_STATE_OPENING) ||
            (tlsio_instance->state == TLSIO_APPLEIOS_STATE_OPEN) ||
            (tlsio_instance->state == TLSIO_APPLEIOS_STATE_CLOSING))
        {
            /* Codes_SRS_TLSIO_APPLEIOS_32_035: [ If the tlsio state is TLSIO_APPLEIOS_STATE_ERROR, TLSIO_APPLEIOS_STATE_OPENING, TLSIO_APPLEIOS_STATE_OPEN, or TLSIO_APPLEIOS_STATE_CLOSING, the tlsio_appleios_open shall set the tlsio state as TLSIO_APPLEIOS_STATE_ERROR, and return _LINE_. ]*/
            LogError("Try to open a connection with an already opened or broken TLS.");
            result = __FAILURE__;
        }
        else
        {
		    CFStreamStatus status;
			
			status = (tlsio_instance->sockRead == NULL)? kCFStreamStatusClosed : CFReadStreamGetStatus(tlsio_instance->sockRead);
			
			if (status != kCFStreamStatusClosed)
            {
                /* Codes_SRS_TLSIO_APPLEIOS_32_037: [ If the ssl client is connected, the tlsio_appleios_open shall change the state to TLSIO_APPLEIOS_STATE_ERROR, log the error, and return _LINE_. ]*/
                LogError("SSL stream is not closed.");
                tlsio_instance->state = TLSIO_APPLEIOS_STATE_ERROR;
                result = __FAILURE__;
            }
			else
			{
	            CFStreamCreatePairWithSocketToHost(NULL, tlsio_instance->remote_host, tlsio_instance->port, &tlsio_instance->sockRead, &tlsio_instance->sockWrite);
				
				if (tlsio_instance->sockRead == NULL || tlsio_instance->sockWrite == NULL)
				{
					LogError("Unable to create socket streams.");
					tlsio_instance->state = TLSIO_APPLEIOS_STATE_ERROR;
					result = __FAILURE__;
				}
				else if (!CFReadStreamSetProperty(tlsio_instance->sockRead, kCFStreamPropertySSLSettings, kCFStreamSocketSecurityLevelTLSv1))
				{
					LogError("Unable to start SSL on socket.");
					tlsio_instance->state = TLSIO_APPLEIOS_STATE_ERROR;
					result = __FAILURE__;
				}
	            else if (!CFReadStreamOpen(tlsio_instance->sockRead) || !CFWriteStreamOpen(tlsio_instance->sockWrite))
				{
					LogError("Failure: connect failure");
					CFReadStreamClose(tlsio_instance->sockRead);
					CFWriteStreamClose(tlsio_instance->sockWrite);
					tlsio_instance->sockRead = NULL;
					tlsio_instance->sockWrite = NULL;
					result = __FAILURE__;
				}
				else
				{
					tlsio_instance->state = TLSIO_APPLEIOS_STATE_OPENING;
					result = 0;
				}
			}
		}
    }

    return result;
}

/* Codes_SRS_TLSIO_APPLEIOS_32_043: [ The tlsio_appleios_close shall start the process to close the ssl connection. ]*/
int tlsio_appleios_close(CONCRETE_IO_HANDLE tlsio_handle, ON_IO_CLOSE_COMPLETE on_io_close_complete, void* on_io_close_complete_context)
{
    int result;
    TLS_IO_INSTANCE* tlsio_instance = (TLS_IO_INSTANCE*)tlsio_handle;

    if (tlsio_handle == NULL)
    {
        /* Codes_SRS_TLSIO_APPLEIOS_32_049: [ If the tlsio_handle is NULL, the tlsio_appleios_close shall not do anything, and return _LINE_. ]*/
        LogError("NULL TLS handle.");
        result = __FAILURE__;
    }
    else
    {
        /* Codes_SRS_TLSIO_APPLEIOS_32_045: [ The tlsio_appleios_close shall store the provided on_io_close_complete callback function address. ]*/
        tlsio_instance->on_io_close_complete = on_io_close_complete;
        /* Codes_SRS_TLSIO_APPLEIOS_32_046: [ The tlsio_appleios_close shall store the provided on_io_close_complete_context handle. ]*/
        tlsio_instance->on_io_close_complete_context = on_io_close_complete_context;

        if ((tlsio_instance->state == TLSIO_APPLEIOS_STATE_CLOSED) || (tlsio_instance->state == TLSIO_APPLEIOS_STATE_ERROR))
        {
            /* Codes_SRS_TLSIO_APPLEIOS_32_079: [ If the tlsio state is TLSIO_APPLEIOS_STATE_ERROR, or TLSIO_APPLEIOS_STATE_CLOSED, the tlsio_appleios_close shall set the tlsio state as TLSIO_APPLEIOS_STATE_CLOSE, and return 0. ]*/
            tlsio_instance->state = TLSIO_APPLEIOS_STATE_CLOSED;
            result = 0;
        }
        else if ((tlsio_instance->state == TLSIO_APPLEIOS_STATE_OPENING) || (tlsio_instance->state == TLSIO_APPLEIOS_STATE_CLOSING))
        {
            /* Codes_SRS_TLSIO_APPLEIOS_32_048: [ If the tlsio state is TLSIO_APPLEIOS_STATE_OPENING, or TLSIO_APPLEIOS_STATE_CLOSING, the tlsio_appleios_close shall set the tlsio state as TLSIO_APPLEIOS_STATE_CLOSE, and return _LINE_. ]*/
            LogError("Try to close the connection with an already closed TLS.");
            tlsio_instance->state = TLSIO_APPLEIOS_STATE_ERROR;
            result = __FAILURE__;
        }
        else
        {
            CFReadStreamClose(tlsio_instance->sockRead);
            CFWriteStreamClose(tlsio_instance->sockWrite);
            /* Codes_SRS_TLSIO_APPLEIOS_32_047: [ If tlsio_appleios_close get success to start the process to close the ssl connection, it shall set the tlsio state as TLSIO_APPLEIOS_STATE_CLOSING, and return 0. ]*/
            tlsio_instance->state = TLSIO_APPLEIOS_STATE_CLOSING;
            result = 0;
        }
    }

    return result;
}

/* Codes_SRS_TLSIO_APPLEIOS_32_052: [ The tlsio_appleios_send shall send all bytes in a buffer to the ssl connectio. ]*/
int tlsio_appleios_send_async(CONCRETE_IO_HANDLE tlsio_handle, const void* buffer, size_t size, ON_SEND_COMPLETE on_send_complete, void* callback_context)
{
    int result;
    TLS_IO_INSTANCE* tlsio_instance = (TLS_IO_INSTANCE*)tlsio_handle;

    if ((tlsio_handle == NULL) || (buffer == NULL) || (size == 0))
    {
        /* Codes_SRS_TLSIO_APPLEIOS_32_059: [ If the tlsio_handle is NULL, the tlsio_appleios_send shall not do anything, and return _LINE_. ]*/
        /* Codes_SRS_TLSIO_APPLEIOS_32_060: [ If the buffer is NULL, the tlsio_appleios_send shall not do anything, and return _LINE_. ]*/
        /* Codes_SRS_TLSIO_APPLEIOS_32_061: [ If the size is 0, the tlsio_appleios_send shall not do anything, and return _LINE_. ]*/
        LogError("Invalid parameter");
        result = __FAILURE__;
    }
    else if (tlsio_instance->state != TLSIO_APPLEIOS_STATE_OPEN)
    {
        /* Codes_SRS_TLSIO_APPLEIOS_32_058: [ If the tlsio state is TLSIO_APPLEIOS_STATE_ERROR, TLSIO_APPLEIOS_STATE_OPENING, TLSIO_APPLEIOS_STATE_CLOSING, or TLSIO_APPLEIOS_STATE_CLOSED, the tlsio_appleios_send shall call the on_send_complete with IO_SEND_ERROR, and return _LINE_. ]*/
        LogError("TLS is not ready to send data");
        result = __FAILURE__;
    }
    else
    {
        CFIndex send_result;
        size_t send_size = size;
        const uint8_t* runBuffer = (const uint8_t *)buffer;
        result = __FAILURE__;
        /* Codes_SRS_TLSIO_APPLEIOS_32_055: [ if the ssl was not able to send all data in the buffer, the tlsio_appleios_send shall call the ssl again to send the remaining bytes. ]*/
        while (send_size > 0)
        {
			send_result = CFWriteStreamWrite(tlsio_instance->sockWrite, buffer, send_size);
			
			if (send_result == -1)
            {
                /* Codes_SRS_TLSIO_APPLEIOS_32_056: [ if the ssl was not able to send any byte in the buffer, the tlsio_appleios_send shall call the on_send_complete with IO_SEND_ERROR, and return _LINE_. ]*/
                LogError("TLS failed sending data");
                send_size = 0;
                result = __FAILURE__;
            }
            else if (send_result >= send_size) /* Transmit it all. */
            {
                /* Codes_SRS_TLSIO_APPLEIOS_32_057: [ if the ssl finish to send all bytes in the buffer, the tlsio_appleios_send shall call the on_send_complete with IO_SEND_OK, and return 0 ]*/
                /* Codes_SRS_TLSIO_APPLEIOS_32_053: [ The tlsio_appleios_send shall use the provided on_io_send_complete callback function address. ]*/
                /* Codes_SRS_TLSIO_APPLEIOS_32_054: [ The tlsio_appleios_send shall use the provided on_io_send_complete_context handle. ]*/
                if (on_send_complete != NULL)
                {
                    /* Codes_SRS_TLSIO_APPLEIOS_32_003: [ The tlsio_appleios shall report the send operation status using the IO_SEND_RESULT enumerator defined in the `xio.h`. ]*/
                    on_send_complete(callback_context, IO_SEND_OK);
                }
                result = 0;
                send_size = 0;
            }
            else /* Still have buffer to transmit. */
            {
                runBuffer += send_result;
                send_size -= send_result;
            }
        }
    }
    return result;
}

/* Codes_SRS_TLSIO_APPLEIOS_32_062: [ The tlsio_appleios_dowork shall execute the async jobs for the tlsio. ]*/
void tlsio_appleios_dowork(CONCRETE_IO_HANDLE tlsio_handle)
{
    if (tlsio_handle == NULL)
    {
        /* Codes_SRS_TLSIO_APPLEIOS_32_074: [ If the tlsio_handle is NULL, the tlsio_appleios_dowork shall not do anything. ]*/
        LogError("Invalid parameter");
    }
    else
    {
        long received;
        TLS_IO_INSTANCE* tlsio_instance = (TLS_IO_INSTANCE*)tlsio_handle;
        /* Codes_SRS_TLSIO_APPLEIOS_32_075: [ The tlsio_appleios_dowork shall create a buffer to store the data received from the ssl client. ]*/
        /* Codes_SRS_TLSIO_APPLEIOS_32_076: [ The tlsio_appleios_dowork shall delete the buffer to store the data received from the ssl client. ]*/
        uint8_t RecvBuffer[RECEIVE_BUFFER_SIZE];
		
	    CFStreamStatus statusRead = CFReadStreamGetStatus(tlsio_instance->sockRead);
	    //CFStreamStatus statusWrite = CFWriteStreamGetStatus(tlsio_instance->sockWrite);

        switch (tlsio_instance->state)
        {
        case TLSIO_APPLEIOS_STATE_OPENING:
            if (statusRead == kCFStreamStatusOpen)
            {
                /* Codes_SRS_TLSIO_APPLEIOS_32_063: [ If the tlsio state is TLSIO_APPLEIOS_STATE_OPENING, and ssl client is connected, the tlsio_appleios_dowork shall change the tlsio state to TLSIO_APPLEIOS_STATE_OPEN, and call the on_io_open_complete with IO_OPEN_OK. ]*/
                tlsio_instance->state = TLSIO_APPLEIOS_STATE_OPEN;
                /* Codes_SRS_TLSIO_APPLEIOS_32_002: [ The tlsio_appleios shall report the open operation status using the IO_OPEN_RESULT enumerator defined in the `xio.h`.]*/
                CallOpenCallback(IO_OPEN_OK);
            }
            break;
        case TLSIO_APPLEIOS_STATE_OPEN:
            if (statusRead == kCFStreamStatusNotOpen || statusRead == kCFStreamStatusError)
            {
                /* Codes_SRS_TLSIO_APPLEIOS_32_071: [ If the tlsio state is TLSIO_APPLEIOS_STATE_OPEN, and ssl client is not connected, the tlsio_appleios_dowork shall change the state to TLSIO_APPLEIOS_STATE_ERROR, call on_io_error. ]*/
                tlsio_instance->state = TLSIO_APPLEIOS_STATE_ERROR;
                LogError("SSL closed the connection.");
                CallErrorCallback();
            }
            else
            {
                /* Codes_SRS_TLSIO_APPLEIOS_32_069: [ If the tlsio state is TLSIO_APPLEIOS_STATE_OPEN, the tlsio_appleios_dowork shall read data from the ssl client. ]*/
				while (CFReadStreamHasBytesAvailable(tlsio_instance->sockRead))
				{
					received = CFReadStreamRead(tlsio_instance->sockRead, (uint8_t*)RecvBuffer, (CFIndex)RECEIVE_BUFFER_SIZE);
					if (received > 0)
					{
						/* Codes_SRS_TLSIO_APPLEIOS_32_070: [ If the tlsio state is TLSIO_APPLEIOS_STATE_OPEN, and there are received data in the ssl client, the tlsio_appleios_dowork shall read this data and call the on_bytes_received with the pointer to the buffer with the data. ]*/
						if (tlsio_instance->on_bytes_received != NULL)
						{
							// explictly ignoring here the result of the callback
							(void)tlsio_instance->on_bytes_received(tlsio_instance->on_bytes_received_context, (const unsigned char*)RecvBuffer, received);
						}
					}
                    else if (received < 0)
                    {
                        LogError("Error reading from socket");
                        CallErrorCallback();
                    }
				}
            }
            break;
        case TLSIO_APPLEIOS_STATE_CLOSING:
            if (statusRead == kCFStreamStatusClosed)
            {
                /* Codes_SRS_TLSIO_APPLEIOS_32_066: [ If the tlsio state is TLSIO_APPLEIOS_STATE_CLOSING, and ssl client is not connected, the tlsio_appleios_dowork shall change the tlsio state to TLSIO_APPLEIOS_STATE_CLOSE, and call the on_io_close_complete. ]*/
                tlsio_instance->state = TLSIO_APPLEIOS_STATE_CLOSED;
                CallCloseCallback();
            }
            break;
        case TLSIO_APPLEIOS_STATE_CLOSED:
            if (statusRead != kCFStreamStatusClosed)
            {
                /* Codes_SRS_TLSIO_APPLEIOS_32_072: [ If the tlsio state is TLSIO_APPLEIOS_STATE_CLOSED, and ssl client is connected, the tlsio_appleios_dowork shall change the state to TLSIO_APPLEIOS_STATE_ERROR, call on_io_error. ]*/
                tlsio_instance->state = TLSIO_APPLEIOS_STATE_ERROR;
                LogError("SSL keep the connection open with TLS closed.");
                CallErrorCallback();
            }
        case TLSIO_APPLEIOS_STATE_ERROR:
            /* Codes_SRS_TLSIO_APPLEIOS_32_073: [ If the tlsio state is TLSIO_APPLEIOS_STATE_ERROR, the tlsio_appleios_dowork shall not do anything. ]*/
        default:
            break;
        }
    }
}

int tlsio_appleios_setoption(CONCRETE_IO_HANDLE tlsio_handle, const char* optionName, const void* value)
{
    /* Codes_SRS_TLSIO_APPLEIOS_32_077: [ The tlsio_appleios_setoption shall not do anything, and return 0. ]*/
    (void)tlsio_handle, (void)optionName, (void)value;

    /* Not implementing any options */
    return 0;
}

OPTIONHANDLER_HANDLE tlsio_appleios_retrieve_options(CONCRETE_IO_HANDLE tlsio_handle)
{
    /* Codes_SRS_TLSIO_APPLEIOS_32_078: [ The tlsio_appleios_retrieve_options shall not do anything, and return NULL. ]*/
    (void)(tlsio_handle);
        
    /* Not implementing any options */
    return NULL;
}


