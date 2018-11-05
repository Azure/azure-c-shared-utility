// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "azure_c_shared_utility/xlogging.h"
#include "azure_c_shared_utility/platform.h"

#include <stdlib.h>
#include <unistd.h>
#include <sys/utsname.h>

#include "tlsio_appleios.h"
#include <string.h>

int platform_init(void)
{
    return 0;
}

const IO_INTERFACE_DESCRIPTION* platform_get_default_tlsio(void)
{
    return tlsio_appleios_get_interface_description();
}

STRING_HANDLE platform_get_platform_info(void)
{
    STRING_HANDLE result;
    struct utsname nnn;

    if (uname(&nnn) == 0)
    {
        result = STRING_construct_sprintf("(%s; %s)", nnn.sysname, nnn.machine);
        
        if (result == NULL)
        {
            LogInfo("ERROR: Failed to create machine info string");
        }
    }
    else
    {
        LogInfo("WARNING: failed to find machine info.");
        result = STRING_construct("iOS");

		if (result == NULL)
		{
			LogInfo("ERROR: Failed to create machine info string");
		}
    }

    return result;
}

void platform_deinit(void)
{
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
        strcpy(proxyHostPort2, proxyHostPort);
    else
        proxyHostPort2[0] = '\0';

    if (proxyUsernamePassword)
        strcpy(proxyUserPassword2, proxyUsernamePassword);
    else
        proxyUserPassword2[0] = '\0';
}
