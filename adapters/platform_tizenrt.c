// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdlib.h>
#include "azure_c_shared_utility/platform.h"
#include "azure_c_shared_utility/tlsio_mbedtls.h"

int platform_init(void)
{
	return 0;
}

const IO_INTERFACE_DESCRIPTION* platform_get_default_tlsio(void)
{
	return tlsio_mbedtls_get_interface_description();
}

STRING_HANDLE platform_get_platform_info(void)
{
    // Expected format: "(<runtime name>; <operating system name>; <platform>)"

    return STRING_construct("(native; tizenrt; undefined)");
}

void platform_deinit(void)
{
	return;
}

static char proxyHostPort2[256] = { 0, };
static char proxyUserPassword2[256] = { 0, };

void platform_get_http_proxy(const char** proxyHostnamePort, const char** usernamePassword)
{
    if (proxyHostnamePort)
        *proxyHostnamePort = &proxyHostPort2[0];

    if (usernamePassword)
        *usernamePassword = &proxyUserPassword2[0];
}

void platform_set_http_proxy(const char* proxyHostPort, const char* proxyUsernamePassword)
{
    if (proxyHostPort)
        sprintf_s(proxyHostPort2, sizeof(proxyHostPort2), "%s", proxyHostPort);
    else
        proxyHostPort2[0] = '\0';

    if (proxyUsernamePassword)
        sprintf_s(proxyUserPassword2, sizeof(proxyUserPassword2), "%s", proxyUsernamePassword);
    else
        proxyUserPassword2[0] = '\0';
}
