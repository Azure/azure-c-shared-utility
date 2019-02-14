// Copyright (C) Firmwave Ltd., All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "azure_c_shared_utility/platform.h"
#include "azure_c_shared_utility/xio.h"
#ifdef USE_CYCLONESSL
#include "azure_c_shared_utility/tlsio_cyclonessl.h"
#else
#include "azure_c_shared_utility/tlsio_mbedtls.h"
#endif
#include "azure_c_shared_utility/threadapi.h"

#include "debug.h"

int platform_init(void)
{
    ThreadAPI_Sleep(10000);
    return 0;
}

const IO_INTERFACE_DESCRIPTION* platform_get_default_tlsio(void)
{
#ifdef USE_CYCLONESSL
    return tlsio_cyclonessl_get_interface_description();
#else
    return tlsio_mbedtls_get_interface_description();
#endif
}

STRING_HANDLE platform_get_platform_info(void)
{
    // Expected format: "(<runtime name>; <operating system name>; <platform>)"

    return STRING_construct("(native; freertos; undefined)");
}

void platform_deinit(void)
{
    TRACE_INFO("Deinitializing platform \r\n");
    while(1) {};
}
