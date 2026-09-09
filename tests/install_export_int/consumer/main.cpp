// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

// Compiling this public header as C++ verifies the imported target's CXX usage
// requirements independently from the C consumer in main.c.
#include "azure_c_shared_utility/xlogging.h"

int main()
{
    return 0;
}