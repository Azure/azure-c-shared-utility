// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdint.h>
#include <inttypes.h>

#include "azure_c_shared_utility/xlogging.h"

#include "windows.h"
#include "azure_c_shared_utility/etwlogger_driver.h"

#ifndef NO_LOGGING

#ifndef _MSC_VER
#error only supported on Windows
#endif

static LOGGER_LOG global_log_function = etwlogger_log;

#ifdef LOGERROR_CAPTURES_STACK_TRACES
#include "dbghelp.h"
static volatile LONG doSymInit = 0; /*0 = not initialized, 1 = initializing, 2= initialized*/

void* logging_malloc(size_t size)
{
    return malloc(size);
}
void logging_free(void* ptr)
{
    free(ptr);
}

#endif

#if defined _MSC_VER
#ifdef LOGERROR_CAPTURES_STACK_TRACES
#define TRACE_MAX_STACK_FRAMES  1024
#define TRACE_MAX_SYMBOL_SIZE   1024
#define TRACE_MAX_STACK_AS_STRING_SIZE 10*1024
#define TRACE_MAX_STACK_LINE_AS_STRING_SIZE 1024

typedef union SYMBOL_INFO_EXTENDED_TAG
{
    SYMBOL_INFO symbol;
    unsigned char extendsUnion[sizeof(SYMBOL_INFO) + TRACE_MAX_SYMBOL_SIZE - 1]; /*this field only exists to extend the size of the union to encompass "CHAR    Name[1];" of the SYMBOL_INFO to be as big as TRACE_MAX_SYMBOL_SIZE - 1 */ /*and the reason why it is not malloc'd is to exactly avoid a malloc that cannot be LogError'd (how does one log errors in a log function?!)*/
}SYMBOL_INFO_EXTENDED;


static SRWLOCK lockOverSymCalls = SRWLOCK_INIT;
char* getStackAsString(void)
{
    char* result = malloc(TRACE_MAX_STACK_AS_STRING_SIZE);
    if (result == NULL)
    {
        /*return as is!*/
    }
    else
    {
        char* destination = result;
        uint32_t destinationSize = TRACE_MAX_STACK_AS_STRING_SIZE;

        size_t destinationPos = 0;
        destination[0] = '\0';

        /*try fill destination up to destinationSize*/
        char resultLine[TRACE_MAX_STACK_LINE_AS_STRING_SIZE];
        void* stack[TRACE_MAX_STACK_FRAMES];

        /*all following function calls are protected by the same SRW*/
        AcquireSRWLockExclusive(&lockOverSymCalls);

        uint16_t numberOfFrames = CaptureStackBackTrace(1, TRACE_MAX_STACK_FRAMES, stack, NULL);
        HANDLE process = GetCurrentProcess();

        SYMBOL_INFO_EXTENDED symbolExtended;
        SYMBOL_INFO* symbol = &symbolExtended.symbol;

        IMAGEHLP_LINE64 line;

        symbol->MaxNameLen = TRACE_MAX_SYMBOL_SIZE;
        symbol->SizeOfStruct = sizeof(SYMBOL_INFO);

        for (uint32_t j = 0; j < numberOfFrames; j++)
        {
            DWORD64 address = (DWORD64)(stack[j]);
            DWORD displacement = 0;

            if (!SymFromAddr(process, address, NULL, symbol))
            {
                (void)strncat(destination, "SymFromAddr failed\n", destinationSize - destinationPos);
            }
            else
            {
                line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);
                if (SymGetLineFromAddr64(process, address, &displacement, &line))
                {
                    int snprintfResult = snprintf(resultLine, sizeof(resultLine), "!%s %s:%" PRIu32 "\n", symbol->Name, line.FileName, line.LineNumber);
                    if (!(
                        (snprintfResult >= 0) && /*the returned value is nonnegative [...]*/
                        (snprintfResult < sizeof(resultLine)) /*[...] and less than n.*/
                        ))
                    {
                        (void)strncat(destination, "snprintf failed\n", destinationSize - destinationPos);
                    }
                    else
                    {
                        (void)strncat(destination, resultLine, destinationSize - destinationPos);
                    }
                }
                else
                {
                    int snprintfResult = snprintf(resultLine, sizeof(resultLine), "!%s Address 0x%" PRIX64 "\n", symbol->Name, line.Address);
                    if (!(
                        (snprintfResult >= 0) && /*the returned value is nonnegative [...]*/
                        (snprintfResult < sizeof(resultLine)) /*[...] and less than n.*/
                        ))
                    {
                        (void)strncat(destination, "snprintf failed\n", destinationSize - destinationPos);
                    }
                    else
                    {
                        (void)strncat(destination, resultLine, destinationSize - destinationPos);
                    }
                }
            }
            destinationPos = strlen(destination);
        }

        ReleaseSRWLockExclusive(&lockOverSymCalls);

    }
    return result;
}
#endif
#endif


void xlogging_set_log_function(LOGGER_LOG log_function)
{
    global_log_function = log_function;
}

LOGGER_LOG xlogging_get_log_function(void)
{
    return global_log_function;
}

LOGGER_LOG_GETLASTERROR global_log_function_GetLastError = etwlogger_log_with_GetLastError;

void xlogging_set_log_function_GetLastError(LOGGER_LOG_GETLASTERROR log_function_GetLastError)
{
    global_log_function_GetLastError = log_function_GetLastError;
}

LOGGER_LOG_GETLASTERROR xlogging_get_log_function_GetLastError(void)
{
    return global_log_function_GetLastError;
}

void xlogging_LogErrorWinHTTPWithGetLastErrorAsStringFormatter(int errorMessageID)
{
    (void)errorMessageID;
}
#endif
