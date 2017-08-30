// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdlib.h>
#include <stdint.h>
#include "azure_c_shared_utility/uniqueid.h"
#include "azure_c_shared_utility/xlogging.h"
#include <rpc.h>

DEFINE_ENUM_STRINGS(UNIQUEID_RESULT, UNIQUEID_RESULT_VALUES);

UNIQUEID_RESULT UniqueId_Generate(char* uid, size_t len)
{
    UNIQUEID_RESULT result;

    /* Codes_SRS_UNIQUEID_07_002: [If uid is NULL then UniqueId_Generate shall return UNIQUEID_INVALID_ARG] */
    /* Codes_SRS_UNIQUEID_07_003: [If len is less then 37 then UniqueId_Generate shall return UNIQUEID_INVALID_ARG] */
    if (uid == NULL || len < 37)
    {
        result = UNIQUEID_INVALID_ARG;
        LogError("Buffer Size is Null or length is less then 37 bytes");
    }
    else 
    {
        UUID uuidVal;
        RPC_STATUS status = UuidCreate(&uuidVal);
        if (status != RPC_S_OK && status != RPC_S_UUID_LOCAL_ONLY)
        {
            LogError("Unable to aquire unique Id");
            result = UNIQUEID_ERROR;
        }
        else
        {
            unsigned char* randomResult;
            status = UuidToStringA(&uuidVal, &randomResult);
            if (status != RPC_S_OK)
            {
                LogError("Unable to convert Id to string");
                result = UNIQUEID_ERROR;
            }
            else
            {
				size_t cpyLen;
                /* Codes_SRS_UNIQUEID_07_001: [UniqueId_Generate shall create a unique Id 36 character long string.] */
                memset(uid, 0, len);
                cpyLen = strlen((char*)randomResult);
                if (cpyLen > len - 1)
                {
                    cpyLen = len - 1;
                }
                (void)memcpy(uid, randomResult, len);
                RpcStringFreeA(&randomResult);
                result = UNIQUEID_OK;
            }
        }
    }
    return result;
}

UNIQUEID_RESULT UniqueId_GetStringFromBytes(unsigned char* uid_bytes, size_t uuid_size, char* output_string)
{
    UNIQUEID_RESULT result;

    // Codes_SRS_UNIQUEID_09_001: [ If `uid` or `output_string` are NULL, UniqueId_GetStringFromBytes shall return UNIQUEID_INVALID_ARG ]
    // Codes_SRS_UNIQUEID_09_002: [ If `uid_size` is zero or not a multiple of two, UniqueId_GetStringFromBytes shall return UNIQUEID_INVALID_ARG ]
    if (uid_bytes == NULL || uuid_size == 0 || ((uuid_size % 2) != 0) || output_string == NULL)
    {
        LogError("Invalid argument (uid=%p, uuid_size=%d, output_string=%p)", uid_bytes, uuid_size, output_string);
        result = UNIQUEID_INVALID_ARG;
    }
    else
    {
        // Codes_SRS_UNIQUEID_09_003: [ `output_string` shall be filled according to RFC4122 using the byte sequence in `uid` ]
        size_t i, j;
        for (i = 0, j = 0; i < uuid_size; i++, j += 2)
        {
            (void)sprintf(output_string + j, "%02x", uid_bytes[i]);

            if (i == 3 || i == 5 || i == 7 || i == 9)
            {
                output_string[j + 2] = '-';
                j++;
            }
        }

        // Codes_SRS_UNIQUEID_09_004: [ If no failures occur, UniqueId_Generate shall return UNIQUEID_OK ]  
        result = UNIQUEID_OK;
    }

    return result;
}
