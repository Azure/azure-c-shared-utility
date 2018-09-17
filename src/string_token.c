// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include "azure_c_shared_utility/gballoc.h"
#include "azure_c_shared_utility/optimize_size.h"
#include "azure_c_shared_utility/xlogging.h"
#include "azure_c_shared_utility/crt_abstractions.h"
#include "azure_c_shared_utility/string_token.h"

typedef struct STRING_TOKEN_TAG
{
    const char* source;
    size_t length;

    const char* token_start;
    const char* delimiter_start;
    const char* delimiter;
} STRING_TOKEN;

static size_t* get_delimiters_lengths(const char** delimiters, size_t n_delims)
{
    size_t* result;

    if ((result = malloc(sizeof(size_t) * n_delims)) == NULL)
    {
        LogError("Failed to allocate array for delimiters lengths");
    }
    else
    {
        size_t i;
        for (i = 0; i < n_delims; i++)
        {
            if (delimiters[i] == NULL)
            {
                // Codes_SRS_STRING_TOKENIZER_09_002: [ If any of the strings in `delimiters` are NULL, the function shall return NULL ]
                LogError("Invalid argument (delimiter %d is NULL)", i);
                free(result);
                result = NULL;
                break;
            }
            else
            {
                result[i] = strlen(delimiters[i]);
            }
        }
    }

    return result;
}

static int get_next_token(STRING_TOKEN* token, const char** delimiters, size_t n_delims)
{
    int result;

    if (token->token_start != NULL && token->delimiter_start == NULL)
    {
        // The parser reached the end of the input string.
        result = __FAILURE__;
    }
    else
    {
        size_t* delimiters_lengths;

        if ((delimiters_lengths = get_delimiters_lengths(delimiters, n_delims)) == NULL)
        {
            LogError("Failed to get delimiters lengths");
            result = __FAILURE__;
        }
        else
        {
            const char* new_token_start;
            const char* current_pos;
            const char* stop_pos = (char*)token->source + token->length;
            size_t j; // iterator for the delimiters.

            if (token->delimiter_start == NULL)
            {
                // Codes_SRS_STRING_TOKENIZER_09_005: [ The source string shall be split in a token starting from the beginning of `source` up to occurrence of any one of the `demiliters`, whichever occurs first in the order provided ]
                new_token_start = (char*)token->source;
            }
            else
            {
                // Codes_SRS_STRING_TOKENIZER_09_010: [ The next token shall be selected starting from the position in `source` right after the previous delimiter up to occurrence of any one of `demiliters`, whichever occurs first in the order provided ]
                new_token_start = token->delimiter_start + strlen(token->delimiter);
            }

            current_pos = new_token_start;
            result = 0;

            while (current_pos < stop_pos)
            {
                for (j = 0; j < n_delims; j++)
                {
                    if (*current_pos == *delimiters[j])
                    {
                        size_t k;
                        for (k = 1; k < delimiters_lengths[j] && (current_pos + k) < stop_pos; k++)
                        {
                            if (*(current_pos + k) != *(delimiters[j] + k))
                            {
                                break;
                            }
                        }

                        if (k == delimiters_lengths[j])
                        {
                            token->delimiter_start = current_pos;
                            token->delimiter = delimiters[j];

                            if (token->delimiter_start == token->source)
                            {
                                // Delimiter occurs in the beginning of the source string.
                                token->token_start = NULL;
                            }
                            else
                            {
                                token->token_start = new_token_start;
                            }
                            goto SCAN_COMPLETED;
                        }
                    }
                }

                current_pos++;
            }

            // Codes_SRS_STRING_TOKENIZER_09_006: [ If the source string does not have any of the `demiliters`, the resulting token shall be the entire `source` string ]
            // Codes_SRS_STRING_TOKENIZER_09_011: [ If the source string, starting right after the position of the last delimiter found, does not have any of the `demiliters`, the resulting token shall be the entire remaining of the `source` string ]
            if (current_pos == stop_pos)
            {
                token->token_start = new_token_start;
                token->delimiter_start = NULL;
                // Codes_SRS_STRING_TOKENIZER_09_019: [ If the current token extends to the end of `source`, the function shall return NULL ]
                token->delimiter = NULL;
            }

SCAN_COMPLETED:
            free(delimiters_lengths);
        }
    }

    return result;
}

STRING_TOKEN_HANDLE StringToken_GetFirst(const char* source, size_t length, const char** delimiters, size_t n_delims)
{
    STRING_TOKEN* result;

    // Codes_SRS_STRING_TOKENIZER_09_001: [ If `source` or `delimiters` are NULL, or `length` or `n_delims` are zero, the function shall return NULL ]
    if (source == NULL || length == 0 || delimiters == NULL || n_delims == 0)
    {
        LogError("Invalid argument (source=%p, length=%lu, delimiters=%p, n_delims=%lu)", source, (unsigned long)length, delimiters, (unsigned long)n_delims);
        result = NULL;
    }
    else
    {
        // Codes_SRS_STRING_TOKENIZER_09_003: [ A STRING_TOKEN structure shall be allocated to hold the token parameters ]
        if ((result = (STRING_TOKEN*)malloc(sizeof(STRING_TOKEN))) == NULL)
        {
            // Codes_SRS_STRING_TOKENIZER_09_004: [ If the STRING_TOKEN structure fails to be allocated, the function shall return NULL ]
            LogError("Failed allocating STRING_TOKEN");
        }
        else
        {
            (void)memset(result, 0, sizeof(STRING_TOKEN));
            result->source = source;
            result->length = length;

            if (get_next_token(result, delimiters, n_delims) != 0)
            {
                LogError("Failed to get first token");
                // Codes_SRS_STRING_TOKENIZER_09_007: [ If any failure occurs, all memory allocated by this function shall be released ]
                free(result);
                result = NULL;
            }
        }
    }

    return result;
}


bool StringToken_GetNext(STRING_TOKEN_HANDLE token, const char** delimiters, size_t n_delims)
{
    bool result;

    // Codes_SRS_STRING_TOKENIZER_09_008: [ If `token` or `delimiters` are NULL, or `n_delims` is zero, the function shall return false ]
    if (token == NULL || delimiters == NULL || n_delims == 0)
    {
        LogError("Invalid argument (token=%p, delimiters=%p, n_delims=%lu)", token, delimiters, (unsigned long)n_delims);
        result = false;
    }
    else if (get_next_token(token, delimiters, n_delims) != 0)
    {
        // Codes_SRS_STRING_TOKENIZER_09_009: [ If the previous token already extended to the end of `source`, the function shall return false ]
        result = false;
    }
    else
    {
        // Codes_SRS_STRING_TOKENIZER_09_012: [ If a token was identified, the function shall return true ]
        result = true;
    }

    return result;
}

const char* StringToken_GetValue(STRING_TOKEN_HANDLE token)
{
    const char* result;

    // Codes_SRS_STRING_TOKENIZER_09_013: [ If `token` is NULL the function shall return NULL ]
    if (token == NULL)
    {
        LogError("Invalig argument (token is NULL)");
        result = NULL;
    }
    else if (token->token_start == (token->source + token->length))
    {
        result = NULL;
    }
    else
    {
        // Codes_SRS_STRING_TOKENIZER_09_014: [ The function shall return the pointer to the position in `source` where the current token starts. ]
        result = token->token_start;
    }

    return result;
}

size_t StringToken_GetLength(STRING_TOKEN_HANDLE token)
{
    size_t result;

    // Codes_SRS_STRING_TOKENIZER_09_015: [ If `token` is NULL the function shall return zero ]
    if (token == NULL)
    {
        LogError("Invalig argument (token is NULL)");
        result = 0;
    }
    // Codes_SRS_STRING_TOKENIZER_09_016: [ The function shall return the length of the current token ]
    else if (token->token_start == NULL)
    {
        result = 0;
    }
    else if (token->delimiter_start == NULL)
    {
        result = token->source + token->length - token->token_start;
    }
    else
    {
        result = token->delimiter_start - token->token_start;
    }

    return result;
}

const char* StringToken_GetDelimiter(STRING_TOKEN_HANDLE token)
{
    const char* result;

    if (token == NULL)
    {
        // Codes_SRS_STRING_TOKENIZER_09_017: [ If `token` is NULL the function shall return NULL ]
        LogError("Invalig argument (token is NULL)");
        result = NULL;
    }
    else
    {
        // Codes_SRS_STRING_TOKENIZER_09_018: [ The function shall return a pointer to the delimiter that defined the current token, as passed to the previous call to `StringToken_GetNext()` or `StringToken_GetFirst()` ]
        result = token->delimiter;
    }

    return result;
}

void StringToken_Destroy(STRING_TOKEN_HANDLE token)
{
    if (token == NULL)
    {
        // Codes_SRS_STRING_TOKENIZER_09_020: [ If `token` is NULL the function shall return ]
        LogError("Invalig argument (token is NULL)");
    }
    else
    {
        // Codes_SRS_STRING_TOKENIZER_09_021: [ Otherwise the memory allocated for STRING_TOKEN shall be released ]
        free(token);
    }
}