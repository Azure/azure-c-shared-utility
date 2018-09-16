STRING TOKENIZER Requirements
================

## Overview
The STRING TOKENIZER provides the functionality of splitting a STRING into multiples tokens

## Exposed API
```C
extern STRING_TOKEN_HANDLE StringToken_GetFirst(const char* source, size_t length, const char** delimiters, size_t n_delims);
extern STRING_TOKEN_HANDLE StringToken_GetNext(STRING_TOKEN_HANDLE token, const char** delimiters, size_t n_delims);
extern const char* StringToken_GetValue(STRING_TOKEN_HANDLE token);
extern size_t StringToken_GetLength(STRING_TOKEN_HANDLE token);
extern const char* StringToken_GetDelimiter(STRING_TOKEN_HANDLE token);
extern void StringToken_Destroy(STRING_TOKEN_HANDLE token);
```

###  StringToken_GetFirst
```c
extern STRING_TOKEN_HANDLE StringToken_GetFirst(const char* source, size_t length, const char** delimiters, size_t n_delims);
```

**SRS_STRING_TOKENIZER_09_001: [** If `source` or `delimiters` are NULL, or `length` or `n_delims` are zero, the function shall return NULL **]**

**SRS_STRING_TOKENIZER_09_002: [** If any of the strings in `delimiters` are NULL, the function shall return NULL **]**

**SRS_STRING_TOKENIZER_09_003: [** A STRING_TOKEN structure shall be allocated to hold the token parameters **]**

**SRS_STRING_TOKENIZER_09_004: [** If the STRING_TOKEN structure fails to be allocated, the function shall return NULL **]**

**SRS_STRING_TOKENIZER_09_005: [** The source string shall be split in a token starting from the beginning of `source` up to occurrence of any one of the `demiliters`, whichever occurs first in the order provided **]**

**SRS_STRING_TOKENIZER_09_006: [** If the source string does not have any of the `demiliters`, the resulting token shall be the entire `source` string **]**

**SRS_STRING_TOKENIZER_09_007: [** If any failure occurs, all memory allocated by this function shall be released **]**


###  StringToken_GetNext
```c
extern STRING_TOKEN_HANDLE StringToken_GetNext(STRING_TOKEN_HANDLE token, const char** delimiters, size_t n_delims);
```

**SRS_STRING_TOKENIZER_09_008: [** If `token` or `delimiters` are NULL, or `n_delims` is zero, the function shall return NULL **]**

**SRS_STRING_TOKENIZER_09_009: [** If the previous token already extended to the end of `source`, the function shall delete `token` and return NULL **]**

**SRS_STRING_TOKENIZER_09_010: [** The next token shall be selected starting from the position in `source` right after the previous delimiter up to occurrence of any one of `demiliters`, whichever occurs first in the order provided **]**

**SRS_STRING_TOKENIZER_09_011: [** If the source string, starting right after the position of the last delimiter found, does not have any of the `demiliters`, the resulting token shall be the entire remaining of the `source` string **]**

**SRS_STRING_TOKENIZER_09_012: [** If any failure occurs, the memory allocated for STRING_TOKEN shall be released **]**


###  StringToken_GetValue
```c
extern const char* StringToken_GetValue(STRING_TOKEN_HANDLE token);
```

**SRS_STRING_TOKENIZER_09_013: [** If `token` is NULL the function shall return NULL **]**

**SRS_STRING_TOKENIZER_09_014: [** The function shall return the pointer to the position in `source` where the current token starts. **]**


###  StringToken_GetLength
```c
extern size_t StringToken_GetLength(STRING_TOKEN_HANDLE token);
```

**SRS_STRING_TOKENIZER_09_015: [** If `token` is NULL the function shall return zero **]**

**SRS_STRING_TOKENIZER_09_016: [** The function shall return the length of the current token **]**


###  StringToken_GetDelimiter
```c
extern const char* StringToken_GetDelimiter(STRING_TOKEN_HANDLE token);
```

**SRS_STRING_TOKENIZER_09_017: [** If `token` is NULL the function shall return NULL **]**

**SRS_STRING_TOKENIZER_09_018: [** The function shall return a pointer to the delimiter that defined the current token, as passed to the previous call to `StringToken_GetNext()` or `StringToken_GetFirst()` **]**

**SRS_STRING_TOKENIZER_09_019: [** If the current token extends to the end of `source`, the function shall return NULL **]**


###  StringToken_Destroy
```c
extern void StringToken_Destroy(STRING_TOKEN_HANDLE token);
```

**SRS_STRING_TOKENIZER_09_020: [** If `token` is NULL the function shall return **]**

**SRS_STRING_TOKENIZER_09_021: [** Otherwise the memory allocated for STRING_TOKEN shall be released **]**
