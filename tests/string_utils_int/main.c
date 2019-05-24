// Copyright(C) Microsoft Corporation.All rights reserved.

#include <stddef.h>
#include "testrunnerswitcher.h"

int main(void)
{
    size_t failedTestCount = 0;
    RUN_TEST_SUITE(string_utils_int_tests, failedTestCount);

#ifdef VLD_OPT_REPORT_TO_STDOUT
    failedTestCount = (failedTestCount>0)?failedTestCount:-(int)VLDGetLeaksCount();
#endif

    return failedTestCount;
}
