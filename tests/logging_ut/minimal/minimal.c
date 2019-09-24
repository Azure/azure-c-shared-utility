// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "azure_c_shared_utility/xlogging.h"

int main(void)
{

    LogError("some int has value %d", 42);

    LogInfo("some other int has value %d", 0x42);

    LogLastError("nothing is expected, but here's a parameter of type int = %d", 3);

    return 0;
}
