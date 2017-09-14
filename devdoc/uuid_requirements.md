uuid requirements
=================

## Overview
The UUID module provides functions to create and convert UUID values.

## Exposed API
```c
typedef unsigned char UUID[16];

extern int uuid_generate(UUID* uuid);
extern int uuid_from_string(char* uuid_string, UUID* uuid);
extern char* uuid_to_string(UUID* uuid);
```

###  uuid_generate
```c
extern int uuid_generate(UUID* uuid);
```
**SRS_UUID_09_001: [** If `uuid` is NULL, uuid_generate shall return a non-zero value **]**

**SRS_UUID_09_002: [** uuid_generate shall obtain an UUID string from UniqueId_Generate **]**

**SRS_UUID_09_003: [** If the UUID string fails to be obtained, uuid_generate shall fail and return a non-zero value **]**

**SRS_UUID_09_004: [** The UUID string shall be parsed into an UUID type (16 unsigned char array) and filled in `uuid` **]**  

**SRS_UUID_09_005: [** If `uuid` fails to be set, uuid_generate shall fail and return a non-zero value **]**

**SRS_UUID_09_006: [** If no failures occur, uuid_generate shall return zero **]**


###  uuid_from_string
```c
extern int uuid_from_string(char* uuid_string, UUID* uuid);
```
**SRS_UUID_09_007: [** If `uuid_string` or `uuid` are NULL, uuid_from_string shall return a non-zero value **]**

**SRS_UUID_09_008: [** Each pair of digits in `uuid_string`, excluding dashes, shall be read as a single HEX value and saved on the respective position in `uuid` **]**  

**SRS_UUID_09_009: [** If `uuid` fails to be generated, uuid_from_string shall return a non-zero value **]**

**SRS_UUID_09_010: [** If no failures occur, uuid_from_string shall return zero **]**


###  uuid_to_string
```c
extern char* uuid_to_string(UUID* uuid);
```
**SRS_UUID_09_011: [** If `uuid` is NULL, uuid_to_string shall return a non-zero value **]**  

**SRS_UUID_09_012: [** uuid_to_string shall allocate a valid UUID string (`uuid_string`) as per RFC 4122 **]**  

**SRS_UUID_09_013: [** If `uuid_string` fails to be allocated, uuid_to_string shall return NULL **]**  

**SRS_UUID_09_014: [** Each character in `uuid` shall be written in the respective positions of `uuid_string` as a 2-digit HEX value **]**  

**SRS_UUID_09_015: [** If `uuid_string` fails to be set, uuid_to_string shall return NULL **]**  

**SRS_UUID_09_016: [** If no failures occur, uuid_to_string shall return `uuid_string` **]**  
