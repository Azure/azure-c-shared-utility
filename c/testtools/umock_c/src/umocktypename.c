// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdlib.h>
#ifdef _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#include <ctype.h>
#include <string.h>
#include "umocktypename.h"

char* umocktypename_normalize(const char* type)
{
    size_t length = 0;
    size_t pos = 0;
    char* result;

    /* Codes_SRS_UMOCKTYPES_01_039: [ All extra spaces (more than 1 space between non-space characters) shall be removed. ]*/
    while (type[pos] != '\0')
    {
        if (!((pos > 0) && isspace(type[pos]) && isspace(type[pos - 1])))
        {
            length++;
        }

        pos++;
    }

    result = (char*)malloc(length + 1);
    if (result != NULL)
    {
        pos = 0;
        length = 0;

        while (type[pos] != '\0')
        {
            if (!((pos > 0) && isspace(type[pos]) && isspace(type[pos - 1])))
            {
                result[length] = type[pos];
                length++;
            }

            pos++;
        }

        result[length] = '\0';
    }

    return result;
}
