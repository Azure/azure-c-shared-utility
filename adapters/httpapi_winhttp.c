// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdlib.h>
#ifdef _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#include "azure_c_shared_utility/gballoc.h"
#include "azure_c_shared_utility/crt_abstractions.h"

#include <stddef.h>
#include "windows.h"
#include "winhttp.h"
#include <string.h>
#include "azure_c_shared_utility/httpapi.h"
#include "azure_c_shared_utility/httpheaders.h"
#include "azure_c_shared_utility/xlogging.h"
#include "azure_c_shared_utility/strings.h"

#define TEMP_BUFFER_SIZE 1024
#define MESSAGE_BUFFER_SIZE 260

#define LogErrorWinHTTPWithGetLastErrorAsString(FORMAT, ...) { \
                DWORD errorMessageID = GetLastError(); \
                LogError(FORMAT, __VA_ARGS__); \
                CHAR messageBuffer[MESSAGE_BUFFER_SIZE]; \
                if (errorMessageID == 0) \
                {\
                    LogError("GetLastError() returned 0. Make sure you are calling this right after the code that failed. "); \
                } \
                else\
                {\
                    int size = FormatMessage(FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_IGNORE_INSERTS, \
                        GetModuleHandle("WinHttp"), errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), messageBuffer, MESSAGE_BUFFER_SIZE, NULL); \
                    if (size == 0)\
                    {\
                        size = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), messageBuffer, MESSAGE_BUFFER_SIZE, NULL); \
                        if (size == 0)\
                        {\
                            LogError("GetLastError Code: %d. ", errorMessageID); \
                        }\
                        else\
                        {\
                            LogError("GetLastError: %s.", messageBuffer); \
                        }\
                    }\
                    else\
                    {\
                        LogError("GetLastError: %s.", messageBuffer); \
                    }\
                }\
            } \

DEFINE_ENUM_STRINGS(HTTPAPI_RESULT, HTTPAPI_RESULT_VALUES)

typedef enum HTTPAPI_STATE_TAG
{
    HTTPAPI_NOT_INITIALIZED,
    HTTPAPI_INITIALIZED
} HTTPAPI_STATE;

typedef struct X509CRYPTO_TAG
{
    HCRYPTPROV hProv;
    HCRYPTKEY x509hcryptkey;
    PCCERT_CONTEXT x509certificate_context;
}X509_CRYPTO;

static X509_CRYPTO*  HTTPAPI_CreateCrypto(const char* x509certificate, const char* x509privatekey)
{
    X509_CRYPTO *result;
    /*this is what happens with the x509 certificate and the x509 private key in this function*/
    /*
    step 1: they are converted to binary form.
    step 1.1: the size of the binary form is computed
    step 1.2: the conversion happens
    step 2: the binary form is decoded
    step 2.1: the decoded form needed size is computed
    step 2.2: the decoded form is actually decoded
    step 3: a crypto provider is created
    step 4: the x509 private key is associated with the crypto provider
    step 5: a certificate context is created
    step 6: the certificate context is linked to the crypto provider
    */

    result = (X509_CRYPTO*)malloc(sizeof(X509_CRYPTO));
    if (result == NULL)
    {
        LogError("unable to malloc");
        /*return as is*/

    }
    else
    {
        DWORD binaryx509certificateSize;
        if (!CryptStringToBinaryA(x509certificate, 0, CRYPT_STRING_ANY, NULL, &binaryx509certificateSize, NULL, NULL))
        {
            LogErrorWinHTTPWithGetLastErrorAsString("Failed convert x509 certificate");
            free(result);
            result = NULL;
        }
        else
        {
            unsigned char* binaryx509Certificate = (unsigned char*)malloc(binaryx509certificateSize);
            if (binaryx509Certificate == NULL)
            {
                LogError("unable to allocate memory for x509 certificate");
                free(result);
                result = NULL;
            }
            else
            {
                if (!CryptStringToBinaryA(x509certificate, 0, CRYPT_STRING_ANY, binaryx509Certificate, &binaryx509certificateSize, NULL, NULL))
                {
                    LogErrorWinHTTPWithGetLastErrorAsString("Failed convert x509 certificate");
                    free(result);
                    result = NULL;
                }
                else
                {
                    /*at this moment x509 certificate is ready to be used in CertCreateCertificateContext*/
                    DWORD binaryx509privatekeySize;
                    if (!CryptStringToBinaryA(x509privatekey, 0, CRYPT_STRING_ANY, NULL, &binaryx509privatekeySize, NULL, NULL))
                    {
                        LogErrorWinHTTPWithGetLastErrorAsString("Failed convert x509 private key");
                        free(result);
                        result = NULL;
                    }
                    else
                    {
                        unsigned char* binaryx509privatekey = (unsigned char*)malloc(binaryx509privatekeySize);
                        if (binaryx509privatekey == NULL)
                        {
                            LogError("unable to malloc for binaryx509privatekey");
                            free(result);
                            result = NULL;
                        }
                        else
                        {
                            if (!CryptStringToBinaryA(x509privatekey, 0, CRYPT_STRING_ANY, binaryx509privatekey, &binaryx509privatekeySize, NULL, NULL))
                            {
                                LogErrorWinHTTPWithGetLastErrorAsString("Failed convert x509 private key");
                                free(result);
                                result = NULL;
                            }
                            else
                            {
                                DWORD x509privatekeyBlobSize;
                                if (!CryptDecodeObjectEx(X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, PKCS_RSA_PRIVATE_KEY, binaryx509privatekey, binaryx509privatekeySize, 0, NULL, NULL, &x509privatekeyBlobSize))
                                {
                                    LogErrorWinHTTPWithGetLastErrorAsString("Failed to CryptDecodeObjectEx x509 private key");
                                    free(result);
                                    result = NULL;
                                }
                                else
                                {
                                    unsigned char* x509privatekeyBlob = (unsigned char*)malloc(x509privatekeyBlobSize);
                                    if (x509privatekeyBlob == NULL)
                                    {
                                        LogError("unable to malloc for x509 private key blob");
                                        free(result);
                                        result = NULL;
                                    }
                                    else
                                    {
                                        if (!CryptDecodeObjectEx(X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, PKCS_RSA_PRIVATE_KEY, binaryx509privatekey, binaryx509privatekeySize, 0, NULL, x509privatekeyBlob, &x509privatekeyBlobSize))
                                        {
                                            LogErrorWinHTTPWithGetLastErrorAsString("Failed to CryptDecodeObjectEx x509 private key");
                                            free(result);
                                            result = NULL;
                                        }
                                        else
                                        {
                                            /*at this moment, both the private key and the certificate are decoded for further usage*/
                                            if (!CryptAcquireContext(&(result->hProv), NULL, MS_ENHANCED_PROV, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT))
                                            {
                                                LogErrorWinHTTPWithGetLastErrorAsString("CryptAcquireContext failed");
                                                free(result);
                                                result = NULL;
                                            }
                                            else
                                            {
                                                if (!CryptImportKey(result->hProv, x509privatekeyBlob, x509privatekeyBlobSize, (HCRYPTKEY)NULL, 0, &(result->x509hcryptkey)))
                                                {
                                                    if (!CryptReleaseContext(result->hProv, 0))
                                                    {
                                                        LogErrorWinHTTPWithGetLastErrorAsString("unable to CryptReleaseContext");
                                                    }
                                                    LogErrorWinHTTPWithGetLastErrorAsString("CryptImportKey for private key failed");
                                                    free(result);
                                                    result = NULL;
                                                }
                                                else
                                                {
                                                     result->x509certificate_context = CertCreateCertificateContext(
                                                        X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                                                        binaryx509Certificate,
                                                        binaryx509certificateSize
                                                    );
                                                    if (result->x509certificate_context == NULL)
                                                    {
                                                        LogErrorWinHTTPWithGetLastErrorAsString("unable to CertCreateCertificateContext");
                                                        if (!CryptDestroyKey(result->x509hcryptkey))
                                                        {
                                                            LogErrorWinHTTPWithGetLastErrorAsString("unable to CryptDestroyKey");
                                                        }
                                                        if (!CryptReleaseContext(result->hProv, 0))
                                                        {
                                                            LogErrorWinHTTPWithGetLastErrorAsString("unable to CryptReleaseContext");
                                                        }
                                                        free(result);
                                                        result = NULL;
                                                    }
                                                    else
                                                    {
                                                        if (!CertSetCertificateContextProperty(result->x509certificate_context, CERT_KEY_PROV_HANDLE_PROP_ID, 0, (void*)result->hProv))
                                                        {
                                                            LogErrorWinHTTPWithGetLastErrorAsString("unable to CertSetCertificateContextProperty");
                                                            if (!CertFreeCertificateContext(result->x509certificate_context))
                                                            {
                                                                LogErrorWinHTTPWithGetLastErrorAsString("unable to CertFreeCertificateContext");
                                                            }
                                                            if (!CryptDestroyKey(result->x509hcryptkey))
                                                            {
                                                                LogErrorWinHTTPWithGetLastErrorAsString("unable to CryptDestroyKey");
                                                            }
                                                            if (!CryptReleaseContext(result->hProv, 0))
                                                            {
                                                                LogErrorWinHTTPWithGetLastErrorAsString("unable to CryptReleaseContext");
                                                            }
                                                            free(result);
                                                            result = NULL;
                                                        }
                                                        else
                                                        {
                                                            /*return as is, all is fine*/
                                                        }
                                                        
                                                    }
                                                    
                                                }
                                            }
                                        }
                                        free(x509privatekeyBlob);
                                    }
                                }
                            }
                            free(binaryx509privatekey);
                        }
                    }
                }
                free(binaryx509Certificate);
            }
        }
    }
    return result;
}

static void HTTPAPI_DestroyCrypto(X509_CRYPTO* x509crypto)
{
    if (x509crypto != NULL)
    {
        if (!CertFreeCertificateContext(x509crypto->x509certificate_context))
        {
            LogErrorWinHTTPWithGetLastErrorAsString("unable to CertFreeCertificateContext");
        }

        if (!CryptDestroyKey(x509crypto->x509hcryptkey))
        {
            LogErrorWinHTTPWithGetLastErrorAsString("unable to CryptDestroyKey");
        }

        if (!CryptReleaseContext(x509crypto->hProv, 0))
        {
            LogErrorWinHTTPWithGetLastErrorAsString("unable to CryptReleaseContext");
        }

        free(x509crypto);
    }
}

typedef struct HTTP_HANDLE_DATA_TAG
{
    /*working set*/
    HINTERNET ConnectionHandle;
    X509_CRYPTO* x509crypto;
    /*options*/
    unsigned int timeout;
    const char* x509certificate;
    const char* x509privatekey;
} HTTP_HANDLE_DATA;

static HTTPAPI_STATE g_HTTPAPIState = HTTPAPI_NOT_INITIALIZED;

/*There's a global SessionHandle for all the connections*/
static HINTERNET g_SessionHandle;
static size_t nUsersOfHTTPAPI = 0; /*used for reference counting (a weak one)*/

/*returns NULL if it failed to construct the headers*/
static const char* ConstructHeadersString(HTTP_HEADERS_HANDLE httpHeadersHandle)
{
    char* result;
    size_t headersCount;

    if (HTTPHeaders_GetHeaderCount(httpHeadersHandle, &headersCount) != HTTP_HEADERS_OK)
    {
        result = NULL;
        LogError("HTTPHeaders_GetHeaderCount failed.");
    }
    else
    {
        size_t i;
        
        /*the total size of all the headers is given by sumof(lengthof(everyheader)+2)*/
        size_t toAlloc = 0;
        for (i = 0; i < headersCount; i++)
        {
            char *temp;
            if (HTTPHeaders_GetHeader(httpHeadersHandle, i, &temp) == HTTP_HEADERS_OK)
            {
                toAlloc += strlen(temp);
                toAlloc += 2;
                free(temp);
            }
            else
            {
                LogError("HTTPHeaders_GetHeader failed");
                break;
            }
        }

        if (i < headersCount)
        {
            result = NULL;
        }
        else
        {
            result = malloc(toAlloc*sizeof(char) + 1 );
            
            if (result == NULL)
            {
                LogError("unable to malloc");
                /*let it be returned*/
            }
            else
            {
                result[0] = '\0';
                for (i = 0; i < headersCount; i++)
                {
                    char* temp;
                    if (HTTPHeaders_GetHeader(httpHeadersHandle, i, &temp) != HTTP_HEADERS_OK)
                    {
                        LogError("unable to HTTPHeaders_GetHeader");
                        break;
                    }
                    else
                    {
                        (void)strcat(result, temp);
                        (void)strcat(result, "\r\n");
                        free(temp);
                    }
                }

                if (i < headersCount)
                {
                    free(result);
                    result = NULL;
                }
                else
                {
                    /*all is good*/
                }
            }
        }
    }

    return result;
}

HTTPAPI_RESULT HTTPAPI_Init(void)
{
    HTTPAPI_RESULT result;

    if (nUsersOfHTTPAPI == 0)
    {
        if ((g_SessionHandle = WinHttpOpen(
            NULL,
            WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
            WINHTTP_NO_PROXY_NAME,
            WINHTTP_NO_PROXY_BYPASS,
            0)) == NULL)
        {
            LogErrorWinHTTPWithGetLastErrorAsString("WinHttpOpen failed.");
            result = HTTPAPI_INIT_FAILED;
        }
        else
        {
            nUsersOfHTTPAPI++;
            g_HTTPAPIState = HTTPAPI_INITIALIZED;
            result = HTTPAPI_OK;
        }
    }
    else
    {
        nUsersOfHTTPAPI++;
        result = HTTPAPI_OK;
    }

    return result;
}

void HTTPAPI_Deinit(void)
{
    if (nUsersOfHTTPAPI > 0)
    {
        nUsersOfHTTPAPI--;
        if (nUsersOfHTTPAPI == 0)
        {
            if (g_SessionHandle != NULL)
            {
                (void)WinHttpCloseHandle(g_SessionHandle);
                g_SessionHandle = NULL;
                g_HTTPAPIState = HTTPAPI_NOT_INITIALIZED;
            }
        }
    }

    
}

HTTP_HANDLE HTTPAPI_CreateConnection(const char* hostName)
{
    HTTP_HANDLE_DATA* result;
    if (g_HTTPAPIState != HTTPAPI_INITIALIZED)
    {
        LogError("g_HTTPAPIState not HTTPAPI_INITIALIZED");
        result = NULL;
    }
    else
    {
        result = (HTTP_HANDLE_DATA*)malloc(sizeof(HTTP_HANDLE_DATA));
        if (result == NULL)
        {
            LogError("malloc returned NULL.");
        }
        else
        {
            wchar_t* hostNameTemp;
            size_t hostNameTemp_size = MultiByteToWideChar(CP_ACP, 0, hostName, -1, NULL, 0);
            if (hostNameTemp_size == 0)
            {
                LogError("MultiByteToWideChar failed");
                free(result);
                result = NULL;
            }
            else
            {
                hostNameTemp = malloc(sizeof(wchar_t)*hostNameTemp_size);
                if (hostNameTemp == NULL)
                {
                    LogError("malloc failed");
                    free(result);
                    result = NULL;
                }
                else
                {
                    if (MultiByteToWideChar(CP_ACP, 0, hostName, -1, hostNameTemp, (int)hostNameTemp_size) == 0)
                    {
                        LogError("MultiByteToWideChar failed");
                        free(result);
                        result = NULL;
                    }
                    else
                    {
                        result->ConnectionHandle = WinHttpConnect(
                            g_SessionHandle,
                            hostNameTemp,
                            INTERNET_DEFAULT_HTTPS_PORT,
                            0);

                        if (result->ConnectionHandle == NULL)
                        {
                            LogErrorWinHTTPWithGetLastErrorAsString("WinHttpConnect returned NULL.");
                            free(result);
                            result = NULL;
                        }
                        else
                        {
                            result->timeout = 30000; /*default from MSDN (  https://msdn.microsoft.com/en-us/library/windows/desktop/aa384116(v=vs.85).aspx  )*/
                            result->x509certificate = NULL;
                            result->x509privatekey = NULL;
                            result->x509crypto = NULL;
                        }
                    }
                    free(hostNameTemp);
                }
            }
        }
    }

    return (HTTP_HANDLE)result;
}

void HTTPAPI_CloseConnection(HTTP_HANDLE handle)
{
    if (g_HTTPAPIState != HTTPAPI_INITIALIZED)
    {
        LogError("g_HTTPAPIState not HTTPAPI_INITIALIZED");
    }
    else
    {
        HTTP_HANDLE_DATA* handleData = (HTTP_HANDLE_DATA*)handle;

        if (handleData != NULL)
        {
            if (handleData->ConnectionHandle != NULL)
            {
                (void)WinHttpCloseHandle(handleData->ConnectionHandle);
                /*no x509 free because the options are owned by httpapiex.*/
                handleData->ConnectionHandle = NULL;
            }
            HTTPAPI_DestroyCrypto(handleData->x509crypto);
            free(handleData);
        }
    }
}

HTTPAPI_RESULT HTTPAPI_ExecuteRequest(HTTP_HANDLE handle, HTTPAPI_REQUEST_TYPE requestType, const char* relativePath,
    HTTP_HEADERS_HANDLE httpHeadersHandle, const unsigned char* content,
    size_t contentLength, unsigned int* statusCode,
    HTTP_HEADERS_HANDLE responseHeadersHandle, BUFFER_HANDLE responseContent)
{
    HTTPAPI_RESULT result;
    if (g_HTTPAPIState != HTTPAPI_INITIALIZED)
    {
        LogError("g_HTTPAPIState not HTTPAPI_INITIALIZED");
        result = HTTPAPI_NOT_INIT;
    }
    else
    {
        HTTP_HANDLE_DATA* handleData = (HTTP_HANDLE_DATA*)handle;

        if ((handleData == NULL) ||
            (relativePath == NULL) ||
            (httpHeadersHandle == NULL))
        {
            result = HTTPAPI_INVALID_ARG;
            LogError("NULL parameter detected (result = %s)", ENUM_TO_STRING(HTTPAPI_RESULT, result));
        }
        else
        {
            wchar_t* requestTypeString = NULL;

            switch (requestType)
            {
            default:
                break;

            case HTTPAPI_REQUEST_GET:
                requestTypeString = L"GET";
                break;

            case HTTPAPI_REQUEST_POST:
                requestTypeString = L"POST";
                break;

            case HTTPAPI_REQUEST_PUT:
                requestTypeString = L"PUT";
                break;

            case HTTPAPI_REQUEST_DELETE:
                requestTypeString = L"DELETE";
                break;

            case HTTPAPI_REQUEST_PATCH:
                requestTypeString = L"PATCH";
                break;
            }

            if (requestTypeString == NULL)
            {
                result = HTTPAPI_INVALID_ARG;
                LogError("requestTypeString was NULL (result = %s)", ENUM_TO_STRING(HTTPAPI_RESULT, result));
            }
            else
            {
                const char* headers2;
                headers2 = ConstructHeadersString(httpHeadersHandle);
                if (headers2 != NULL)
                {
                    size_t requiredCharactersForRelativePath = MultiByteToWideChar(CP_ACP, 0, relativePath, -1, NULL, 0);
                    wchar_t* relativePathTemp = malloc((requiredCharactersForRelativePath+1) * sizeof(wchar_t));
                    result = HTTPAPI_OK; /*legacy code*/

                    if (relativePathTemp == NULL)
                    {
                        result = HTTPAPI_ALLOC_FAILED;
                        LogError("malloc failed (result = %s)", ENUM_TO_STRING(HTTPAPI_RESULT, result));
                    }
                    else
                    {
                        if (MultiByteToWideChar(CP_ACP, 0, relativePath, -1, relativePathTemp, (int)requiredCharactersForRelativePath) == 0)
                        {
                            result = HTTPAPI_STRING_PROCESSING_ERROR;
                            LogError("MultiByteToWideChar was 0. (result = %s)", ENUM_TO_STRING(HTTPAPI_RESULT, result));
                        }
                        else
                        {
                            size_t requiredCharactersForHeaders = MultiByteToWideChar(CP_ACP, 0, headers2, -1, NULL, 0);

                            wchar_t* headersTemp = malloc((requiredCharactersForHeaders +1) * sizeof(wchar_t) );
                            if (headersTemp == NULL)
                            {
                                result = HTTPAPI_STRING_PROCESSING_ERROR;
                                LogError("MultiByteToWideChar was 0. (result = %s)", ENUM_TO_STRING(HTTPAPI_RESULT, result));
                            }
                            else
                            {
                                if (MultiByteToWideChar(CP_ACP, 0, headers2, -1, headersTemp, (int)requiredCharactersForHeaders) == 0)
                                {
                                    result = HTTPAPI_STRING_PROCESSING_ERROR;
                                    LogError("MultiByteToWideChar was 0(result = %s)", ENUM_TO_STRING(HTTPAPI_RESULT, result));
                                }
                                else
                                {
                                    HINTERNET requestHandle = WinHttpOpenRequest(
                                        handleData->ConnectionHandle,
                                        requestTypeString,
                                        relativePathTemp,
                                        NULL,
                                        WINHTTP_NO_REFERER,
                                        WINHTTP_DEFAULT_ACCEPT_TYPES,
                                        WINHTTP_FLAG_SECURE);
                                    if (requestHandle == NULL)
                                    {
                                        result = HTTPAPI_OPEN_REQUEST_FAILED;
                                        LogErrorWinHTTPWithGetLastErrorAsString("WinHttpOpenRequest failed (result = %s).", ENUM_TO_STRING(HTTPAPI_RESULT, result));
                                    }
                                    else
                                    {
                                        if ((handleData->x509crypto!=NULL) && 
                                            !WinHttpSetOption(
                                                requestHandle,
                                                WINHTTP_OPTION_CLIENT_CERT_CONTEXT,
                                                (void*)handleData->x509crypto->x509certificate_context,
                                                sizeof(CERT_CONTEXT)
                                        ))
                                        {
                                            LogErrorWinHTTPWithGetLastErrorAsString("unable to WinHttpSetOption");
                                            result = HTTPAPI_SET_X509_FAILURE;
                                        }

                                        else
                                        {
                                            if (WinHttpSetTimeouts(requestHandle,
                                                0,                      /*_In_  int dwResolveTimeout - The initial value is zero, meaning no time-out (infinite). */
                                                60000,                  /*_In_  int dwConnectTimeout, -  The initial value is 60,000 (60 seconds).*/
                                                handleData->timeout,    /*_In_  int dwSendTimeout, -  The initial value is 30,000 (30 seconds).*/
                                                handleData->timeout     /* int dwReceiveTimeout The initial value is 30,000 (30 seconds).*/
                                            ) == FALSE)
                                            {
                                                result = HTTPAPI_SET_TIMEOUTS_FAILED;
                                                LogErrorWinHTTPWithGetLastErrorAsString("WinHttpOpenRequest failed (result = %s)", ENUM_TO_STRING(HTTPAPI_RESULT, result));
                                            }
                                            else
                                            {
                                                DWORD dwSecurityFlags = 0;

                                                if (!WinHttpSetOption(
                                                    requestHandle,
                                                    WINHTTP_OPTION_SECURITY_FLAGS,
                                                    &dwSecurityFlags,
                                                    sizeof(dwSecurityFlags)))
                                                {
                                                    result = HTTPAPI_SET_OPTION_FAILED;
                                                    LogErrorWinHTTPWithGetLastErrorAsString("WinHttpSetOption failed (result = %s).", ENUM_TO_STRING(HTTPAPI_RESULT, result));
                                                }
                                                else
                                                {
                                                    if (!WinHttpSendRequest(
                                                        requestHandle,
                                                        headersTemp,
                                                        (DWORD)-1L, /*An unsigned long integer value that contains the length, in characters, of the additional headers. If this parameter is -1L ... */
                                                        (void*)content,
                                                        (DWORD)contentLength,
                                                        (DWORD)contentLength,
                                                        0))
                                                    {
                                                        result = HTTPAPI_SEND_REQUEST_FAILED;
                                                        LogErrorWinHTTPWithGetLastErrorAsString("WinHttpSendRequest: (result = %s).", ENUM_TO_STRING(HTTPAPI_RESULT, result));
                                                    }
                                                    else
                                                    {
                                                        if (!WinHttpReceiveResponse(
                                                            requestHandle,
                                                            0))
                                                        {
                                                            result = HTTPAPI_RECEIVE_RESPONSE_FAILED;
                                                            LogErrorWinHTTPWithGetLastErrorAsString("WinHttpReceiveResponse: (result = %s).", ENUM_TO_STRING(HTTPAPI_RESULT, result));
                                                        }
                                                        else
                                                        {
                                                            DWORD dwStatusCode = 0;
                                                            DWORD dwBufferLength = sizeof(DWORD);
                                                            DWORD responseBytesAvailable;

                                                            if (!WinHttpQueryHeaders(
                                                                requestHandle,
                                                                WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
                                                                WINHTTP_HEADER_NAME_BY_INDEX,
                                                                &dwStatusCode,
                                                                &dwBufferLength,
                                                                WINHTTP_NO_HEADER_INDEX))
                                                            {
                                                                result = HTTPAPI_QUERY_HEADERS_FAILED;
                                                                LogErrorWinHTTPWithGetLastErrorAsString("WinHttpQueryHeaders failed (result = %s)", ENUM_TO_STRING(HTTPAPI_RESULT, result));
                                                            }
                                                            else
                                                            {
                                                                BUFFER_HANDLE useToReadAllResponse = (responseContent != NULL) ? responseContent : BUFFER_new();

                                                                if (statusCode != NULL)
                                                                {
                                                                    *statusCode = dwStatusCode;
                                                                }

                                                                if (useToReadAllResponse == NULL)
                                                                {
                                                                    result = HTTPAPI_ERROR;
                                                                    LogError("(result = %s)", ENUM_TO_STRING(HTTPAPI_RESULT, result));
                                                                }
                                                                else
                                                                {

                                                                    int goOnAndReadEverything = 1;
                                                                    do
                                                                    {
                                                                        /*from MSDN: If no data is available and the end of the file has not been reached, one of two things happens. If the session is synchronous, the request waits until data becomes available.*/
                                                                        if (!WinHttpQueryDataAvailable(requestHandle, &responseBytesAvailable))
                                                                        {
                                                                            result = HTTPAPI_QUERY_DATA_AVAILABLE_FAILED;
                                                                            LogErrorWinHTTPWithGetLastErrorAsString("WinHttpQueryDataAvailable failed (result = %s).", ENUM_TO_STRING(HTTPAPI_RESULT, result));
                                                                            goOnAndReadEverything = 0;
                                                                        }
                                                                        else if (responseBytesAvailable == 0)
                                                                        {
                                                                            /*end of the stream, go out*/
                                                                            result = HTTPAPI_OK;
                                                                            goOnAndReadEverything = 0;
                                                                        }
                                                                        else
                                                                        {
                                                                            if (BUFFER_enlarge(useToReadAllResponse, responseBytesAvailable) != 0)
                                                                            {
                                                                                result = HTTPAPI_ERROR;
                                                                                LogError("(result = %s)", ENUM_TO_STRING(HTTPAPI_RESULT, result));
                                                                                goOnAndReadEverything = 0;
                                                                            }
                                                                            else
                                                                            {
                                                                                /*Add the read bytes to the response buffer*/
                                                                                size_t bufferSize;
                                                                                const unsigned char* bufferContent;

                                                                                if (BUFFER_content(useToReadAllResponse, &bufferContent) != 0)
                                                                                {
                                                                                    result = HTTPAPI_ERROR;
                                                                                    LogError("(result = %s)", ENUM_TO_STRING(HTTPAPI_RESULT, result));
                                                                                    goOnAndReadEverything = 0;
                                                                                }
                                                                                else if (BUFFER_size(useToReadAllResponse, &bufferSize) != 0)
                                                                                {
                                                                                    result = HTTPAPI_ERROR;
                                                                                    LogError("(result = %s)", ENUM_TO_STRING(HTTPAPI_RESULT, result));
                                                                                    goOnAndReadEverything = 0;
                                                                                }
                                                                                else
                                                                                {
                                                                                    DWORD bytesReceived;
                                                                                    if (!WinHttpReadData(requestHandle, (LPVOID)(bufferContent + bufferSize - responseBytesAvailable), responseBytesAvailable, &bytesReceived))
                                                                                    {
                                                                                        result = HTTPAPI_READ_DATA_FAILED;
                                                                                        LogErrorWinHTTPWithGetLastErrorAsString("WinHttpReadData failed (result = %s)", ENUM_TO_STRING(HTTPAPI_RESULT, result));
                                                                                        goOnAndReadEverything = 0;
                                                                                    }
                                                                                    else
                                                                                    {
                                                                                        /*if for some reason bytesReceived is zero If you are using WinHttpReadData synchronously, and the return value is TRUE and the number of bytes read is zero, the transfer has been completed and there are no more bytes to read on the handle.*/
                                                                                        if (bytesReceived == 0)
                                                                                        {
                                                                                            /*end of everything, but this looks like an error still, or a non-conformance between WinHttpQueryDataAvailable and WinHttpReadData*/
                                                                                            result = HTTPAPI_READ_DATA_FAILED;
                                                                                            LogError("bytesReceived was unexpectedly zero (result = %s)", ENUM_TO_STRING(HTTPAPI_RESULT, result));
                                                                                            goOnAndReadEverything = 0;
                                                                                        }
                                                                                        else
                                                                                        {
                                                                                            /*all is fine, keep going*/
                                                                                        }
                                                                                    }
                                                                                }
                                                                            }
                                                                        }

                                                                    } while (goOnAndReadEverything != 0);
                                                                }
                                                            }

                                                            if (result == HTTPAPI_OK && responseHeadersHandle != NULL)
                                                            {
                                                                wchar_t* responseHeadersTemp;
                                                                DWORD responseHeadersTempLength = sizeof(responseHeadersTemp);

                                                                (void)WinHttpQueryHeaders(
                                                                    requestHandle,
                                                                    WINHTTP_QUERY_RAW_HEADERS_CRLF,
                                                                    WINHTTP_HEADER_NAME_BY_INDEX,
                                                                    WINHTTP_NO_OUTPUT_BUFFER,
                                                                    &responseHeadersTempLength,
                                                                    WINHTTP_NO_HEADER_INDEX);

                                                                responseHeadersTemp = (wchar_t*)malloc(responseHeadersTempLength + 2);
                                                                if (responseHeadersTemp == NULL)
                                                                {
                                                                    result = HTTPAPI_ALLOC_FAILED;
                                                                    LogError("malloc failed: (result = %s)", ENUM_TO_STRING(HTTPAPI_RESULT, result));
                                                                }
                                                                else
                                                                {
                                                                    if (WinHttpQueryHeaders(
                                                                        requestHandle,
                                                                        WINHTTP_QUERY_RAW_HEADERS_CRLF,
                                                                        WINHTTP_HEADER_NAME_BY_INDEX,
                                                                        responseHeadersTemp,
                                                                        &responseHeadersTempLength,
                                                                        WINHTTP_NO_HEADER_INDEX))
                                                                    {
                                                                        wchar_t *next_token;
                                                                        wchar_t* token = wcstok_s(responseHeadersTemp, L"\r\n", &next_token);
                                                                        while ((token != NULL) &&
                                                                            (token[0] != L'\0'))
                                                                        {
                                                                            char* tokenTemp;
                                                                            size_t tokenTemp_size;

                                                                            tokenTemp_size = WideCharToMultiByte(CP_ACP, 0, token, -1, NULL, 0, NULL, NULL);
                                                                            if (tokenTemp_size == 0)
                                                                            {
                                                                                LogError("WideCharToMultiByte failed");
                                                                            }
                                                                            else
                                                                            {
                                                                                tokenTemp = malloc(sizeof(char)*tokenTemp_size);
                                                                                if (tokenTemp == NULL)
                                                                                {
                                                                                    LogError("malloc failed");
                                                                                }
                                                                                else
                                                                                {
                                                                                    if (WideCharToMultiByte(CP_ACP, 0, token, -1, tokenTemp, (int)tokenTemp_size, NULL, NULL) > 0)
                                                                                    {
                                                                                        /*breaking the token in 2 parts: everything before the first ":" and everything after the first ":"*/
                                                                                        /* if there is no such character, then skip it*/
                                                                                        /*if there is a : then replace is by a '\0' and so it breaks the original string in name and value*/

                                                                                        char* whereIsColon = strchr(tokenTemp, ':');
                                                                                        if (whereIsColon != NULL)
                                                                                        {
                                                                                            *whereIsColon = '\0';
                                                                                            if (HTTPHeaders_AddHeaderNameValuePair(responseHeadersHandle, tokenTemp, whereIsColon + 1) != HTTP_HEADERS_OK)
                                                                                            {
                                                                                                LogError("HTTPHeaders_AddHeaderNameValuePair failed");
                                                                                                result = HTTPAPI_HTTP_HEADERS_FAILED;
                                                                                                break;
                                                                                            }
                                                                                        }
                                                                                    }
                                                                                    else
                                                                                    {
                                                                                        LogError("WideCharToMultiByte failed");
                                                                                    }
                                                                                    free(tokenTemp);
                                                                                }
                                                                            }


                                                                            token = wcstok_s(NULL, L"\r\n", &next_token);
                                                                        }
                                                                    }
                                                                    else
                                                                    {
                                                                        LogError("WinHttpQueryHeaders failed");
                                                                    }

                                                                    free(responseHeadersTemp);
                                                                }
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                        (void)WinHttpCloseHandle(requestHandle);
                                    }
                                }
                                free(headersTemp);
                            }
                        }
                        free(relativePathTemp);
                    }
                    free((void*)headers2);
                }
                else
                {
                    result = HTTPAPI_ALLOC_FAILED; /*likely*/
                    LogError("ConstructHeadersString failed");
                }
            }
        }
    }

    return result;
}

HTTPAPI_RESULT HTTPAPI_SetOption(HTTP_HANDLE handle, const char* optionName, const void* value)
{
    HTTPAPI_RESULT result;
    if (
        (handle == NULL) ||
        (optionName == NULL) ||
        (value == NULL)
        )
    {
        result = HTTPAPI_INVALID_ARG;
        LogError("invalid parameter (NULL) passed to HTTPAPI_SetOption");
    }
    else
    {
        HTTP_HANDLE_DATA* httpHandleData = (HTTP_HANDLE_DATA*)handle;
        if (strcmp("timeout", optionName) == 0)
        {
            long timeout = (long)(*(unsigned int*)value);
            httpHandleData->timeout = timeout;
            result = HTTPAPI_OK;
        }
        else if (strcmp("x509certificate", optionName) == 0)
        {
            httpHandleData->x509certificate = value;
            if (httpHandleData->x509privatekey != NULL)
            {
                httpHandleData->x509crypto = HTTPAPI_CreateCrypto(httpHandleData->x509certificate, httpHandleData->x509privatekey);
                if (httpHandleData->x509crypto == NULL)
                {
                    LogError("unable to HTTPAPI_CreateCrypto");
                    result = HTTPAPI_ERROR;
                }
                else
                {
                    result = HTTPAPI_OK;
                }
            }
            else
            {
                /*if certificate comes 1st and private key is not set yet, then return OK and wait for the private key to be set*/
                result = HTTPAPI_OK;
            }
        }
        else if (strcmp("x509privatekey", optionName) == 0)
        {
            httpHandleData->x509privatekey = value;
            if (httpHandleData->x509certificate != NULL)
            {
                httpHandleData->x509crypto = HTTPAPI_CreateCrypto(httpHandleData->x509certificate, httpHandleData->x509privatekey);
                if (httpHandleData->x509crypto == NULL)
                {
                    LogError("unable to HTTPAPI_CreateCrypto");
                    result = HTTPAPI_ERROR;
                }
                else
                {
                    result = HTTPAPI_OK;
                }
            }
            else
            {
                /*if privatekey comes 1st and certificate is not set yet, then return OK and wait for the certificate to be set*/
                result = HTTPAPI_OK;
            }
        }
        else
        {
            result = HTTPAPI_INVALID_ARG;
            LogError("unknown option %s", optionName);
        }
    }
    return result;
}

HTTPAPI_RESULT HTTPAPI_CloneOption(const char* optionName, const void* value, const void** savedValue)
{
    HTTPAPI_RESULT result;
    if (
        (optionName == NULL) ||
        (value == NULL) ||
        (savedValue == NULL)
        )
    {
        result = HTTPAPI_INVALID_ARG;
        LogError("invalid argument(NULL) passed to HTTPAPI_CloneOption");
    }
    else
    {
        if (strcmp("timeout", optionName) == 0)
        {
            /*by convention value is pointing to an unsigned int */
            unsigned int* temp = malloc(sizeof(unsigned int)); /*shall be freed by HTTPAPIEX*/
            if (temp == NULL)
            {
                result = HTTPAPI_ERROR;
                LogError("malloc failed (result = %s)", ENUM_TO_STRING(HTTPAPI_RESULT, result));
            }
            else
            {
                *temp = *(const unsigned int*)value;
                *savedValue = temp;
                result = HTTPAPI_OK;
            }
        }
        else if (strcmp("x509certificate", optionName) == 0)
        {
            /*this is getting the x509 certificate. In this case, value is a pointer to a const char* that contains the certificate as a null terminated string*/
            if (mallocAndStrcpy_s((char**)savedValue, value) != 0)
            {
                LogError("unable to clone the x509 certificate content");
                result = HTTPAPI_ERROR;
            }
            else
            {
                /*return OK when the certificate has been clones successfully*/
                result = HTTPAPI_OK;
            }
        }
        else if (strcmp("x509privatekey", optionName) == 0)
        {
            /*this is getting the x509 private key. In this case, value is a pointer to a const char* that contains the private key as a null terminated string*/
            if (mallocAndStrcpy_s((char**)savedValue, value) != 0)
            {
                LogError("unable to clone the x509 private key content");
                result = HTTPAPI_ERROR;
            }
            else
            {
                /*return OK when the private key has been clones successfully*/
                result = HTTPAPI_OK;
            }
        }
        else
        {
            result = HTTPAPI_INVALID_ARG;
            LogError("unknown option %s", optionName);
        }
    }
    return result;
}