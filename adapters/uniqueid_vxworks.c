// Copyright (c) 2021 Wind River Systems, Inc.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <vxWorks.h>
#include <uuid.h>

#include "azure_macro_utils/macro_utils.h"
#include "azure_c_shared_utility/uniqueid.h"

MU_DEFINE_ENUM_STRINGS(UNIQUEID_RESULT, UNIQUEID_RESULT_VALUES);

UNIQUEID_RESULT UniqueId_Generate(char* uid, size_t len)
{
    UNIQUEID_RESULT result;

    /* Codes_SRS_UNIQUEID_07_002: [If uid is NULL then UniqueId_Generate shall return UNIQUEID_INVALID_ARG] */
    /* Codes_SRS_UNIQUEID_07_003: [If len is less then 37 then UniqueId_Generate shall return UNIQUEID_INVALID_ARG] */
    if (uid == NULL || len < 37)
    {
        result = UNIQUEID_INVALID_ARG;
    }
    else
    {
        uuid_t uuidVal;
        uint32_t status;
        uuid_create(&uuidVal, &status);

        /* Codes_SRS_UNIQUEID_07_001: [UniqueId_Generate shall create a unique Id 36 character long string.] */
        memset(uid, 0, len);
        uuid_to_string(&uuidVal, uid, &status);
        result = UNIQUEID_OK;
    }
    return result;
}
