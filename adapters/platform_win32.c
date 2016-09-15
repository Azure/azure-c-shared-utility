// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdlib.h>
#ifdef _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#include "azure_c_shared_utility/platform.h"
#include "azure_c_shared_utility/xio.h"

#include "winsock2.h"

#ifdef USE_OPENSSL
#include "azure_c_shared_utility/tlsio_openssl.h"
#else
#include "azure_c_shared_utility/tlsio_schannel.h"
#endif

int platform_init(void)
{
    int result;

    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        result = __LINE__;
    }
    else
    {
        result = 0;
    }

#ifdef USE_OPENSSL
	tlsio_openssl_init();
#endif
	
	return result;
}

const IO_INTERFACE_DESCRIPTION* platform_get_default_tlsio(void)
{
#ifdef USE_OPENSSL
	return tlsio_openssl_get_interface_description();
#else
#ifndef WINCE
    return tlsio_schannel_get_interface_description();
#else
	LogError("TLS IO interface currently not supported on WEC 2013");
	return (IO_INTERFACE_DESCRIPTION*)NULL;
#endif
#endif
}

void platform_deinit(void)
{
    (void)WSACleanup();

#ifdef USE_OPENSSL
	tlsio_openssl_deinit();
#endif
}
