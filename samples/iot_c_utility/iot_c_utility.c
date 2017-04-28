// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "stdio.h"
#include "azure_c_shared_utility/agenttime.h"
#include "azure_c_shared_utility/gballoc.h"
#include "azure_c_shared_utility/http_proxy_io.h"
#include "azure_c_shared_utility/map.h"
#include "azure_c_shared_utility/optimize_size.h"
#include "azure_c_shared_utility/platform.h"
#include "azure_c_shared_utility/sastoken.h"
#include "azure_c_shared_utility/strings.h"
#include "azure_c_shared_utility/urlencode.h"
#include "azure_c_shared_utility/xlogging.h"

void test_http_proxy_io()
{
    const IO_INTERFACE_DESCRIPTION* interface_desc = http_proxy_io_get_interface_description();
    if (interface_desc == NULL)
    {
        (void)printf("Failed to create interface_desc.\n");
    }
}

void show_sastoken_example()
{
    STRING_HANDLE sas_token = SASToken_CreateString("key", "scope", "name", 987654321);
    if (sas_token == NULL)
    {
        (void)printf("Failed to create SAS Token.\n");
    }
    else
    {
        STRING_delete(sas_token);
    }
}

int main(int argc, char** argv)
{
    (void)argc, (void)argv;

    if (platform_init() != 0)
    {
        (void)printf("Cannot initialize platform.\n");
    }
    else
    {
        show_sastoken_example();
        test_http_proxy_io();
        platform_deinit();
    }
    return 0;
}
