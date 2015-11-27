# Azure C Shared Utility

azure-c-shared-utility which is a C library provising common functionality for basic tasks (like string, list manipulation, IO, etc.).
azure-c-shared-utility is available here: https://github.com/Azure/azure-c-shared-utility.

## Dependencies

azure-c-shared-utility provides 3 tlsio implementations:
- tlsio_schannel - runs only on Windows
- tlsio_openssl - depends on OpenSSL being installed
- tlsio_wolfssl - depends on WolfSSL being installed 

azure-c-shared-utility depends on curl for its HTTPAPI.

azure-c-shared-utility uses cmake for configuring build files.

## Setup

1. Clone azure-c-shared-utility

2.
Create a folder build under azure-uamqp-c

3.
Switch to the build folder and run
cmake ..

## Configuration options

In order to turn on/off the tlsio implementations use the following CMAKE options:

-Duse_schannel:bool={ON/OFF} - turns on/off the SChannel support
-Duse_openssl:bool={ON/OFF} - turns on/off the OpenSSL support. If this option is use an environment variable name OpenSSLDir should be set to point to the OpenSSL folder.
-Duse_wolfssl:bool={ON/OFF} - turns on/off the WolfSSL support. If this option is use an environment variable name WolfSSLDir should be set to point to the WolfSSL folder.
-Duse_http:bool={ON/OFF} - turns on/off the HTTP API support.
