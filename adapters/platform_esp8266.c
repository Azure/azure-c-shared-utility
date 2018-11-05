// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdlib.h>
#ifdef _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#include "azure_c_shared_utility/platform.h"
#include "azure_c_shared_utility/xio.h"
#include "azure_c_shared_utility/tlsio_openssl.h"
#include "azure_c_shared_utility/xlogging.h"
#include "lwip/apps/sntp.h"
#include "lwip/apps/sntp_time.h"

int platform_init(void)
{
    sntp_init();
    u32_t ts = 0;
    while(ts == 0){
        vTaskDelay(5000 / portTICK_RATE_MS);
        ts = sntp_get_current_timestamp();
        LogInfo("%s", sntp_get_real_time(ts));
    }
    return 0;
}

const IO_INTERFACE_DESCRIPTION* platform_get_default_tlsio(void)
{
    return tlsio_openssl_get_interface_description();
}

void platform_deinit(void)
{
     sntp_stop();
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
