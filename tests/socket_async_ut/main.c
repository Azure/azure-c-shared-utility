// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "testrunnerswitcher.h"

int main(void)
{
    size_t failedTestCount = 0;
    /**
     * Identify the test suite to run here.
     */
    RUN_TEST_SUITE(socket_async_ut, failedTestCount);

    return failedTestCount;
}
