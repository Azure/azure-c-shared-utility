// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

// The point of this file is the include style: a consumer of the installed
// package includes the public headers as "azure_c_shared_utility/<header>.h",
// so the include directory exported by the imported target has to be the
// directory that CONTAINS azure_c_shared_utility/. xlogging.h is included
// because it transitively pulls in the c-logging headers, which are installed
// under a different root again.
#include "azure_c_shared_utility/xlogging.h"
#include "azure_c_shared_utility/azure_base64.h"
#include "azure_c_shared_utility/strings.h"

#include <string.h>
#include <stdio.h>

// No logging call and no allocator hook is exercised here: the package must be
// usable without any prior initialization, and the point of the test is the
// include/link contract, not runtime behaviour that the unit tests already cover.
int main(void)
{
    int result = 0;

    STRING_HANDLE encoded = Azure_Base64_Encode_Bytes((const unsigned char*)"azure", 5);
    if (encoded == NULL)
    {
        (void)printf("Azure_Base64_Encode_Bytes failed\n");
        result = 1;
    }
    else
    {
        BUFFER_HANDLE decoded = Azure_Base64_Decode(STRING_c_str(encoded));
        if (decoded == NULL)
        {
            (void)printf("Azure_Base64_Decode failed\n");
            result = 1;
        }
        else
        {
            if ((BUFFER_length(decoded) != 5) ||
                (memcmp(BUFFER_u_char(decoded), "azure", 5) != 0))
            {
                (void)printf("base64 round trip produced unexpected content\n");
                result = 1;
            }

            BUFFER_delete(decoded);
        }

        STRING_delete(encoded);
    }

    if (result == 0)
    {
        (void)printf("installed package consumed successfully\n");
    }

    return result;
}
