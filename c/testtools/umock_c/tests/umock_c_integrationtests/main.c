// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "testrunnerswitcher.h"

int main(void)
{
    size_t failedTestCount = 0;
    size_t totalFailedTestCount = 0;
    RUN_TEST_SUITE(umock_c_unittests, failedTestCount);
    totalFailedTestCount += failedTestCount;
    RUN_TEST_SUITE(umock_c_generate_function_declaration_unittests, failedTestCount);
    totalFailedTestCount += failedTestCount;
    return totalFailedTestCount;
}
