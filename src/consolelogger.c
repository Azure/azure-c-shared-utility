// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdarg.h>
#include <stdio.h>
#include <time.h>
#include "azure_c_shared_utility/xlogging.h"
#include "azure_c_shared_utility/consolelogger.h"

#if (defined(_MSC_VER)) && (!(defined WINCE))
#include "windows.h"
#include "tchar.h"


wchar_t *  to_wchar(const char * strA)
{
    wchar_t*  resultW;
    if (strA == NULL)
    {
        return NULL; 
    }
    DWORD dwNum = MultiByteToWideChar(CP_ACP, 0, strA, -1, NULL, 0);
    resultW = (wchar_t*)malloc(dwNum * sizeof(wchar_t));
    if (resultW == NULL)
    {
        return NULL;
    }
    MultiByteToWideChar(CP_ACP, 0, strA, -1, resultW, dwNum);

    return resultW;
}

/*returns a string as if printed by vprintf*/
static wchar_t* vprintf_alloc(const wchar_t* format, va_list va)
{
    wchar_t* result;
    int neededSize = _vsnwprintf(NULL, 0, format, va);
    if (neededSize < 0)
    {
        result = NULL;
    }
    else
    {
        result = (wchar_t*)malloc(neededSize + 1);
        if (result == NULL)
        {
            /*return as is*/
        }
        else
        {
            if (_vsnwprintf(result, neededSize + 1, format, va) != neededSize)
            {
                free(result);
                result = NULL;
            }
        }
    }
    return result;
}

/*returns a string as if printed by printf*/
static wchar_t* printf_alloc(const wchar_t* format, ...)
{
    wchar_t* result = NULL;
    va_list va;
    va_start(va, format);
    result = vprintf_alloc(format, va);
    va_end(va);
    return result;
}

/*returns NULL if it fails*/
static wchar_t* lastErrorToString(DWORD lastError)
{
    wchar_t* result;
    if (lastError == 0)
    {
        result = printf_alloc(L""); /*no error should appear*/
        if (result == NULL)
        {
            (void)wprintf(L"failure in printf_alloc");
        }
        else
        {
            /*return as is*/
        }
    }
    else
    {
        wchar_t temp[MESSAGE_BUFFER_SIZE];
        if (FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, lastError, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), temp, MESSAGE_BUFFER_SIZE, NULL) == 0)
        {
            result = printf_alloc(L"GetLastError()=0X%x", lastError);
            if (result == NULL)
            {
                (void)wprintf(L"failure in printf_alloc\n");
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
            wchar_t* whereAreThey;
            if ((whereAreThey = wcschr(temp, '\r')) != NULL)
            {
                *whereAreThey = '\0';
            }
            if ((whereAreThey = wcschr(temp, '\n')) != NULL)
            {
                *whereAreThey = '\0';
            }

            result = printf_alloc(L"GetLastError()==0X%x (%s)", lastError, temp);

            if (result == NULL)
            {
                (void)wprintf(L"failure in printf_alloc\n");
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
/*this function will use 1x printf (in the happy case) .*/
/*more than 1x printf / function call can mean intermingled LogErrors in a multithreaded env*/
/*the function will also attempt to produce some human readable strings for GetLastError*/
void consolelogger_log_with_GetLastError(const char* file, const char* func, int line, const char* format, ...)
{
	DWORD lastError;
	wchar_t* lastErrorAsString;
	int lastErrorAsString_should_be_freed;
	time_t t;
    int systemMessage_should_be_freed;
	wchar_t* systemMessage;
    int userMessage_should_be_freed;
    wchar_t* userMessage;

    va_list args;
    wchar_t * formatw = to_wchar(format);
    va_start(args, formatw);

    /*this is what this case will do:
    1. snip the last error
    2. create a string with what that last error means
    3. printf the system message (__FILE__, __LINE__ etc) + the last error + whatever the user wanted
    */
    /*1. snip the last error*/
    lastError = GetLastError();

    /*2. create a string with what that last error means*/
    lastErrorAsString = lastErrorToString(lastError);
    if (lastErrorAsString == NULL)
    {
        (void)wprintf(L"failure in lastErrorToString");
        lastErrorAsString = L"";
        lastErrorAsString_should_be_freed = 0;
    }
    else
    {
        lastErrorAsString_should_be_freed = 1;
    }

    _tctime(&t);
    systemMessage = printf_alloc(L"Error: Time:%.24s File:%S Func:%S Line:%d %s", _wctime(&t), file, func, line, lastErrorAsString);

    if (systemMessage == NULL)
    {
        systemMessage = L"";
        (void)wprintf(L"Error: [FAILED] Time:%.24s File : %S Func : %S Line : %d %s", _wctime(&t), file, func, line, lastErrorAsString);
        systemMessage_should_be_freed = 0;
    }
    else
    {
        systemMessage_should_be_freed = 1;
    }

    userMessage = vprintf_alloc(formatw, args);
    if (userMessage == NULL)
    {
        (void)wprintf(L"[FAILED] ");
        (void)vwprintf(formatw, args);
        (void)wprintf(L"\n");
        userMessage_should_be_freed = 0;
    }
    else
    {
        /*3. printf the system message(__FILE__, __LINE__ etc) + the last error + whatever the user wanted*/
        (void)wprintf(L"%s %s\n", systemMessage, userMessage);
        userMessage_should_be_freed = 1;
    }

    if (userMessage_should_be_freed == 1)
    {
        free(userMessage);
    }

    if (systemMessage_should_be_freed == 1)
    {
        free(systemMessage);
    }

    if (lastErrorAsString_should_be_freed == 1)
    {
        free(lastErrorAsString);
    }
    if (formatw)
    {
        free(formatw);
    }
    va_end(args);
}
#endif

#if defined(__GNUC__)
__attribute__ ((format (printf, 6, 7)))
#endif
void consolelogger_log(LOG_CATEGORY log_category, const char* file, const char* func, int line, unsigned int options, const char* format, ...)
{
    time_t t;
    va_list args;

    wchar_t * formatw = to_wchar(format);
    
    va_start(args, formatw);

    t = time(NULL);

    switch (log_category)
    {
    case AZ_LOG_INFO:
        (void)wprintf(L"Info: ");
        break;
    case AZ_LOG_ERROR:
        (void)wprintf(L"Error: Time:%.24s File:%S Func:%S Line:%d ", _wctime(&t), file, func, line);
        break;
    default:
        break;
    }

    (void)vwprintf(formatw, args);
    va_end(args);

    (void)log_category;
    if (options & LOG_LINE)
    {
        (void)wprintf(L"\r\n");
    }

    if (formatw)
    {
        free(formatw);
    }
}


