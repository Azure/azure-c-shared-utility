// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "testrunnerswitcher.h"
#include "tx_api.h"

/* Define the helper thread for running Azure SDK on ThreadX (THREADX IoT Platform).  */
#ifndef THREADX_AZURE_SDK_HELPER_THREAD_STACK_SIZE
#define THREADX_AZURE_SDK_HELPER_THREAD_STACK_SIZE      (2048)
#endif /* THREADX_AZURE_SDK_HELPER_THREAD_STACK_SIZE  */

#ifndef THREADX_AZURE_SDK_HELPER_THREAD_PRIORITY
#define THREADX_AZURE_SDK_HELPER_THREAD_PRIORITY        (4)
#endif /* THREADX_AZURE_SDK_HELPER_THREAD_PRIORITY  */

/* Define the memory area for helper thread.  */
UCHAR threadx_azure_sdk_helper_thread_stack[THREADX_AZURE_SDK_HELPER_THREAD_STACK_SIZE];

/* Define the prototypes for helper thread.  */
TX_THREAD threadx_azure_sdk_helper_thread;
void threadx_azure_sdk_helper_thread_entry(ULONG parameter);
extern void threadx_azure_sdk_initialize(void);

int main(void)
{

    /* Enter the ThreadX kernel.  */
    tx_kernel_enter();
}


/* Define what the initial system looks like.  */
void tx_application_define(void *first_unused_memory)
{

UINT  status;

    /* Create THREADX Azure SDK helper thread. */
    status = tx_thread_create(&threadx_azure_sdk_helper_thread, "THREADX Azure SDK Help Thread",
                     threadx_azure_sdk_helper_thread_entry, 0,
                     threadx_azure_sdk_helper_thread_stack, THREADX_AZURE_SDK_HELPER_THREAD_STACK_SIZE,
                     THREADX_AZURE_SDK_HELPER_THREAD_PRIORITY, THREADX_AZURE_SDK_HELPER_THREAD_PRIORITY, 
                     TX_NO_TIME_SLICE, TX_AUTO_START);    
    
    /* Check status.  */
    if (status)
        printf("THREADX Azure SDK Helper Thread Create Fail.\r\n");
}

/* Define THREADX Azure SDK helper thread entry.  */
void threadx_azure_sdk_helper_thread_entry(ULONG parameter)
{

    size_t failedTestCount = 0;
    RUN_TEST_SUITE(tickcounter_unittests, failedTestCount);
}

