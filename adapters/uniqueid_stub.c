// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdlib.h>
#include <stdint.h>
#include "azure_c_shared_utility/uniqueid.h"
#include "azure_c_shared_utility/xlogging.h"
#include <time.h>

DEFINE_ENUM_STRINGS(UNIQUEID_RESULT, UNIQUEID_RESULT_VALUES);

static const char tochar[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
static void generate128BitUUID(unsigned char* arrayOfByte)
{
    size_t arrayIndex;

    for (arrayIndex = 0; arrayIndex < 16; arrayIndex++)
    {
        arrayOfByte[arrayIndex] = (unsigned char)rand();
    }

    //
    // Stick in the version field for random uuid.
    //
    arrayOfByte[7] &= 0x0f; //clear the bit field
    arrayOfByte[7] |= 0x40; //set the ones we care about

    //
    // Stick in the variant field for the random uuid.
    //
    arrayOfByte[8] &= 0xf3; // Clear
    arrayOfByte[8] |= 0x08; // Set

}

// TODO: The User will need to call srand before calling this function
UNIQUEID_RESULT UniqueId_Generate(char* uid, size_t len)
{
    UNIQUEID_RESULT result;
    unsigned char arrayOfChar[16];

    /* Codes_SRS_UNIQUEID_07_002: [If uid is NULL then UniqueId_Generate shall return UNIQUEID_INVALID_ARG] */
    /* Codes_SRS_UNIQUEID_07_003: [If len is less then 37 then UniqueId_Generate shall return UNIQUEID_INVALID_ARG] */
    if (uid == NULL || len < 37)
    {
        result = UNIQUEID_INVALID_ARG;
        LogError("Buffer Size is Null or length is less then 37 bytes");
    }
    else 
    {
        size_t arrayIndex;
        size_t shiftCount;
        size_t characterPosition = 0;

        /* Codes_SRS_UNIQUEID_07_001: [UniqueId_Generate shall create a unique Id 36 character long string.] */
        generate128BitUUID(arrayOfChar);
        for (arrayIndex = 0; arrayIndex < 16; arrayIndex++)
        {
            for (shiftCount = 0; shiftCount <= 1; shiftCount++)
            {
                char hexChar = tochar[arrayOfChar[arrayIndex] & 0xf];
                if ((characterPosition == 8) || (characterPosition == 13) || (characterPosition == 18) || (characterPosition == 23))
                {
                    uid[characterPosition] = '-';
                    characterPosition++;
                }
                uid[characterPosition] = hexChar;
                characterPosition++;
                arrayOfChar[arrayIndex] = arrayOfChar[arrayIndex] >> 4;
            }
        }
        uid[characterPosition] = 0;
        result = UNIQUEID_OK;
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