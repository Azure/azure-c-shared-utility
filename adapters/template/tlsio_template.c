// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

/* This is a template file used for porting */

/* This is a template for a TLS IO adapter for a TLS library that directly talks to the sockets
Go through each TODO item in this file and replace the called out function calls with your own TLS library calls.
Please refer to the porting guide for more details.

Make sure that you replace tlsio_template everywhere in this file with your own TLS library name (like tlsio_mytls) */

#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include "azure_c_shared_utility/gballoc.h"
#include "azure_c_shared_utility/tlsio.h"
#include "azure_c_shared_utility/tlsio_template.h"
#include "azure_c_shared_utility/xlogging.h"
#include "azure_c_shared_utility/crt_abstractions.h"

/* TODO: If more states are needed, simply add them to the enum. Most of the implementation will not require additional states.
   Example of another state would be TLSIO_STATE_RENEGOTIATING, etc. */
typedef enum TLSIO_STATE_TAG
{
    TLSIO_STATE_NOT_OPEN,
    TLSIO_STATE_OPEN,
    TLSIO_STATE_ERROR
} TLSIO_STATE;

typedef struct TLS_IO_INSTANCE_TAG
{
    ON_IO_OPEN_COMPLETE on_io_open_complete;
    void* on_io_open_complete_context;
    ON_BYTES_RECEIVED on_bytes_received;
    void* on_bytes_received_context;
    ON_IO_ERROR on_io_error;
    void* on_io_error_context;
    TLSIO_STATE tlsio_state;
    char* hostname;
    int port;
    char* certificates;
	
	/* TODO: A typical thing to do is to add here a member variable for the TLS library context, like
	TlsContext tls_context;
	*/
} TLS_IO_INSTANCE;

static int tlsio_template_close(CONCRETE_IO_HANDLE tls_io, ON_IO_CLOSE_COMPLETE on_io_close_complete, void* on_io_close_complete_context);

/*this function will clone an option given by name and value*/
static void* tlsio_template_clone_option(const char* name, const void* value)
{
    void* result;

    if((name == NULL) || (value == NULL))
    {
        LogError("invalid parameter detected: const char* name=%p, const void* value=%p", name, value);
        result = NULL;
    }
    else
    {
		/* TODO: This only handles TrustedCerts, if you need to handle more options specific to your TLS library, fill the code in here

        if (strcmp(name, "...my_option...") == 0)
        {
			// ... copy the option and assign it to result to be returned.
		}
		else
		*/
        if (strcmp(name, "TrustedCerts") == 0)
        {
            if(mallocAndStrcpy_s((char**)&result, value) != 0)
            {
                LogError("unable to mallocAndStrcpy_s TrustedCerts value");
                result = NULL;
            }
            else
            {
                /*return as is*/
            }
        }
        else
        {
            LogError("not handled option : %s", name);
            result = NULL;
        }
    }
    return result;
}

/*this function destroys an option previously created*/
static void tlsio_template_destroy_option(const char* name, const void* value)
{
    /*since all options for this layer are actually string copies., disposing of one is just calling free*/

    if ((name == NULL) || (value == NULL))
    {
        LogError("invalid parameter detected: const char* name=%p, const void* value=%p", name, value);
    }
	else
	{
		/* TODO: This only handles TrustedCerts, if you need to handle more options specific to your TLS library, fill the code in here

        if (strcmp(name, "...my_option...") == 0)
        {
			// ... free any resources for the option
		}
		else
		*/
		if (strcmp(name, "TrustedCerts") == 0)
		{
			free((void*)value);
		}
        else
        {
            LogError("not handled option : %s", name);
        }
    }
}

static CONCRETE_IO_HANDLE tlsio_template_create(void* io_create_parameters)
{
    TLS_IO_INSTANCE* result;

	/* check whether the argument is good */
    if (io_create_parameters == NULL)
    {
        result = NULL;
        LogError("NULL tls_io_config.");
    }
    else
    {
        TLSIO_CONFIG* tls_io_config = io_create_parameters;

		/* check if the hostname is good */
        if (tls_io_config->hostname == NULL)
        {
            result = NULL;
            LogError("NULL hostname in the TLS IO configuration.");
        }
        else
        {
			/* allocate */
            result = malloc(sizeof(TLS_IO_INSTANCE));
            if (result == NULL)
            {
                LogError("Failed allocating TLSIO instance.");
            }
            else
            {
                size_t i;

				/* copy the hostname for later use in open */
                if (mallocAndStrcpy_s(&result->hostname, tls_io_config->hostname) != 0)
                {
                    LogError("Failed to copy the hostname.");
                    free(result);
                    result = NULL;
                }
                else
                {
					/* copy port and initialize all the callback data */
                    result->port = tls_io_config->port;
                    result->certificate = NULL;
                    result->on_bytes_received = NULL;
                    result->on_bytes_received_context = NULL;
                    result->on_io_open_complete = NULL;
                    result->on_io_open_complete_context = NULL;
                    result->on_io_error = NULL;
                    result->on_io_error_context = NULL;
                    result->tlsio_state = TLSIO_STATE_NOT_OPEN;

					/* TODO: here you would have to instantiate the TLS library context/handle (TlsCreate is the function that creates a TLS context for your library):

					result->tls_context = {TlsCreate}(...);
					if (... check for error creating context ...)
					{
						free(result->hostname);
						free(result);
						LogError("Creating the TLS context failed");
						result = NULL;
					}*/
                }
            }
        }
    }

    return result;
}

static void tlsio_template_destroy(CONCRETE_IO_HANDLE tls_io)
{
    if (tls_io == NULL)
    {
        LogError("NULL tls_io.");
    }
    else
    {
        TLS_IO_INSTANCE* tls_io_instance = (TLS_IO_INSTANCE*)tls_io;

        /* force a close when destroying */
        tlsio_template_close(tls_io, NULL, NULL);

        /* TODO: here you should free the TLS context for your library. Replace the TlsFree call with whatever function call is there for your TLS library.
        TlsFree(tls_io_instance->tlsContext);*/

        if (tls_io_instance->certificate != NULL)
        {
            free(tls_io_instance->certificate);
        }
        free(tls_io_instance->hostname);
        free(tls_io);
    }
}

static int tlsio_template_open(CONCRETE_IO_HANDLE tls_io, ON_IO_OPEN_COMPLETE on_io_open_complete, void* on_io_open_complete_context, ON_BYTES_RECEIVED on_bytes_received, void* on_bytes_received_context, ON_IO_ERROR on_io_error, void* on_io_error_context)
{
    int result;

    /* check arguments */
    if ((tls_io == NULL) ||
        (on_io_open_complete == NULL) ||
        (on_bytes_received == NULL) ||
        (on_io_error == NULL))
    {
        result = __LINE__;
        LogError("NULL tls_io.");
    }
    else
    {
        TLS_IO_INSTANCE* tls_io_instance = (TLS_IO_INSTANCE*)tls_io;

        if (tls_io_instance->tlsio_state != TLSIO_STATE_NOT_OPEN)
        {
            result = __LINE__;
            LogError("Invalid tlsio_state. Expected state is TLSIO_STATE_NOT_OPEN.");
        }
        else
        {
			unsigned char is_error = 0;
			
            tls_io_instance->on_bytes_received = on_bytes_received;
            tls_io_instance->on_bytes_received_context = on_bytes_received_context;

            tls_io_instance->on_io_error = on_io_error;
            tls_io_instance->on_io_error_context = on_io_error_context;

            tls_io_instance->on_io_open_complete = on_io_open_complete;
            tls_io_instance->on_io_open_complete_callback = on_io_open_complete_callback;
			
			if (tls_io_instance->certificate != NULL)
			{
				/* TODO: Replace this with the call to your library that sets the trusted certificates */
				if (tlsSetTrustedCaList(tls_io_instance->tlsContext, tls_io_instance->certificate, strlen(tls_io_instance->certificate)))
				{
					is_error = 1;
				}
			}

			if (is_error != 0)
			{
				LogError("Setting the trusted certificates failed");
				result = __LINE__;
			}
			else
			{
				/* TODO: Call here the function that kicks off the TLS connection and handshake for your TLS library. Note that this might be a multi step process in which case
				you will need to make more function calls here
				if (tlsConnect(tls_io_instance->tlsContext))) */
				{
					LogError("tlsConnect failed");
					result = __LINE__;
				}
				else
				{
					/* TODO: setting the state to OPEN here is the way to go for a blocking connect.
					If your TLS library is non-blocking you will most likely need to implement a callback that is called by your TLS library and within that callback execute 
					the below code that sets the state to OPEN and notifies the IO consumer or it. */
					tls_io_instance->tlsio_state = TLSIO_STATE_OPEN;
					on_io_open_complete(on_io_open_complete_context, IO_OPEN_OK);

					result = 0;
				}
            }
        }
    }

    return result;
}

static int tlsio_template_close(CONCRETE_IO_HANDLE tls_io, ON_IO_CLOSE_COMPLETE on_io_close_complete, void* on_io_close_complete_context)
{
    int result = 0;

    if (tls_io == NULL)
    {
        result = __LINE__;
        LogError("NULL tls_io.");
    }
    else
    {
        TLS_IO_INSTANCE* tls_io_instance = (TLS_IO_INSTANCE*)tls_io;

        /* If we're not open do not try to close */
        if (tls_io_instance->tlsio_state == TLSIO_STATE_NOT_OPEN)
        {
            result = __LINE__;
            LogError("Invalid tlsio_state. Expected state is TLSIO_STATE_NOT_OPEN or TLSIO_STATE_CLOSING.");
        }
        else
        {
            /* TODO: Here you should call the function for your TLS library that shuts down the connection. 
            if (TlsShutdown(tls_io_instance->tlsContext))*/
            {
                LogError("Shutting down TLS connection failed\r\n");
                result = __LINE__;
            }
            else
            {
                tlsio_template_socket_destroy(tls_io_instance->socket);
                tls_io_instance->socket = (TlsSocket)NULL;
                tls_io_instance->tlsio_state = TLSIO_STATE_NOT_OPEN;

				/* trigger the callback and return */
                if (on_io_close_complete != NULL)
                {
                    on_io_close_complete(on_io_close_complete_context);
                }

                result = 0;
            }
        }
    }

    return result;
}

static int tlsio_template_send(CONCRETE_IO_HANDLE tls_io, const void* buffer, size_t size, ON_SEND_COMPLETE on_send_complete, void* on_send_complete_context)
{
    int result;

    if ((tls_io == NULL) ||
        (buffer == NULL) ||
        (size == 0))
    {
        result = __LINE__;
        LogError("NULL tls_io.");
    }
    else
    {
        TLS_IO_INSTANCE* tls_io_instance = (TLS_IO_INSTANCE*)tls_io;

		/* If we are not open, do not try to send */
        if (tls_io_instance->tlsio_state != TLSIO_STATE_OPEN)
        {
            result = __LINE__;
            LogError("Invalid tlsio_state. Expected state is TLSIO_STATE_OPEN.");
        }
        else
        {
            /* TODO: Call here the TLS library function to encrypt the bytes and write them to the socket. Replace the TlsWrite call with your own function.
            if (TlsWrite(tls_io_instance->tlsContext, buffer, size, ...) != 0) */
            {
                result = __LINE__;
                LogError("TLS library failed to encrypt bytes.");
            }
            else
            {
				/* TODO: this assumes that all writes are blocking and no buffering is needed. If buffering is needed you would have to implement additional code here. */
                if (on_send_complete != NULL)
                {
                    on_send_complete(on_send_complete_context, IO_SEND_OK);
                }

                result = 0;
            }
        }
    }

    return result;
}

static void tlsio_template_dowork(CONCRETE_IO_HANDLE tls_io)
{
	/* check arguments */
    if (tls_io == NULL)
    {
        LogError("NULL tls_io.");
    }
    else
    {
        TLS_IO_INSTANCE* tls_io_instance = (TLS_IO_INSTANCE*)tls_io;

		/* only perform work if we are not in error */
        if ((tls_io_instance->tlsio_state != TLSIO_STATE_NOT_OPEN) &&
            (tls_io_instance->tlsio_state != TLSIO_STATE_ERROR))
        {
            unsigned char buffer[64];

            size_t received;
			
			/* TODO: Some TLS libraries might require that you trigger them to do their work here (send outstanding bytes, process incoming bytes, etc. )... */

			/* TODO: call the TLS library to read some decrypted bytes. Replace the TlsRead call with your own TLS library's function. 
            if (tlsRead(tls_io_instance->tlsContext, buffer, sizeof(buffer), &received, ...) != 0) */
            {
                LogError("Error received bytes");

				/* mark state as error and indicate it to the upper layer */
                tls_io_instance->tlsio_state = TLSIO_STATE_ERROR;
                tls_io_instance->on_io_error(tls_io_instance->on_io_error_context);
            }
            else
            {
                if (received > 0)
                {
                    /* if bytes have been received indicate them */
                    tls_io_instance->on_bytes_received(tls_io_instance->on_bytes_received_context, buffer, received);
                }
            }
        }
    }
}

static int tlsio_template_setoption(CONCRETE_IO_HANDLE tls_io, const char* optionName, const void* value)
{
    int result;

	/* check arguments */
    if ((tls_io == NULL) || (optionName == NULL))
    {
        LogError("NULL tls_io");
        result = __LINE__;
    }
    else
    {
        TLS_IO_INSTANCE* tls_io_instance = (TLS_IO_INSTANCE*)tls_io;

		/* TODO: This only handles TrustedCerts, if you need to handle more options specific to your TLS library, fill the code in here

        if (strcmp(name, "...my_option...") == 0)
        {
			// ... store the option value as needed. Make sure that you make a copy when storing.
		}
		else
		*/
        if (strcmp("TrustedCerts", optionName) == 0)
        {
            const char* cert = (const char*)value;

            if (tls_io_instance->certificate != NULL)
            {
                // Free the memory if it has been previously allocated
                free(tls_io_instance->certificate);
                tls_io_instance->certificate = NULL;
            }

            if (cert == NULL)
            {
                result = 0;
            }
            else
            {
                // Store the certificate
                if (mallocAndStrcpy_s(&tls_io_instance->certificate, cert) != 0)
                {
                    LogError("Error allocating memory for certificates");
                    result = __LINE__;
                }
                else
                {
                    result = 0;
                }
            }
        }
        else
        {
            LogError("Unrecognized option");
            result = __LINE__;
        }
    }

    return result;
}

static OPTIONHANDLER_HANDLE tlsio_template_retrieve_options(CONCRETE_IO_HANDLE handle)
{
    OPTIONHANDLER_HANDLE result;

    /* Codes_SRS_tlsio_template_01_064: [ If parameter handle is `NULL` then `tlsio_template_retrieve_options` shall fail and return NULL. ]*/
    if (handle == NULL)
    {
        LogError("invalid parameter detected: CONCRETE_IO_HANDLE handle=%p", handle);
        result = NULL;
    }
    else
    {
        /* Codes_SRS_tlsio_template_01_065: [ `tlsio_template_retrieve_options` shall produce an OPTIONHANDLER_HANDLE. ]*/
        result = OptionHandler_Create(tlsio_template_clone_option, tlsio_template_destroy_option, tlsio_template_setoption);
        if (result == NULL)
        {
            /* Codes_SRS_tlsio_template_01_068: [ If producing the OPTIONHANDLER_HANDLE fails then tlsio_template_retrieve_options shall fail and return NULL. ]*/
            LogError("unable to OptionHandler_Create");
            /*return as is*/
        }
        else
        {
            TLS_IO_INSTANCE* tls_io_instance = (TLS_IO_INSTANCE*)handle;

            /* Codes_SRS_tlsio_template_01_066: [ `tlsio_template_retrieve_options` shall add to it the options: ]*/
            if (
                (tls_io_instance->certificate != NULL) &&
				/* TODO: This only handles TrustedCerts, if you need to handle more options specific to your TLS library, fill the code in here

                (OptionHandler_AddOption(result, "my_option", tls_io_instance->...) != 0) ||
				*/
                (OptionHandler_AddOption(result, "TrustedCerts", tls_io_instance->certificate) != 0)
                )
            {
                LogError("unable to save TrustedCerts option");
                OptionHandler_Destroy(result);
                result = NULL;
            }
            else
            {
                /*all is fine, all interesting options have been saved*/
                /*return as is*/
            }
        }
    }
    return result;
}

static const IO_INTERFACE_DESCRIPTION tlsio_template_interface_description =
{
    tlsio_template_retrieve_options,
    tlsio_template_create,
    tlsio_template_destroy,
    tlsio_template_open,
    tlsio_template_close,
    tlsio_template_send,
    tlsio_template_dowork,
    tlsio_template_setoption
};

/* This simply returns the concrete implementations for the TLS adapter */
const IO_INTERFACE_DESCRIPTION* tlsio_template_get_interface_description(void)
{
    return &tlsio_template_interface_description;
}
