uniqueid requirements
=================

## Overview
The UniqueIDs module provides pseudo unique identifiers to the rest of the modules

## Exposed API
```C
#define UNIQUEID_RESULT_VALUES    \
    UNIQUEID_OK,                  \
    UNIQUEID_INVALID_ARG,         \
    UNIQUEID_ERROR

    DEFINE_ENUM(UNIQUEID_RESULT, UNIQUEID_RESULT_VALUES)

    extern UNIQUEID_RESULT UniqueId_Generate(char* uid, size_t bufferSize);
	extern UNIQUEID_RESULT UniqueId_GetStringFromBytes(unsigned char* uid, size_t uuid_size, char* output_string);
```
###  UniqueId_Generate
```C
extern UNIQUEID_RESULT UniqueId_Generate(char* uid, size_t len);
```
**SRS_UNIQUEID_07_001: [** UniqueId_Generate shall create a unique Id 36 character long string. **]**

**SRS_UNIQUEID_07_002: [** If uid is NULL then UniqueId_Generate shall return UNIQUEID_INVALID_ARG **]**

**SRS_UNIQUEID_07_003: [** If len is less then 37 then UniqueId_Generate shall return UNIQUEID_INVALID_ARG **]**

**SRS_UNIQUEID_07_004: [** If there is a failure for any reason the UniqueId_Generate shall return UNIQUEID_ERROR **]**  


###  UniqueId_GetStringFromBytes
```C
extern UNIQUEID_RESULT UniqueId_GetStringFromBytes(unsigned char* uid, size_t uuid_size, char* output_string);
```
**SRS_UNIQUEID_09_001: [** If `uid` or `output_string` are NULL, UniqueId_GetStringFromBytes shall return UNIQUEID_INVALID_ARG **]**

**SRS_UNIQUEID_09_002: [** If `uid_size` is zero or not a multiple of two, UniqueId_GetStringFromBytes shall return UNIQUEID_INVALID_ARG **]**

**SRS_UNIQUEID_09_003: [** `output_string` shall be filled according to RFC4122 using the byte sequence in `uid` **]**

**SRS_UNIQUEID_09_004: [** If no failures occur, UniqueId_Generate shall return UNIQUEID_OK **]**  