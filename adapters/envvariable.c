// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdlib.h>  
#include <stdio.h>  
#include <string.h>

#include "azure_c_shared_utility/envvariable.h"

const char* environment_get_variable(const char *variable_name)
{
    return getenv(variable_name);
}


static char proxyHostPort2[256] = { 0, };
static char proxyUserPassword2[256] = { 0, };

#ifdef WIN32
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

#else

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
        strncpy(proxyHostPort2, proxyHostPort, sizeof(proxyHostPort2) - 1);
    else
        proxyHostPort2[0] = '\0';

    if (proxyUsernamePassword)
        strncpy(proxyUserPassword2, proxyUsernamePassword, sizeof(proxyUserPassword2) - 1);
    else
        proxyUserPassword2[0] = '\0';
}
#endif
