// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "testrunnerswitcher.h"

int main(void)
{
    size_t failedTestCount = 0;
    RUN_TEST_SUITE(memory_data_ut, failedTestCount);

#ifdef VLD_OPT_REPORT_TO_STDOUT
    failedTestCount = (failedTestCount>0)?failedTestCount:-(int)VLDGetLeaksCount();
#endif

    return failedTestCount;
}
