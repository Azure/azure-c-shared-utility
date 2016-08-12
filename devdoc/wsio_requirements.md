# wsio requirements
 
## Overview

wsio is module that implements a concrete IO that transports data over WebSockets using libwebsockets.

## Exposed API

```c
typedef struct WSIO_CONFIG_TAG
{
	const char* host;
	int port;
	const char* protocol_name;
	const char* relative_path;
	bool use_ssl;
	const char* trusted_ca;
} WSIO_CONFIG;

static wsio_retrieveoptions(CONCRETE_IO_HANDLE handle);
extern CONCRETE_IO_HANDLE wsio_create(void* io_create_parameters);
extern void wsio_destroy(CONCRETE_IO_HANDLE ws_io);
extern int wsio_open(CONCRETE_IO_HANDLE ws_io, ON_IO_OPEN_COMPLETE on_io_open_complete, ON_BYTES_RECEIVED on_bytes_received, ON_IO_ERROR on_io_error, void* callback_context);
extern int wsio_close(CONCRETE_IO_HANDLE ws_io, ON_IO_CLOSE_COMPLETE on_io_close_complete, void* callback_context);
extern int wsio_send(CONCRETE_IO_HANDLE ws_io, const void* buffer, size_t size, ON_SEND_COMPLETE on_send_complete, void* callback_context);
extern void wsio_dowork(CONCRETE_IO_HANDLE ws_io);
extern int wsio_setoption(CONCRETE_IO_HANDLE socket_io, const char* optionName, const void* value);

extern const IO_INTERFACE_DESCRIPTION* wsio_get_interface_description(void);
```

### wsio_create

```c
extern CONCRETE_IO_HANDLE wsio_create(void* io_create_parameters);
```

**SRS_WSIO_01_001: \[**wsio_create shall create an instance of a wsio and return a non-NULL handle to it.**\]**
**SRS_WSIO_01_002: \[**If the argument io_create_parameters is NULL then wsio_create shall return NULL.**\]**
**SRS_WSIO_01_003: \[**io_create_parameters shall be used as a WSIO_CONFIG* .**\]**
**SRS_WSIO_01_004: \[**If any of the WSIO_CONFIG fields host, protocol_name or relative_path is NULL then wsio_create shall return NULL.**\]**
**SRS_WSIO_01_100: \[**The trusted_ca member shall be optional (it can be NULL).**\]**
**SRS_WSIO_01_005: \[**If allocating memory for the new wsio instance fails then wsio_create shall return NULL.**\]**
**SRS_WSIO_01_006: \[**The members host, protocol_name, relative_path and trusted_ca shall be copied for later use (they are needed when the IO is opened).**\]** 
**SRS_WSIO_01_098: \[**wsio_create shall create a pending IO list that is to be used when sending buffers over the libwebsockets IO by calling list_create.**\]** 
**SRS_WSIO_01_099: \[**If list_create fails then wsio_create shall fail and return NULL.**\]** 

### wsio_destroy

```c
extern void wsio_destroy(CONCRETE_IO_HANDLE ws_io);
```

**SRS_WSIO_01_007: \[**wsio_destroy shall free all resources associated with the wsio instance.**\]**
**SRS_WSIO_01_008: \[**If ws_io is NULL, wsio_destroy shall do nothing.**\]** 
**SRS_WSIO_01_009: \[**wsio_destroy shall execute a close action if the IO has already been open or an open action is already pending.**\]** 

### wsio_open

```c
extern int wsio_open(CONCRETE_IO_HANDLE ws_io, ON_IO_OPEN_COMPLETE on_io_open_complete, ON_BYTES_RECEIVED on_bytes_received, ON_IO_ERROR on_io_error, void* callback_context);
```

**SRS_WSIO_01_010: \[**wsio_open shall create a context for the libwebsockets connection by calling lws_create_context.**\]**
**SRS_WSIO_01_104: \[**On success, wsio_open shall return 0.**\]**
**SRS_WSIO_01_011: \[**The port member of the info argument shall be set to CONTEXT_PORT_NO_LISTEN.**\]**
**SRS_WSIO_01_091: \[**The extensions field shall be set to the internal extensions obtained by calling lws_get_internal_extensions.**\]**
**SRS_WSIO_01_092: \[**gid and uid shall be set to -1.**\]**
**SRS_WSIO_01_093: \[**The members iface, token_limits, ssl_cert_filepath, ssl_private_key_filepath, ssl_private_key_password, ssl_ca_filepath, ssl_cipher_list and provided_client_ssl_ctx shall be set to NULL.**\]**
**SRS_WSIO_01_094: \[**No proxy support shall be implemented, thus setting http_proxy_address to NULL.**\]**
**SRS_WSIO_01_095: \[**The member options shall be set to 0.**\]**
**SRS_WSIO_01_096: \[**The member user shall be set to a user context that will be later passed by the libwebsockets callbacks.**\]**
**SRS_WSIO_01_097: \[**Keep alive shall not be supported, thus ka_time shall be set to 0.**\]**
**SRS_WSIO_01_012: \[**The protocols member shall be populated with 2 protocol entries, one containing the actual protocol to be used and one empty (fields shall be NULL or 0).**\]**

The first protocol entry shall have:
-	**SRS_WSIO_01_013: \[**callback shall be set to a callback used by the wsio module to listen to libwebsockets events.**\]** 
-	**SRS_WSIO_01_014: \[**id shall be set to 0**\]** 
-	**SRS_WSIO_01_015: \[**name shall be set to protocol_name as passed to wsio_create**\]** 
-	**SRS_WSIO_01_016: \[**per_session_data_size shall be set to 0**\]** 
-	**SRS_WSIO_01_017: \[**rx_buffer_size shall be set to 0, as there is no need for atomic frames**\]** 
-	**SRS_WSIO_01_019: \[**user shall be set to NULL**\]** 

**SRS_WSIO_01_022: \[**If creating the context fails then wsio_open shall fail and return a non-zero value.**\]** 
**SRS_WSIO_01_023: \[**wsio_open shall trigger the libwebsocket connect by calling lws_client_connect and passing to it the following arguments**\]**:
-	**SRS_WSIO_01_024: \[**clients shall be the context created earlier in wsio_open**\]** 
-	**SRS_WSIO_01_025: \[**address shall be the hostname passed to wsio_create**\]** 
-	**SRS_WSIO_01_026: \[**port shall be the port passed to wsio_create**\]** 
-	**SRS_WSIO_01_027: \[**if use_ssl passed in wsio_create is true, the use_ssl argument shall be 1**\]**; **SRS_WSIO_01_103: \[**otherwise it shall be 0.**\]** 
-	**SRS_WSIO_01_028: \[**path shall be the relative_path passed in wsio_create**\]** 
-	**SRS_WSIO_01_029: \[**host shall be the host passed to wsio_create**\]** 
-	**SRS_WSIO_01_030: \[**origin shall be the host passed to wsio_create**\]** 
-	**SRS_WSIO_01_031: \[**protocol shall be the protocol_name passed to wsio_create**\]** 
-	**SRS_WSIO_01_032: \[**ietf_version_or_minus_one shall be -1**\]** 

**SRS_WSIO_01_033: \[**If lws_client_connect fails then wsio_open shall fail and return a non-zero value.**\]** 
**SRS_WSIO_01_034: \[**If another open is in progress or has completed successfully (the IO is open), wsio_open shall fail and return a non-zero value without performing any connection related activities.**\]**
**SRS_WSIO_01_035: \[**If a close action is in progress, wsio_open shall fail and return a non-zero value without performing any connection related activities.**\]** 
**SRS_WSIO_01_036: \[**The callback on_io_open_complete shall be called with io_open_result being set to IO_OPEN_OK when the open action is succesfull.**\]**
**SRS_WSIO_01_037: \[**If any error occurs while the open action is in progress, the callback on_io_open_complete shall be called with io_open_result being set to IO_OPEN_ERROR.**\]**
**SRS_WSIO_01_038: \[**If wsio_close is called while the open action is in progress, the callback on_io_open_complete shall be called with io_open_result being set to IO_OPEN_CANCELLED and then the wsio_close shall proceed to close the IO.**\]**
**SRS_WSIO_01_039: \[**The callback_context argument shall be passed to on_io_open_complete as is.**\]** 
**SRS_WSIO_01_040: \[**The argument on_io_open_complete shall be optional, if NULL is passed by the caller then no open complete callback shall be triggered.**\]** 

### wsio_close

```c
extern int wsio_close(CONCRETE_IO_HANDLE ws_io, ON_IO_CLOSE_COMPLETE on_io_close_complete, void* callback_context);
```

**SRS_WSIO_01_041: \[**wsio_close shall close the websockets IO if an open action is either pending or has completed successfully (if the IO is open).**\]** 
**SRS_WSIO_01_042: \[**if ws_io is NULL, wsio_close shall return a non-zero value.**\]** 
**SRS_WSIO_01_043: \[**wsio_close shall close the connection by calling lws_context_destroy.**\]** **SRS_WSIO_01_044: \[**On success wsio_close shall return 0.**\]** 
**SRS_WSIO_01_045: \[**wsio_close when no open action has been issued shall fail and return a non-zero value.**\]**
**SRS_WSIO_01_046: \[**wsio_close after a wsio_close shall fail and return a non-zero value.**\]** 
**SRS_WSIO_01_047: \[**The callback on_io_close_complete shall be called after the close action has been completed in the context of wsio_close (wsio_close is effectively blocking).**\]**
**SRS_WSIO_01_048: \[**The callback_context argument shall be passed to on_io_close_complete as is.**\]** 
**SRS_WSIO_01_049: \[**The argument on_io_close_complete shall be optional, if NULL is passed by the caller then no close complete callback shall be triggered.**\]** 
**SRS_WSIO_01_108: \[**wsio_close shall obtain all the pending IO items by repetitively querying for the head of the pending IO list and freeing that head item.**\]**
**SRS_WSIO_01_111: \[**Obtaining the head of the pending IO list shall be done by calling list_get_head_item.**\]**
**SRS_WSIO_01_109: \[**For each pending item the send complete callback shall be called with IO_SEND_CANCELLED.**\]**
**SRS_WSIO_01_110: \[**The callback context passed to the on_send_complete callback shall be the context given to wsio_send.**\]** 

### wsio_send

```c
extern int wsio_send(CONCRETE_IO_HANDLE ws_io, const void* buffer, size_t size, ON_SEND_COMPLETE on_send_complete, void* callback_context);
```

**SRS_WSIO_01_050: \[**wsio_send shall queue the buffer bytes for sending through the websockets connection.**\]**
**SRS_WSIO_01_107: \[**On success, wsio_send shall return 0.**\]**
**SRS_WSIO_01_051: \[**If the wsio is not OPEN (open has not been called or is still in progress) then wsio_send shall fail and return a non-zero value.**\]**
**SRS_WSIO_01_052: \[**If any of the arguments ws_io or buffer are NULL, wsio_send shall fail and return a non-zero value.**\]**
**SRS_WSIO_01_053: \[**If size is zero then wsio_send shall fail and return a non-zero value.**\]**
**SRS_WSIO_01_054: \[**wsio_send shall queue the buffer and size until the libwebsockets callback is invoked with the event LWS_CALLBACK_CLIENT_WRITEABLE.**\]**
**SRS_WSIO_01_105: \[**The data and callback shall be queued by calling list_add on the list created in wsio_create.**\]**
**SRS_WSIO_01_055: \[**If queueing the data fails (i.e. due to insufficient memory), wsio_send shall fail and return a non-zero value.**\]**
**SRS_WSIO_01_056: \[**After queueing the data, wsio_send shall call lws_callback_on_writable, while passing as arguments the websockets instance previously obtained in wsio_open from lws_client_connect.**\]**
**SRS_WSIO_01_106: \[**If lws_callback_on_writable returns a negative value, wsio_send shall fail and return a non-zero value.**\]** 
**SRS_WSIO_01_057: \[**The callback on_send_complete shall be called with SEND_RESULT_OK when the send is indicated as complete.**\]**
**SRS_WSIO_01_059: \[**The callback_context argument shall be passed to on_send_complete as is.**\]** 
**SRS_WSIO_01_060: \[**The argument on_send_complete shall be optional, if NULL is passed by the caller then no send complete callback shall be triggered.**\]** 

### wsio_dowork

```c
extern void wsio_dowork(CONCRETE_IO_HANDLE ws_io);
```

**SRS_WSIO_01_061: \[**wsio_dowork shall service the libwebsockets context by calling lws_service and passing as argument the context obtained in wsio_open.**\]**
**SRS_WSIO_01_112: \[**The timeout for lws_service shall be 0.**\]** **SRS_WSIO_01_062: \[**This shall be done if the IO is not closed.**\]** 
**SRS_WSIO_01_063: \[**If the ws_io argument is NULL, wsio_dowork shall do nothing.**\]** 

### wsio_setoption

```c
extern int wsio_setoption(CONCRETE_IO_HANDLE socket_io, const char* optionName, const void* value);
```

**SRS_WSIO_03_001: \[**wsio_setoption does not support any options and shall always return non-zero value.**\]**

### wsio_get_interface_description

```c
extern const IO_INTERFACE_DESCRIPTION* wsio_get_interface_description(void);
```

**SRS_WSIO_01_064: \[**wsio_get_interface_description shall return a pointer to an IO_INTERFACE_DESCRIPTION structure that contains pointers to the functions: wsio_retrieveoptions, wsio_create, wsio_destroy, wsio_open, wsio_close, wsio_send and wsio_dowork.**\]** 

### on_ws_callback

The on_ws_callback is to be invoked by the libwebsockets library whenever a libwebsockets event occurs. 

#### LWS_CALLBACK_CLIENT_ESTABLISHED

**SRS_WSIO_01_066: \[**If an open action is pending, the on_io_open_complete callback shall be triggered with IO_OPEN_OK and from now on it shall be possible to send/receive data.**\]**
**SRS_WSIO_01_068: \[**If the IO is already open, the on_io_error callback shall be triggered.**\]** 

#### LWS_CALLBACK_CLIENT_CONNECTION_ERROR

**SRS_WSIO_01_069: \[**If an open action is pending, the on_io_open_complete callback shall be triggered with IO_OPEN_ERROR.**\]**
**SRS_WSIO_01_070: \[**If the IO is already open, the on_io_error callback shall be triggered.**\]** 

#### LWS_CALLBACK_CLIENT_WRITEABLE

**SRS_WSIO_01_120: \[**This event shall only be processed if the IO is open.**\]**
**SRS_WSIO_01_121: \[**If this event is received in while an open action is incomplete, the open_complete callback shall be called with IO_OPEN_ERROR.**\]** 
**SRS_WSIO_01_071: \[**If any pending IO chunks queued in wsio_send are to be sent, then the first one shall be retrieved from the queue.**\]**
**SRS_WSIO_01_072: \[**Enough space to fit the data and LWS_SEND_BUFFER_PRE_PADDING and LWS_SEND_BUFFER_POST_PADDING shall be allocated.**\]**
**SRS_WSIO_01_073: \[**If allocating the memory fails then the send_result callback callback shall be triggered with IO_SEND_ERROR.**\]**
**SRS_WSIO_01_113: \[**If allocating the memory fails for a pending IO that has been partially sent already then the on_io_error callback shall also be triggered.**\]** 
**SRS_WSIO_01_074: \[**The payload queued in wsio_send shall be copied to the newly allocated buffer at the position LWS_SEND_BUFFER_PRE_PADDING.**\]**
**SRS_WSIO_01_075: \[**lws_write shall be called with the websockets interface obtained in wsio_open, the newly constructed padded buffer, the data size queued in wsio_send (actual payload) and the payload type should be set to LWS_WRITE_BINARY.**\]** 
**SRS_WSIO_01_076: \[**If lws_write fails (result is less than 0) then the send_complete callback shall be triggered with IO_SEND_ERROR.**\]**
**SRS_WSIO_01_114: \[**Additionally, if the failure is for a pending IO that has been partially sent already then the on_io_error callback shall also be triggered.**\]** 
**SRS_WSIO_01_077: \[**If lws_write succeeds and the complete payload has been sent, the queued pending IO shall be removed from the pending list.**\]**
**SRS_WSIO_01_078: \[**If the pending IO had an associated on_send_complete, then the on_send_complete function shall be called with the callback_context and IO_SEND_OK as arguments.**\]** 
**SRS_WSIO_01_079: \[**If the send was successful and any error occurs during removing the pending IO from the list then the on_io_error callback shall be triggered.**\]**
**SRS_WSIO_01_117: \[**on_io_error should not be triggered twice when removing a pending IO that failed and a partial send for it has already been done.**\]** 
**SRS_WSIO_01_080: \[**If lws_write succeeds and less bytes than the complete payload have been sent, then the sent bytes shall be removed from the pending IO and only the leftover bytes shall be left as pending and sent upon subsequent events.**\]** 
**SRS_WSIO_01_118: \[**If lws_write indicates more bytes sent than were passed to it an error shall be indicated via on_send_complete.**\]**
**SRS_WSIO_01_119: \[**If this error happens after the pending IO being partially sent, the on_io_error shall also be indicated.**\]** 
**SRS_WSIO_01_081: \[**If any pending IOs are in the list, lws_callback_on_writable shall be called, while passing the websockets instance obtained in wsio_open as arguments if:**\]** 
-	**SRS_WSIO_01_115: \[**The send over websockets was successful**\]** 
-	**SRS_WSIO_01_116: \[**The send failed writing to lws or allocating memory for the data to be passed to lws and no partial data has been sent previously for the pending IO.**\]** 

#### LWS_CALLBACK_CLIENT_RECEIVE

**SRS_WSIO_01_082: \[**LWS_CALLBACK_CLIENT_RECEIVE shall only be processed when the IO is open.**\]**
**SRS_WSIO_01_122: \[**If an open action is in progress then the on_open_complete callback shall be invoked with IO_OPEN_ERROR.**\]**
**SRS_WSIO_01_083: \[**When LWS_CALLBACK_CLIENT_RECEIVE is triggered and the IO is open, the on_bytes_received callback passed in wsio_open shall be called.**\]**
**SRS_WSIO_01_084: \[**The bytes argument shall point to the received bytes as indicated by the LWS_CALLBACK_CLIENT_RECEIVE in argument.**\]**
**SRS_WSIO_01_085: \[**The length argument shall be set to the number of received bytes as indicated by the LWS_CALLBACK_CLIENT_RECEIVE len argument.**\]**
**SRS_WSIO_01_086: \[**The callback_context shall be set to the callback_context that was passed in wsio_open.**\]** 
**SRS_WSIO_01_087: \[**If the number of bytes is 0 then the on_io_error callback shall be called.**\]** 
**SRS_WSIO_01_088: \[**If the number of bytes received is positive, but the buffer indicated by the in parameter is NULL, then the on_io_error callback shall be called.**\]** 

#### LWS_CALLBACK_OPENSSL_LOAD_EXTRA_CLIENT_VERIFY_CERTS

**SRS_WSIO_01_089: \[**When LWS_CALLBACK_OPENSSL_LOAD_EXTRA_CLIENT_VERIFY_CERTS is triggered, the certificates passed in the trusted_ca member of WSIO_CONFIG passed in wsio_init shall be loaded in the certificate store.**\]**
**SRS_WSIO_01_090: \[**The OpenSSL certificate store is passed in the user argument.**\]**	
Loading the certificates in the certificate store shall be done by:
-	**SRS_WSIO_01_131: \[**Get the certificate store for the OpenSSL context by calling SSL_CTX_get_cert_store**\]** 
-	**SRS_WSIO_01_123: \[**Creating a new BIO by calling BIO_new.**\]** **SRS_WSIO_01_124: \[**The BIO shall be a memory one (obtained by calling BIO_s_mem).**\]** 
-	**SRS_WSIO_01_125: \[**Setting the certificates string as the input by using BIO_puts.**\]** 
-	**SRS_WSIO_01_126: \[**Reading every certificate by calling PEM_read_bio_X509**\]**  **SRS_WSIO_01_132: \[**When PEM_read_bio_X509 returns NULL then no more certificates are available in the input string.**\]** 
-	**SRS_WSIO_01_127: \[**Adding the read certificate to the store by calling X509_STORE_add_cert**\]** **SRS_WSIO_01_133: \[**If X509_STORE_add_cert fails then the certificate obtained by calling PEM_read_bio_X509 shall be freed with X509_free.**\]** 
-	**SRS_WSIO_01_128: \[**Freeing the BIO**\]** 
**SRS_WSIO_01_129: \[**If any of the APIs fails and an open call is pending the on_open_complete callback shall be triggered with IO_OPEN_ERROR.**\]** 
**SRS_WSIO_01_130: \[**If the event is received when the IO is already open the on_io_error callback shall be triggered.**\]** 

### wsio_retrieveoptions
```c
OPTIONHANDLER_HANDLE wsio_retrieveoptions(CONCRETE_IO_HANDLE handle)
```

wsio_retrieveoptions produces an OPTIONHANDLER_HANDLE. Because of SRS_WSIO_03_001, the produced object is empty. 

 **SRS_WSIO_02_001: [** If parameter handle is `NULL` then `wsio_retrieveoptions` shall fail and return NULL. **]**
 **SRS_WSIO_02_002: [** wsio_retrieveoptions shall produce an empty OPTIOHANDLER_HANDLE. **]**
 **SRS_WSIO_02_003: [** If producing the OPTIONHANDLER_HANDLE fails then wsio_retrieveoptions shall fail and return NULL. **]** 