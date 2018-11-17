// Copyright (c) Microsoft. All rights reserved.

#include "testrunnerswitcher.h"

int main(void)
{
    size_t failedTestCount = 0;
    RUN_TEST_SUITE(constbuffer_array_unittests, failedTestCount);

#ifdef VLD_OPT_REPORT_TO_STDOUT
    failedTestCount = VLDGetLeaksCount() > 0 ? 1 : 0;
#endif

    return failedTestCount;
}
