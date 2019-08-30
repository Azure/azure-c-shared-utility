// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdarg.h>
#include <stdio.h>
#include <time.h>

#include "windows.h"

#include "azure_c_shared_utility/xlogging.h"
#include "azure_c_shared_utility/etwlogger.h"
#include "azure_c_shared_utility/consolelogger.h"

/*returns a string as if printed by vprintf*/
static char* vprintf_alloc(const char* format, va_list va)
{
    char* result;
    int neededSize = vsnprintf(NULL, 0, format, va);
    if (neededSize < 0)
    {
        result = NULL;
    }
    else
    {
        result = (char*)malloc(neededSize + 1);
        if (result == NULL)
        {
            /*return as is*/
        }
        else
        {
            if (vsnprintf(result, neededSize + 1, format, va) != neededSize)
            {
                free(result);
                result = NULL;
            }
        }
    }
    return result;
}

/*returns a string as if printed by printf*/
static char* printf_alloc(const char* format, ...)
{
    char* result;
    va_list va;
    va_start(va, format);
    result = vprintf_alloc(format, va);
    va_end(va);
    return result;
}

/*returns NULL if it fails*/
static char* lastErrorToString(DWORD lastError)
{
    char* result;
    if (lastError == 0)
    {
        result = printf_alloc(""); /*no error should appear*/
        if (result == NULL)
        {
            (void)printf("failure in printf_alloc\r\n");
        }
        else
        {
            /*return as is*/
        }
    }
    else
    {
        char temp[MESSAGE_BUFFER_SIZE];
        if (FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, lastError, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), temp, MESSAGE_BUFFER_SIZE, NULL) == 0)
        {
            result = printf_alloc("GetLastError()=0X%x", lastError);
            if (result == NULL)
            {
                (void)printf("failure in printf_alloc\r\n");
                /*return as is*/
            }
            else
            {
                /*return as is*/
            }
        }
        else
        {
            /*eliminate the \r or \n from the string*/
            /*one replace of each is enough*/
            char* whereAreThey;
            if ((whereAreThey = strchr(temp, '\r')) != NULL)
            {
                *whereAreThey = '\0';
            }
            if ((whereAreThey = strchr(temp, '\n')) != NULL)
            {
                *whereAreThey = '\0';
            }

            result = printf_alloc("GetLastError()==0X%x (%s)", lastError, temp);

            if (result == NULL)
            {
                (void)printf("failure in printf_alloc\r\n");
                /*return as is*/
            }
            else
            {
                /*return as is*/
            }
        }
    }
    return result;
}


static int isETWLoggerInit = 0;

static void lazyRegisterEventProvider(void)
{
    /*lazily init the logger*/
    if (isETWLoggerInit == 0)
    {
        ULONG event_register_result = EventRegisterMicrosoft_ServiceBus();
        if (event_register_result != ERROR_SUCCESS)
        {
            /*go back to printf, maybe somebody notices*/
            (void)printf("failure in EventRegisterMicrosoft_ServiceBus: %lu\r\n", event_register_result);
        }
        else
        {
            /*should check if the provider is registered with the system... maybe at a later time*/
            isETWLoggerInit = 1; /*and stays 1 until the process exits*/ /*sorry, no graceful exit with EventUnregisterMicrosoft_ServiceBus*/
            LogInfo("EventRegisterMicrosoft_ServiceBus success"); /*self logging that the log service has started*/
        }
    }
}

/*the function will also attempt to produce some human readable strings for GetLastError*/
void etwlogger_log_with_GetLastError(const char* file, const char* func, int line, const char* format, ...)
{
    DWORD lastError;
    char* lastErrorAsString;

    lastError = GetLastError(); /*needs to be done before lazRegistedEventProvider*/
    lazyRegisterEventProvider();

    va_list args;
    va_start(args, format);

    SYSTEMTIME t;
    GetSystemTime(&t);

    lastErrorAsString = lastErrorToString(lastError);
    if (lastErrorAsString == NULL)
    {
        char* userMessage = vprintf_alloc(format, args);
        if (userMessage == NULL)
        {
            ULONG event_write_result = EventWriteLogErrorEvent("unable to print user error or last error", file, &t, func, line);
            if (event_write_result != ERROR_SUCCESS)
            {
                (void)printf("failure in EventWriteLogErrorEvent: %lu\r\n", event_write_result);
            }
        }
        else
        {
            ULONG event_write_result = EventWriteLogErrorEvent(userMessage, file, &t, func, line);
            if (event_write_result != ERROR_SUCCESS)
            {
                (void)printf("failure in EventWriteLogErrorEvent: %lu\r\n", event_write_result);
            }
            free(userMessage);
        }
    }
    else
    {
        char* userMessage = vprintf_alloc(format, args);
        if (userMessage == NULL)
        {
            ULONG event_write_result = EventWriteLogErrorEvent(lastErrorAsString, file, &t, func, line);
            if (event_write_result != ERROR_SUCCESS)
            {
                (void)printf("failure in EventWriteLogErrorEvent: %lu\r\n", event_write_result);
            }
        }
        else
        {
            ULONG event_write_result = EventWriteLogLastError(userMessage, file, &t, func, line, lastErrorAsString);
            if (event_write_result != ERROR_SUCCESS)
            {
                (void)printf("failure in EventWriteLogErrorEvent: %lu\r\n", event_write_result);
            }
            free(userMessage);
        }
        free(lastErrorAsString);
    }


    va_end(args);
}

/*the below interlocked variable initially set to 0*/
/* At the first error returned by EventWriteLogInfoEvent or by EventWriteLogErrorEvent a printf will be performed and the variable will be switched to 1. */
/* while the variable is set to "1" no fallback printf will be performed */
/* at the first non-erroneous EventWriteLogInfoEvent or EventWriteLogErrorEvent the printf fallback will be re-enabled */
static volatile LONG stopPrintfing = 0;

void etwlogger_log(LOG_CATEGORY log_category, const char* file, const char* func, int line, unsigned int options, const char* format, ...)
{
    (void)options;

    lazyRegisterEventProvider();

    va_list args;
    va_start(args, format);
    char* text = vprintf_alloc(format, args);
    if (text == NULL)
    {
        switch (log_category)
        {
            case AZ_LOG_INFO:
            {
                ULONG event_write_result = EventWriteLogInfoEvent("INTERNAL LOGGING ERROR: failed in vprintf_alloc");
                if (event_write_result != ERROR_SUCCESS)
                {
                    /*fallback on printf...*/
                    if (InterlockedCompareExchange(&stopPrintfing, 1, 0) == 0)
                    {
                        (void)printf("failed in EventWriteLogInfoEvent: %lu. Further failing calls to EventWriteLogInfoEvent will not result in printf\r\n", event_write_result);
                    }
                }
                else
                {
                    (void)InterlockedExchange(&stopPrintfing, 0);
                }
                break;
            }
            case AZ_LOG_ERROR:
            {
                SYSTEMTIME t;
                GetSystemTime(&t);
                ULONG event_write_result = EventWriteLogErrorEvent("INTERNAL LOGGING ERROR: failed in vprintf_alloc", file, &t, func, line);
                if (event_write_result != ERROR_SUCCESS)
                {
                    /*fallback on printf...*/
                    if (InterlockedCompareExchange(&stopPrintfing, 1, 0) == 0)
                    {
                        (void)printf("failed in EventWriteLogErrorEvent: %lu. Further failing calls to EventWriteLogErrorEvent will not result in printf\r\n", event_write_result);
                    }
                }
                else
                {
                    (void)InterlockedExchange(&stopPrintfing, 0);
                }
                break;
            }
            default:
                break;
        }
    }
    else
    {
#if USE_ETW_AND_CONSOLE
        consolelogger_log(log_category, file, func, line, options, text);
#endif

        switch (log_category)
        {
        case AZ_LOG_INFO:
        {
            ULONG event_write_result = EventWriteLogInfoEvent(text);
            if (event_write_result != ERROR_SUCCESS)
            {
                /*fallback on printf...*/
                if (InterlockedCompareExchange(&stopPrintfing, 1, 0) == 0)
                {
                    (void)printf("failed in EventWriteLogInfoEvent: %lu. Further failing calls to EventWriteLogInfoEvent will not result in printf\r\n", event_write_result);
                }
            }
            else
            {
                (void)InterlockedExchange(&stopPrintfing, 0);
            }
            break;
        }
        case AZ_LOG_ERROR:
        {
            SYSTEMTIME t;
            GetSystemTime(&t);
            ULONG event_write_result = EventWriteLogErrorEvent(text, file, &t, func, line);
            if (event_write_result != ERROR_SUCCESS)
            {
                /*fallback on printf...*/
                if (InterlockedCompareExchange(&stopPrintfing, 1, 0) == 0)
                {
                    (void)printf("failed in EventWriteLogErrorEvent: %lu. Further failing calls to EventWriteLogErrorEvent will not result in printf\r\n", event_write_result);
                }
            }
            else
            {
                (void)InterlockedExchange(&stopPrintfing, 0);
            }
            break;
        }
        default:
            break;
        }
        free(text);
    }
    va_end(args);
}

