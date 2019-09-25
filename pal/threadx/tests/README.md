
# ThreadX

[ThreadX RTOS](https://rtos.com/) is advanced Industrial Grade Real-Time Operating System (RTOS) designed specifically for deeply embedded, real-time, and IoT applications.

## Setup test On Windows

1. Clone **azure-c-shared-utility** test project on threadx using the recursive option:

```
git clone --recursive https://ExpressLogic@dev.azure.com/ExpressLogic/X-Ware/_git/project-X-Ware azure-c-shared-utility-test -b azure-c-shared-utility-test
```

2. Switch to azure-c-shared-utility-test/xware-vs folder.

3. Run update_lib.bat to generate the source code files and test files.

4. Click xware-vs.sln to open and build the projects.

5. Run tests.

Note: If you run platfrom_threadx_ut, socketio_threadx_ut and tlsio_threadx_ut, please make sure that pcap is installed and the
NX_PCAP_SOURCE_NAME is correct in azure-c-shared-utility-test\xware-vs\azure-c-shared-utility-tests\pcap\nx_pcap_network_driver.c.

## C-Utility Tests

### Unit Tests on ThreadX

|test name|test path|result|comment|
|--|--|--|--|
|agenttime_ut|azure-c-shared-utility-xware/tests/|pass||
|azure_base32_ut|azure-c-shared-utility-xware/tests/|pass||
|azure_base64_ut|azure-c-shared-utility-xware/tests/|pass||
|buffer_ut|azure-c-shared-utility-xware/tests/|pass||
|condition_ut|azure-c-shared-utility-xware/tests/|n/a|unsupported yet|
|connectionstringparser_ut|azure-c-shared-utility-xware/tests/|pass||
|constbuffer_array_ut|azure-c-shared-utility-xware/tests/|pass||
|constbuffer_ut|azure-c-shared-utility-xware/tests/|pass||
|constmap_ut|azure-c-shared-utility-xware/tests/|pass||
|crtabstractions_ut|azure-c-shared-utility-xware/tests/|pass||
|dns_async_ut|azure-c-shared-utility-xware/tests/|n/a|unsupported yet|
|doublylinkedlist_ut|azure-c-shared-utility-xware/tests/|pass||
|gballoc_ut|azure-c-shared-utility-xware/tests/|pass||
|gballoc_without_init_ut|azure-c-shared-utility-xware/tests/|pass||
|hmacsha256_ut|azure-c-shared-utility-xware/tests/|pass||
|http_proxy_io_ut|azure-c-shared-utility-xware/tests/|pass||
|httpapicompact_ut|azure-c-shared-utility-xware/tests/|n/a|unsupported yet|
|httpapiex_ut|azure-c-shared-utility-xware/tests/|pass||
|httpapiexsas_ut|azure-c-shared-utility-xware/tests/|pass||
|httpheaders_ut|azure-c-shared-utility-xware/tests/|pass||
|lock_threadx_ut|azure-c-shared-utility-xware/pal/threadx/tests/|pass|only run on threadx platfrom|
|map_ut|azure-c-shared-utility-xware/tests/|pass||
|memory_data_ut|azure-c-shared-utility-xware/tests/|n/a|unsupported yet|
|optionhandler_ut|azure-c-shared-utility-xware/tests/|pass||
|platform_threadx_ut|azure-c-shared-utility-xware/pal/threadx/tests/|pass|only run on threadx platfrom|
|refcount_ut|azure-c-shared-utility-xware/tests/|pass||
|sastoken_ut|azure-c-shared-utility-xware/tests/|pass||
|sha_ut|azure-c-shared-utility-xware/tests/|pass||
|singlylinkedlist_ut|azure-c-shared-utility-xware/tests/|pass||
|socket_async_ut|azure-c-shared-utility-xware/tests/|n/a|unsupported yet|
|socketio_threadx_ut|azure-c-shared-utility-xware/pal/threadx/tests/|pass|only run on threadx platfrom|
|srw_lock_ut|azure-c-shared-utility-xware/tests/|pass||
|string_token_ut|azure-c-shared-utility-xware/tests/|pass||
|string_tokenizer_ut|azure-c-shared-utility-xware/tests/|pass||
|strings_ut|azure-c-shared-utility-xware/tests/|pass||
|template_ut|azure-c-shared-utility-xware/tests/|pass||
|tickcounter_threadx_ut|azure-c-shared-utility-xware/pal/threadx/tests/|pass|only run on threadx platfrom|
|tlsio_option_ut|azure-c-shared-utility-xware/tests/|pass||
|tlsio_threadx_ut|azure-c-shared-utility-xware/pal/threadx/tests/|pass|only run on threadx platfrom|
|uniqueid_ut|azure-c-shared-utility-xware/tests/|n/a|unsupported yet|
|urlencode_ut|azure-c-shared-utility-xware/tests/|pass||
|utf8_checker_ut|azure-c-shared-utility-xware/tests/|pass||
|uuid_ut|azure-c-shared-utility-xware/tests/|pass||
|uws_client_ut|azure-c-shared-utility-xware/tests/|pass||
|uws_frame_encoder_ut|azure-c-shared-utility-xware/tests/|pass||
|vector_ut|azure-c-shared-utility-xware/tests/|pass||
|ws_url_ut|azure-c-shared-utility-xware/tests/|pass||
|wsio_ut|azure-c-shared-utility-xware/tests/|pass||
|x509_threadx_ut|azure-c-shared-utility-xware/tests/|n/a|unsupported yet|
|xio_ut|azure-c-shared-utility-xware/tests/|pass||

### [Detail test report](https://dev.azure.com/ExpressLogic/X-Ware/_git/project-X-Ware?path=%2Ftest_report%2Fc-utility_test_report.docx&version=GBazure-c-shared-utility-test).


## Porting to new devices

Instructions for porting the Azure IoT C SDK to ThreadX devices are located
[here](https://github.com/Azure/azure-c-shared-utility/blob/master/devdoc/porting_guide.md).
