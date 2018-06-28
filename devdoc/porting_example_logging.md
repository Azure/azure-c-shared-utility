# Optional logging functions for porting

Constrained devices will often want to implement some kind of custom logging.
This file demonstrates an example replacement for the Azure IoT SDK's `xlogging.c`
file. It includes a `LogBinary` function that is useful for troubleshooting,
but would never be used in production.

```c
// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full 
// license information.

#include <stdarg.h>
#include <stdio.h>
#include <time.h>
#include "azure_c_shared_utility/xlogging.h"

#ifndef NO_LOGGING

#ifdef MINIMAL_LOGERROR
void minimal_logerror(const char* file_name, int line_number)
{
    printf("error %s: line %d\n", file_name, line_number);
}

#else // MINIMAL_LOGERROR, meaning that full logging is enabled

#if defined(__GNUC__)
__attribute__ ((format (printf, 6, 7)))
#endif
static void consolelogger_log(LOG_CATEGORY log_category, const char* file, 
    const char* func, int line, unsigned int options, const char* format, ...)
{
    time_t t;
    va_list args;
#ifdef XLOGGING_BUFFER_SIZE
    char buffer[XLOGGING_BUFFER_SIZE];
#endif
    va_start(args, format);

    t = time(NULL);

    switch (log_category)
    {
    case AZ_LOG_INFO:
        (void)printf("Info: ");
        break;
    case AZ_LOG_ERROR:
        (void)printf("Error: Time:%.24s File:%s Func:%s Line:%d ", 
            ctime(&t), file, func, line);
        break;
    default:
        break;
    }

    (void)vprintf(format, args);
    va_end(args);

    (void)log_category;
    if (options & LOG_LINE)
    {
        (void)printf("\r\n");
    }
}

LOGGER_LOG global_log_function = consolelogger_log;

///////////////////////////////////////////////////////////////////////
// Defines and implementation for LogBinary
///////////////////////////////////////////////////////////////////////

/* Print up to 16 bytes per line. */
#define LINE_SIZE 16

/* Return the printable char for the provided value. */
#define PRINTABLE(c) ((c >= ' ') && (c <= '~')) ? (char)c : '.'

/* Convert the lower nibble of the provided byte to a hexadecimal printable char. */
#define HEX_STR(c)   (((c) & 0xF) < 0xA) ? (char)(((c) & 0xF) + '0') : (char)(((c) & 0xF) - 0xA + 'A')

void LogBinary(const char* comment, const void* data, size_t size)
{
    char charBuf[LINE_SIZE + 1];
    char hexBuf[LINE_SIZE * 3 + 1];
    size_t countbuf = 0;
    size_t i = 0;
    const unsigned char* bufAsChar = (const unsigned char*)data;
    const unsigned char* startPos = bufAsChar;

    LOG(AZ_LOG_TRACE, LOG_LINE, "%s     %lu bytes", comment, size);

    /* Print the whole buffer. */
    for (i = 0; i < size; i++)
    {
        /* Store the printable value of the char in the charBuf to print. */
        charBuf[countbuf] = PRINTABLE(*bufAsChar);

        /* Convert the high nibble to a printable hexadecimal value. */
        hexBuf[countbuf * 3] = HEX_STR(*bufAsChar >> 4);

        /* Convert the low nibble to a printable hexadecimal value. */
        hexBuf[countbuf * 3 + 1] = HEX_STR(*bufAsChar);

        hexBuf[countbuf * 3 + 2] = ' ';

        countbuf++;
        bufAsChar++;
        /* If the line is full, print it to start another one. */
        if (countbuf == LINE_SIZE)
        {
            charBuf[countbuf] = '\0';
            hexBuf[countbuf * 3] = '\0';
            LOG(AZ_LOG_TRACE, LOG_LINE, "%p: %s    %s", startPos, hexBuf, charBuf);
            countbuf = 0;
            startPos = bufAsChar;
        }
    }

    /* If the last line does not fit the line size. */
    if (countbuf > 0)
    {
        /* Close the charBuf string. */
        charBuf[countbuf] = '\0';

        /* Fill the hexBuf with spaces to keep the charBuf alignment. */
        while ((countbuf++) < LINE_SIZE - 1)
        {
            hexBuf[countbuf * 3] = ' ';
            hexBuf[countbuf * 3 + 1] = ' ';
            hexBuf[countbuf * 3 + 2] = ' ';
        }
        hexBuf[countbuf * 3] = '\0';

        /* Print the last line. */
        LOG(AZ_LOG_TRACE, LOG_LINE, "%p: %s    %s", startPos, hexBuf, charBuf);
    }
}

#endif // MINIMAL_LOGERROR not defined
#endif // NO_LOGGING not defined
```