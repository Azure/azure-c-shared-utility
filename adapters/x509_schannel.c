// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "windows.h"

#include <string.h>

#include "azure_c_shared_utility/gballoc.h"
#include "azure_c_shared_utility/x509_schannel.h"
#include "azure_c_shared_utility/xlogging.h"
#include "azure_c_shared_utility/safe_math.h"

#if _MSC_VER > 1500
#include <ncrypt.h>
#endif

#define KEY_NAME L"AzureAliasKey"
#define ECC_256_MAGIC_NUMBER        0x20
#define ECC_384_MAGIC_NUMBER        0x30

typedef enum x509_CERT_TYPE_TAG
{
    x509_TYPE_RSA,
    x509_TYPE_ECC,
    x509_TYPE_UNKNOWN,
} x509_CERT_TYPE;

typedef struct X509_SCHANNEL_HANDLE_DATA_TAG
{
    HCRYPTPROV hProv;
    HCRYPTKEY x509hcryptkey;
    PCCERT_CONTEXT x509certificate_context;
    x509_CERT_TYPE cert_type;
} X509_SCHANNEL_HANDLE_DATA;

/* The CryptDecodeObjectEx structure types are integers cast to LPCSTR, which MSVC flags as C4306
   on 64 bit builds. They are materialized once here so that the suppression stays in one place. */
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4306)
#endif // _MSC_VER

static const LPCSTR pkcs1_rsa_private_key_type = PKCS_RSA_PRIVATE_KEY;
static const LPCSTR pkcs8_private_key_info_type = PKCS_PRIVATE_KEY_INFO;
static const LPCSTR pkcs8_encrypted_private_key_info_type = PKCS_ENCRYPTED_PRIVATE_KEY_INFO;
#if _MSC_VER > 1500
static const LPCSTR sec1_ecc_private_key_type = X509_ECC_PRIVATE_KEY;
static const LPCSTR sequence_of_any_type = X509_SEQUENCE_OF_ANY;
static const LPCSTR octet_string_type = X509_OCTET_STRING;
#endif

#ifdef _MSC_VER
#pragma warning(pop)
#endif // _MSC_VER

static const char end_certificate_in_pem[] = "-----END CERTIFICATE-----";
static const size_t end_certificate_in_pem_length = sizeof(end_certificate_in_pem) - 1;
static const char pem_crlf_value[] = "\r\n";
static const size_t pem_crlf_value_length = sizeof(pem_crlf_value) - 1;


static unsigned char* convert_cert_to_binary(const char* crypt_value, DWORD crypt_value_in_len, DWORD* crypt_length)
{
    unsigned char* result = NULL;
    DWORD result_length = 0;
    if (!CryptStringToBinaryA(crypt_value, crypt_value_in_len, CRYPT_STRING_ANY, NULL, &result_length, NULL, NULL))
    {
        /*Codes_SRS_X509_SCHANNEL_02_010: [ Otherwise, x509_schannel_create shall fail and return a NULL X509_SCHANNEL_HANDLE. ]*/
        LogErrorWinHTTPWithGetLastErrorAsString("Failed determine crypt value size");
        result = NULL;
    }
    else
    {
        if ((result = (unsigned char*)malloc(result_length)) == NULL)
        {
            /*Codes_SRS_X509_SCHANNEL_02_010: [ Otherwise, x509_schannel_create shall fail and return a NULL X509_SCHANNEL_HANDLE. ]*/
            LogError("unable to allocate memory for crypt value");
        }
        else
        {
            if (!CryptStringToBinaryA(crypt_value, 0, CRYPT_STRING_ANY, result, &result_length, NULL, NULL))
            {
                /*Codes_SRS_X509_SCHANNEL_02_010: [ Otherwise, x509_schannel_create shall fail and return a NULL X509_SCHANNEL_HANDLE. ]*/
                LogErrorWinHTTPWithGetLastErrorAsString("Failed convert crypt value to binary");
                free(result);
                result = NULL;
            }
            else
            {
                if (crypt_length != NULL)
                {
                    *crypt_length = result_length;
                }
            }
        }
    }
    return result;
}

/* Decodes a DER encoded private key of a known structure type (PKCS_RSA_PRIVATE_KEY or
   X509_ECC_PRIVATE_KEY) into the key blob consumed further down by CryptImportKey/NCryptImportKey.
   The size of the resulting blob is returned in blob_size (when not NULL). */
static unsigned char* decode_private_key_blob(LPCSTR key_type, const unsigned char* private_key, DWORD key_length, DWORD* blob_size)
{
    unsigned char* result;
    DWORD private_key_blob_size = 0;

    /*Codes_SRS_X509_SCHANNEL_02_004: [ x509_schannel_create shall decode the private key by calling CryptDecodeObjectEx. ]*/
    if (!CryptDecodeObjectEx(X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, key_type, private_key, key_length, 0, NULL, NULL, &private_key_blob_size))
    {
        /*Codes_SRS_X509_SCHANNEL_02_010: [ Otherwise, x509_schannel_create shall fail and return a NULL X509_SCHANNEL_HANDLE. ]*/
        result = NULL;
    }
    else if ((result = (unsigned char*)malloc(private_key_blob_size)) == NULL)
    {
        /*Codes_SRS_X509_SCHANNEL_02_010: [ Otherwise, x509_schannel_create shall fail and return a NULL X509_SCHANNEL_HANDLE. ]*/
        LogError("unable to malloc for x509 private key blob");
    }
    else if (!CryptDecodeObjectEx(X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, key_type, private_key, key_length, 0, NULL, result, &private_key_blob_size))
    {
        /*Codes_SRS_X509_SCHANNEL_02_010: [ Otherwise, x509_schannel_create shall fail and return a NULL X509_SCHANNEL_HANDLE. ]*/
        LogErrorWinHTTPWithGetLastErrorAsString("Failed to CryptDecodeObjectEx x509 private key");
        free(result);
        result = NULL;
    }
    else if (blob_size != NULL)
    {
        *blob_size = private_key_blob_size;
    }

    return result;
}

#if _MSC_VER > 1500
/* RFC 5915 says the ECPrivateKey "parameters [0]" field MUST be omitted when the key is carried
   inside a PKCS#8 PrivateKeyInfo (the curve lives in the outer AlgorithmIdentifier), and that is
   what OpenSSL emits. Should the ASN.1 decoder reject that shape, rebuild the
   CRYPT_ECC_PRIVATE_KEY_INFO from the raw ECPrivateKey SEQUENCE instead: only the private key
   scalar is consumed downstream, the public point is taken from the certificate and the curve is
   derived from the scalar length. */
static unsigned char* decode_ecc_private_key_without_curve_oid(const unsigned char* ecc_private_key, DWORD ecc_private_key_length, DWORD* blob_size)
{
    unsigned char* result;
    CRYPT_SEQUENCE_OF_ANY* sequence = NULL;
    DWORD sequence_size = 0;

    if (!CryptDecodeObjectEx(X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, sequence_of_any_type, ecc_private_key, ecc_private_key_length, CRYPT_DECODE_ALLOC_FLAG, NULL, &sequence, &sequence_size))
    {
        LogErrorWinHTTPWithGetLastErrorAsString("Failed to CryptDecodeObjectEx the ECC private key sequence");
        result = NULL;
    }
    else if (sequence == NULL)
    {
        LogError("CryptDecodeObjectEx returned a NULL ECC private key sequence");
        result = NULL;
    }
    else
    {
        /*ECPrivateKey ::= SEQUENCE { version INTEGER, privateKey OCTET STRING, ... }*/
        CRYPT_DATA_BLOB* private_key_octets = NULL;
        DWORD private_key_octets_size = 0;

        if (sequence->cValue < 2)
        {
            LogError("ECC private key sequence has %u elements, at least 2 are required", (unsigned int)sequence->cValue);
            result = NULL;
        }
        else if (!CryptDecodeObjectEx(X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, octet_string_type, sequence->rgValue[1].pbData, sequence->rgValue[1].cbData, CRYPT_DECODE_ALLOC_FLAG, NULL, &private_key_octets, &private_key_octets_size) ||
            (private_key_octets == NULL))
        {
            LogErrorWinHTTPWithGetLastErrorAsString("Failed to CryptDecodeObjectEx the ECC private key scalar");
            result = NULL;
        }
        else
        {
            size_t key_info_size = safe_add_size_t(sizeof(CRYPT_ECC_PRIVATE_KEY_INFO), private_key_octets->cbData);

            if ((key_info_size == SIZE_MAX) || (key_info_size > MAXDWORD) ||
                ((result = (unsigned char*)malloc(key_info_size)) == NULL))
            {
                LogError("unable to malloc for the ECC private key info, size:%zu", key_info_size);
                result = NULL;
            }
            else
            {
                CRYPT_ECC_PRIVATE_KEY_INFO* key_info = (CRYPT_ECC_PRIVATE_KEY_INFO*)result;
                memset(key_info, 0, sizeof(CRYPT_ECC_PRIVATE_KEY_INFO));
                key_info->dwVersion = CRYPT_ECC_PRIVATE_KEY_INFO_v1;
                key_info->PrivateKey.cbData = private_key_octets->cbData;
                key_info->PrivateKey.pbData = result + sizeof(CRYPT_ECC_PRIVATE_KEY_INFO);
                (void)memcpy(key_info->PrivateKey.pbData, private_key_octets->pbData, private_key_octets->cbData);

                if (blob_size != NULL)
                {
                    *blob_size = (DWORD)key_info_size;
                }
            }
            (void)LocalFree(private_key_octets);
        }
        (void)LocalFree(sequence);
    }

    return result;
}
#endif

/* Handles a PKCS#8 PrivateKeyInfo ("-----BEGIN PRIVATE KEY-----"). The inner, algorithm specific
   private key is unwrapped and then decoded as PKCS#1 (RSA) or RFC 5915 (ECC).
   Returns NULL without logging when the input is not a PKCS#8 PrivateKeyInfo at all; is_pkcs8 tells
   the caller which of the two happened, so that it does not report an already reported failure. */
static unsigned char* decode_pkcs8_private_key(const unsigned char* private_key, DWORD key_length, DWORD* blob_size, x509_CERT_TYPE* cert_type, BOOL* is_pkcs8)
{
    unsigned char* result;
    CRYPT_PRIVATE_KEY_INFO* key_info = NULL;
    DWORD key_info_size = 0;

    *cert_type = x509_TYPE_UNKNOWN;
    *is_pkcs8 = FALSE;

    if (!CryptDecodeObjectEx(X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, pkcs8_private_key_info_type, private_key, key_length, CRYPT_DECODE_ALLOC_FLAG, NULL, &key_info, &key_info_size))
    {
        /*not a PKCS#8 PrivateKeyInfo, it is up to the caller to report this*/
        result = NULL;
    }
    else if (key_info == NULL)
    {
        *is_pkcs8 = TRUE;
        LogError("CryptDecodeObjectEx returned a NULL PKCS#8 private key info");
        result = NULL;
    }
    else
    {
        *is_pkcs8 = TRUE;

        if (key_info->Algorithm.pszObjId == NULL)
        {
            LogError("PKCS#8 private key does not carry an algorithm identifier");
            result = NULL;
        }
        else if (strcmp(key_info->Algorithm.pszObjId, szOID_RSA_RSA) == 0)
        {
            if ((result = decode_private_key_blob(pkcs1_rsa_private_key_type, key_info->PrivateKey.pbData, key_info->PrivateKey.cbData, blob_size)) == NULL)
            {
                /*Codes_SRS_X509_SCHANNEL_02_010: [ Otherwise, x509_schannel_create shall fail and return a NULL X509_SCHANNEL_HANDLE. ]*/
                LogErrorWinHTTPWithGetLastErrorAsString("Failed to decode the RSA private key wrapped in the PKCS#8 private key info");
            }
            else
            {
                *cert_type = x509_TYPE_RSA;
            }
        }
#if _MSC_VER > 1500
        else if (strcmp(key_info->Algorithm.pszObjId, szOID_ECC_PUBLIC_KEY) == 0)
        {
            if ((result = decode_private_key_blob(sec1_ecc_private_key_type, key_info->PrivateKey.pbData, key_info->PrivateKey.cbData, blob_size)) == NULL)
            {
                /*the ECPrivateKey carried by a PKCS#8 private key info has no curve OID of its own*/
                result = decode_ecc_private_key_without_curve_oid(key_info->PrivateKey.pbData, key_info->PrivateKey.cbData, blob_size);
            }
            if (result == NULL)
            {
                /*Codes_SRS_X509_SCHANNEL_02_010: [ Otherwise, x509_schannel_create shall fail and return a NULL X509_SCHANNEL_HANDLE. ]*/
                LogError("Failed to decode the ECC private key wrapped in the PKCS#8 private key info");
            }
            else
            {
                *cert_type = x509_TYPE_ECC;
            }
        }
#endif
        else
        {
            LogError("Unsupported PKCS#8 private key algorithm \"%s\", only RSA and ECC private keys are supported", key_info->Algorithm.pszObjId);
            result = NULL;
        }
        (void)LocalFree(key_info);
    }

    return result;
}

/*returns TRUE when the DER blob is a PKCS#8 EncryptedPrivateKeyInfo ("-----BEGIN ENCRYPTED PRIVATE KEY-----")*/
static BOOL is_encrypted_pkcs8_private_key(const unsigned char* private_key, DWORD key_length)
{
    BOOL result;
    void* encrypted_key_info = NULL;
    DWORD encrypted_key_info_size = 0;
    DWORD last_error = GetLastError();

    if (CryptDecodeObjectEx(X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, pkcs8_encrypted_private_key_info_type, private_key, key_length, CRYPT_DECODE_ALLOC_FLAG, NULL, &encrypted_key_info, &encrypted_key_info_size))
    {
        if (encrypted_key_info != NULL)
        {
            (void)LocalFree(encrypted_key_info);
        }
        result = TRUE;
    }
    else
    {
        /*the caller reports the error of the decode that actually failed, so this probe must not
          leave its own error behind*/
        SetLastError(last_error);
        result = FALSE;
    }
    return result;
}

static unsigned char* decode_crypt_object(unsigned char* private_key, DWORD key_length, DWORD* blob_size, x509_CERT_TYPE* cert_type)
{
    unsigned char* result;
    LPCSTR key_type = pkcs1_rsa_private_key_type;
    DWORD private_key_blob_size = 0;

    *cert_type = x509_TYPE_UNKNOWN;

    /*Codes_SRS_X509_SCHANNEL_02_004: [ x509_schannel_create shall decode the private key by calling CryptDecodeObjectEx. ]*/
    /*PKCS#1 RSAPrivateKey, "-----BEGIN RSA PRIVATE KEY-----"*/
    if (CryptDecodeObjectEx(X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, key_type, private_key, key_length, 0, NULL, NULL, &private_key_blob_size))
    {
        *cert_type = x509_TYPE_RSA;
    }
#if _MSC_VER > 1500
    else
    {
        /*RFC 5915 / SEC1 ECPrivateKey, "-----BEGIN EC PRIVATE KEY-----"*/
        key_type = sec1_ecc_private_key_type;
        if (CryptDecodeObjectEx(X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, key_type, private_key, key_length, 0, NULL, NULL, &private_key_blob_size))
        {
            *cert_type = x509_TYPE_ECC;
        }
    }
#endif

    if (*cert_type != x509_TYPE_UNKNOWN)
    {
        if ((result = (unsigned char*)malloc(private_key_blob_size)) == NULL)
        {
            /*Codes_SRS_X509_SCHANNEL_02_010: [ Otherwise, x509_schannel_create shall fail and return a NULL X509_SCHANNEL_HANDLE. ]*/
            LogError("unable to malloc for x509 private key blob");
        }
        else
        {
            if (!CryptDecodeObjectEx(X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, key_type, private_key, key_length, 0, NULL, result, &private_key_blob_size))
            {
                /*Codes_SRS_X509_SCHANNEL_02_010: [ Otherwise, x509_schannel_create shall fail and return a NULL X509_SCHANNEL_HANDLE. ]*/
                LogErrorWinHTTPWithGetLastErrorAsString("Failed to CryptDecodeObjectEx x509 private key");
                free(result);
                result = NULL;
            }
            else
            {
                if (blob_size != NULL)
                {
                    *blob_size = private_key_blob_size;
                }
            }
        }
    }
    /*PKCS#8 PrivateKeyInfo, "-----BEGIN PRIVATE KEY-----", the format emitted by current tooling*/
    else
    {
        BOOL is_pkcs8 = FALSE;

        if (((result = decode_pkcs8_private_key(private_key, key_length, blob_size, cert_type, &is_pkcs8)) == NULL) &&
            !is_pkcs8)
        {
            /*the key is in none of the supported encodings; a PKCS#8 private key info that failed to
              decode has already been reported in detail by decode_pkcs8_private_key*/
            /*Codes_SRS_X509_SCHANNEL_02_010: [ Otherwise, x509_schannel_create shall fail and return a NULL X509_SCHANNEL_HANDLE. ]*/
            if (is_encrypted_pkcs8_private_key(private_key, key_length))
            {
                LogError("The x509 private key is an encrypted PKCS#8 private key, which is not supported. Supply an unencrypted PKCS#8, PKCS#1 or EC private key instead");
            }
            else
            {
                LogErrorWinHTTPWithGetLastErrorAsString("Failed to CryptDecodeObjectEx x509 private key");
            }
        }
    }

    return result;
}

static int set_ecc_certificate_info(X509_SCHANNEL_HANDLE_DATA* x509_handle, unsigned char* x509privatekeyBlob)
{
    int result;
#if _MSC_VER > 1500
    BCRYPT_ECCKEY_BLOB* pKeyBlob;
    SECURITY_STATUS status;
    CRYPT_BIT_BLOB* pPubKeyBlob = &x509_handle->x509certificate_context->pCertInfo->SubjectPublicKeyInfo.PublicKey;
    CRYPT_ECC_PRIVATE_KEY_INFO* pPrivKeyInfo = (CRYPT_ECC_PRIVATE_KEY_INFO*)x509privatekeyBlob;
    DWORD pubSize = pPubKeyBlob->cbData - 1;
    DWORD privSize = pPrivKeyInfo->PrivateKey.cbData;
    size_t keyBlobSize = safe_add_size_t(safe_add_size_t(sizeof(BCRYPT_ECCKEY_BLOB), pubSize), privSize);
    BYTE* pubKeyBuf = pPubKeyBlob->pbData + 1;
    BYTE* privKeyBuf = pPrivKeyInfo->PrivateKey.pbData;

    if (keyBlobSize == SIZE_MAX ||
        (pKeyBlob = (BCRYPT_ECCKEY_BLOB*)malloc(keyBlobSize)) == NULL)
    {
        /*Codes_SRS_X509_SCHANNEL_02_010: [ Otherwise, x509_schannel_create shall fail and return a NULL X509_SCHANNEL_HANDLE. ]*/
        LogError("Failed to malloc NCrypt private key blob, size:%zu", keyBlobSize);
        result = MU_FAILURE;
    }
    else
    {
        pKeyBlob->dwMagic = privSize == ECC_256_MAGIC_NUMBER ? BCRYPT_ECDSA_PRIVATE_P256_MAGIC
            : privSize == ECC_384_MAGIC_NUMBER ? BCRYPT_ECDSA_PRIVATE_P384_MAGIC
            : BCRYPT_ECDSA_PRIVATE_P521_MAGIC;
        pKeyBlob->cbKey = privSize;
        memcpy((BYTE*)(pKeyBlob + 1), pubKeyBuf, pubSize);
        memcpy((BYTE*)(pKeyBlob + 1) + pubSize, privKeyBuf, privSize);

        /* Codes_SRS_X509_SCHANNEL_02_005: [ x509_schannel_create shall call CryptAcquireContext. ] */
        /* at this moment, both the private key and the certificate are decoded for further usage */
        /* NOTE: As no WinCrypt key storage provider supports ECC keys, NCrypt is used instead */
        status = NCryptOpenStorageProvider(&x509_handle->hProv, MS_KEY_STORAGE_PROVIDER, 0);
        if (status != ERROR_SUCCESS)
        {
            /* Codes_SRS_X509_SCHANNEL_02_010: [ Otherwise, x509_schannel_create shall fail and return a NULL X509_SCHANNEL_HANDLE. ]*/
            LogError("NCryptOpenStorageProvider failed with error 0x%08X", status);
            result = MU_FAILURE;
        }
        else
        {
            SECURITY_STATUS status2;
            NCryptBuffer ncBuf = { sizeof(KEY_NAME), NCRYPTBUFFER_PKCS_KEY_NAME, KEY_NAME };
            NCryptBufferDesc ncBufDesc;
            ncBufDesc.ulVersion = 0;
            ncBufDesc.cBuffers = 1;
            ncBufDesc.pBuffers = &ncBuf;

            CRYPT_KEY_PROV_INFO keyProvInfo = { KEY_NAME, MS_KEY_STORAGE_PROVIDER, 0, 0, 0, NULL, 0 };

            /*Codes_SRS_X509_SCHANNEL_02_006: [ x509_schannel_create shall import the private key by calling CryptImportKey. ] */
            /*NOTE: As no WinCrypt key storage provider supports ECC keys, NCrypt is used instead*/
            status = NCryptImportKey(x509_handle->hProv, 0, BCRYPT_ECCPRIVATE_BLOB, &ncBufDesc, &x509_handle->x509hcryptkey, (BYTE*)pKeyBlob, (DWORD)keyBlobSize, NCRYPT_OVERWRITE_KEY_FLAG);
            if (status == ERROR_SUCCESS)
            {
                status2 = NCryptFreeObject(x509_handle->x509hcryptkey);
                if (status2 != ERROR_SUCCESS)
                {
                    LogError("NCryptFreeObject for key handle failed with error 0x%08X", status2);
                }
                else
                {
                    x509_handle->x509hcryptkey = 0;
                }
            }

            status2 = NCryptFreeObject(x509_handle->hProv);
            if (status2 != ERROR_SUCCESS)
            {
                LogError("NCryptFreeObject for provider handle failed with error 0x%08X", status2);
            }
            else
            {
                x509_handle->hProv = 0;
            }

            if (status != ERROR_SUCCESS)
            {
                /*Codes_SRS_X509_SCHANNEL_02_010: [ Otherwise, x509_schannel_create shall fail and return a NULL X509_SCHANNEL_HANDLE. ]*/
                LogError("NCryptImportKey failed with error 0x%08X", status);
                result = MU_FAILURE;
            }
            else if (!CertSetCertificateContextProperty(x509_handle->x509certificate_context, CERT_KEY_PROV_INFO_PROP_ID, 0, &keyProvInfo))
            {
                /*Codes_SRS_X509_SCHANNEL_02_010: [ Otherwise, x509_schannel_create shall fail and return a NULL X509_SCHANNEL_HANDLE. ]*/
                LogErrorWinHTTPWithGetLastErrorAsString("CertSetCertificateContextProperty failed to set NCrypt key handle property");
                result = MU_FAILURE;
            }
            else
            {
                result = 0;
            }
        }
        free(pKeyBlob);
    }
#else
    (void)x509_handle;
    (void)x509privatekeyBlob;
    LogError("SChannel ECC is not supported in this compliation");
    result = MU_FAILURE;
#endif
    return result;
}

static int set_rsa_certificate_info(X509_SCHANNEL_HANDLE_DATA* x509_handle, unsigned char* x509privatekeyBlob, DWORD x509privatekeyBlobSize)
{
    int result;
    /*Codes_SRS_X509_SCHANNEL_02_005: [ x509_schannel_create shall call CryptAcquireContext. ]*/
    /*at this moment, both the private key and the certificate are decoded for further usage*/
    if (!CryptAcquireContextA(&(x509_handle->hProv), NULL, MS_ENH_RSA_AES_PROV_A, PROV_RSA_AES, CRYPT_VERIFYCONTEXT))
    {
        /*Codes_SRS_X509_SCHANNEL_02_010: [ Otherwise, x509_schannel_create shall fail and return a NULL X509_SCHANNEL_HANDLE. ]*/
        LogErrorWinHTTPWithGetLastErrorAsString("CryptAcquireContext failed");
        result = MU_FAILURE;
    }
    else
    {
        /*Codes_SRS_X509_SCHANNEL_02_006: [ x509_schannel_create shall import the private key by calling CryptImportKey. ] */
        if (!CryptImportKey(x509_handle->hProv, x509privatekeyBlob, x509privatekeyBlobSize, (HCRYPTKEY)NULL, 0, &(x509_handle->x509hcryptkey)))
        {
            /*Codes_SRS_X509_SCHANNEL_02_010: [ Otherwise, x509_schannel_create shall fail and return a NULL X509_SCHANNEL_HANDLE. ]*/
            LogErrorWinHTTPWithGetLastErrorAsString("CryptImportKey for private key failed");
            if (!CryptReleaseContext(x509_handle->hProv, 0))
            {
                LogErrorWinHTTPWithGetLastErrorAsString("unable to CryptReleaseContext");
            }
            result = MU_FAILURE;
        }
        else
        {
            /*Codes_SRS_X509_SCHANNEL_02_008: [ x509_schannel_create shall call set the certificate private key by calling CertSetCertificateContextProperty. ]*/
            if (!CertSetCertificateContextProperty(x509_handle->x509certificate_context, CERT_KEY_PROV_HANDLE_PROP_ID, 0, (void*)x509_handle->hProv))
            {
                /*Codes_SRS_X509_SCHANNEL_02_010: [ Otherwise, x509_schannel_create shall fail and return a NULL X509_SCHANNEL_HANDLE. ]*/
                LogErrorWinHTTPWithGetLastErrorAsString("unable to CertSetCertificateContextProperty");

                if (!CryptDestroyKey(x509_handle->x509hcryptkey))
                {
                    LogErrorWinHTTPWithGetLastErrorAsString("unable to CryptDestroyKey");
                }
                if (!CryptReleaseContext(x509_handle->hProv, 0))
                {
                    LogErrorWinHTTPWithGetLastErrorAsString("unable to CryptReleaseContext");
                }
                result = MU_FAILURE;
            }
            else
            {
                result = 0;
            }
        }
    }
    return result;
}

X509_SCHANNEL_HANDLE x509_schannel_create(const char* x509certificate, const char* x509privatekey)
{
    X509_SCHANNEL_HANDLE_DATA* result;
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

    /*Codes_SRS_X509_SCHANNEL_02_001: [ If x509certificate or x509privatekey are NULL then x509_schannel_create shall fail and return NULL. ]*/
    if (
        (x509certificate == NULL) ||
        (x509privatekey == NULL)
        )
    {
        /*Codes_SRS_X509_SCHANNEL_02_010: [ Otherwise, x509_schannel_create shall fail and return a NULL X509_SCHANNEL_HANDLE. ]*/
        LogError("invalid argument const char* x509certificate=%p, const char* x509privatekey=%p", x509certificate, x509privatekey);
        result = NULL;
    }
    else
    {
        unsigned char* binaryx509Certificate;
        unsigned char* binaryx509privatekey;
        unsigned char* x509privatekeyBlob;
        DWORD binaryx509certificateSize;
        DWORD binaryx509privatekeySize;
        DWORD x509privatekeyBlobSize;

        result = (X509_SCHANNEL_HANDLE_DATA*)malloc(sizeof(X509_SCHANNEL_HANDLE_DATA));
        if (result == NULL)
        {
            LogError("unable to malloc");
            /*Codes_SRS_X509_SCHANNEL_02_010: [ Otherwise, x509_schannel_create shall fail and return a NULL X509_SCHANNEL_HANDLE. ]*/
            /*return as is*/

        }
        else
        {
            memset(result, 0, sizeof(X509_SCHANNEL_HANDLE_DATA));
            /*Codes_SRS_X509_SCHANNEL_02_002: [ x509_schannel_create shall convert the certificate to binary form by calling CryptStringToBinaryA. ]*/
            if ((binaryx509Certificate = convert_cert_to_binary(x509certificate, 0, &binaryx509certificateSize)) == NULL)
            {
                LogError("Failure converting x509 certificate");
                free(result);
                result = NULL;
            }
            /*Codes_SRS_X509_SCHANNEL_02_003: [ x509_schannel_create shall convert the private key to binary form by calling CryptStringToBinaryA. ]*/
            /*at this moment x509 certificate is ready to be used in CertCreateCertificateContext*/
            else if ((binaryx509privatekey = convert_cert_to_binary(x509privatekey, 0, &binaryx509privatekeySize)) == NULL)
            {
                LogError("Failure converting x509 certificate");
                free(binaryx509Certificate);
                free(result);
                result = NULL;
            }
            else if ((x509privatekeyBlob = decode_crypt_object(binaryx509privatekey, binaryx509privatekeySize, &x509privatekeyBlobSize, &result->cert_type)) == NULL)
            {
                LogError("Failure decoding x509 private key");
                free(binaryx509Certificate);
                free(binaryx509privatekey);
                free(result);
                result = NULL;
            }
            else
            {
                /*Codes_SRS_X509_SCHANNEL_02_007: [ x509_schannel_create shall create a cerficate context by calling CertCreateCertificateContext. ]*/
                PCCERT_CONTEXT cert_ctx = CertCreateCertificateContext(X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, binaryx509Certificate, binaryx509certificateSize);
                if ((result->x509certificate_context = cert_ctx) == NULL)
                {
                    /*Codes_SRS_X509_SCHANNEL_02_010: [ Otherwise, x509_schannel_create shall fail and return a NULL X509_SCHANNEL_HANDLE. ]*/
                    LogErrorWinHTTPWithGetLastErrorAsString("unable to CertCreateCertificateContext");
                    free(result);
                    result = NULL;
                }
                else
                {
                    /* Codes_SRS_X509_SCHANNEL_07_001: [ x509_schannel_create shall determine whether the certificate is of type RSA or ECC. ] */
                    if (result->cert_type == x509_TYPE_RSA)
                    {
                        if (set_rsa_certificate_info(result, x509privatekeyBlob, x509privatekeyBlobSize) != 0)
                        {
                            (void)CertFreeCertificateContext(result->x509certificate_context);
                            free(result);
                            result = NULL;
                        }
                    }
                    else
                    {
                        if (set_ecc_certificate_info(result, x509privatekeyBlob) != 0)
                        {
                            (void)CertFreeCertificateContext(result->x509certificate_context);
                            free(result);
                            result = NULL;
                        }
                    }
                }
                free(x509privatekeyBlob);
                free(binaryx509privatekey);
                free(binaryx509Certificate);
            }
        }
    }
    return result;
}

void x509_schannel_destroy(X509_SCHANNEL_HANDLE x509_schannel_handle)
{
    /*Codes_SRS_X509_SCHANNEL_02_011: [ If parameter x509_schannel_handle is NULL then x509_schannel_destroy shall do nothing. ]*/
    if (x509_schannel_handle != NULL)
    {
        /*Codes_SRS_X509_SCHANNEL_02_012: [ Otherwise, x509_schannel_destroy shall free all used resources. ]*/
        X509_SCHANNEL_HANDLE_DATA* x509crypto = (X509_SCHANNEL_HANDLE_DATA*)x509_schannel_handle;

        if (x509crypto->cert_type == x509_TYPE_RSA)
        {
            if (!CryptDestroyKey(x509crypto->x509hcryptkey))
            {
                LogErrorWinHTTPWithGetLastErrorAsString("unable to CryptDestroyKey");
            }
            if (!CryptReleaseContext(x509crypto->hProv, 0))
            {
                LogErrorWinHTTPWithGetLastErrorAsString("unable to CryptReleaseContext");
            }
        }
        else
        {
#if _MSC_VER > 1500
            if (x509crypto->x509hcryptkey != 0)
            {
                (void)NCryptFreeObject(x509crypto->x509hcryptkey);
            }
            if (x509crypto->hProv != 0)
            {
                (void)NCryptFreeObject(x509crypto->hProv);
            }
#endif
        }
        if (!CertFreeCertificateContext(x509crypto->x509certificate_context))
        {
            LogErrorWinHTTPWithGetLastErrorAsString("unable to CertFreeCertificateContext");
        }
        free(x509crypto);
    }
}

PCCERT_CONTEXT x509_schannel_get_certificate_context(X509_SCHANNEL_HANDLE x509_schannel_handle)
{
    PCCERT_CONTEXT result;
    if (x509_schannel_handle == NULL)
    {
        /*Codes_SRS_X509_SCHANNEL_02_013: [ If parameter x509_schannel_handle is NULL then x509_schannel_get_certificate_context shall return NULL. ]*/
        result = NULL;
    }
    else
    {
        /*Codes_SRS_X509_SCHANNEL_02_014: [ Otherwise, x509_schannel_get_certificate_context shall return a non-NULL PCCERT_CONTEXT pointer. ]*/
        X509_SCHANNEL_HANDLE_DATA* handleData = (X509_SCHANNEL_HANDLE_DATA*)x509_schannel_handle;
        result = handleData->x509certificate_context;
    }
    return result;
}

// For each certificate specified in trustedCertificate, add to hCertStore.  Windows API's (namely
// CryptStringToBinaryA & CertAddEncodedCertificateToStore) do not handle multiple certificates
// at a time in a single call, so add_certificates_to_store() parses the PEM (delimited by "-----END CERTIFICATE-----")
// to call Windows API a cert at a time.
static int add_certificates_to_store(const char* trustedCertificate, HCERTSTORE hCertStore)
{
    int result = 0;
    int numCertificatesAdded = 0;
    const char* trustedCertCurrentRead = trustedCertificate;
    unsigned char* trustedCertificateEncoded = NULL;

    while (result == 0)
    {
        const char* endCertificateCurrentRead;
        DWORD trustedCertificateEncodedLen;
        DWORD lastError;

        if ((endCertificateCurrentRead = strstr(trustedCertCurrentRead, end_certificate_in_pem)) == NULL)
        {
            if (numCertificatesAdded == 0)
            {
                LogError("Certificate missing closing %s.  No certificates can be added to stare", end_certificate_in_pem);
                result = MU_FAILURE;
            }
            break;
        }
        
        endCertificateCurrentRead += end_certificate_in_pem_length;
        if (strncmp(endCertificateCurrentRead, pem_crlf_value, pem_crlf_value_length) == 0)
        {
            endCertificateCurrentRead += pem_crlf_value_length;
        }
        
        if ((trustedCertificateEncoded = convert_cert_to_binary(trustedCertCurrentRead, (DWORD)(endCertificateCurrentRead - trustedCertCurrentRead), &trustedCertificateEncodedLen)) == NULL)
        {
            LogError("Cannot encode trusted certificate");
            result = MU_FAILURE;
        }
        else if (CertAddEncodedCertificateToStore(hCertStore, X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, trustedCertificateEncoded, trustedCertificateEncodedLen, CERT_STORE_ADD_NEW, NULL) != TRUE)
        {
            lastError = GetLastError();
            LogError("CertAddEncodedCertificateToStore failed with error 0x%08x", lastError);
            result = MU_FAILURE;
        }

        if (trustedCertificateEncoded != NULL)
        {   
            free(trustedCertificateEncoded);
        }

        trustedCertCurrentRead = endCertificateCurrentRead;
        numCertificatesAdded++;
    }

    return result;
}

// x509_verify_certificate_in_chain determines whether the certificate in pCertContextToVerify
// chains up to the PEM represented by trustedCertificate or not.
int x509_verify_certificate_in_chain(const char* trustedCertificate, PCCERT_CONTEXT pCertContextToVerify)
{
    int result;
    HCERTSTORE hCertStore = NULL;
    HCERTCHAINENGINE hChainEngine = NULL;
    PCCERT_CHAIN_CONTEXT pChainContextToVerify = NULL;
    DWORD lastError;

    if ((trustedCertificate == NULL) || (pCertContextToVerify == NULL))
    {
        result = MU_FAILURE;
    }
    // Creates an in-memory certificate store that is destroyed at end of this function.
    else if (NULL == (hCertStore = CertOpenStore(CERT_STORE_PROV_MEMORY, X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, 0, CERT_STORE_CREATE_NEW_FLAG, NULL)))
    {
        lastError = GetLastError();
        LogError("CertOpenStore failed with error 0x%08x", lastError);
        result = MU_FAILURE;
    }
    else if (add_certificates_to_store(trustedCertificate, hCertStore) != 0)
    {
        LogError("Cannot add certificates to store");
        result = MU_FAILURE;
    }
    else
    {
        CERT_CHAIN_ENGINE_CONFIG EngineConfig;
        memset(&EngineConfig, 0, sizeof(EngineConfig));
        EngineConfig.cbSize = sizeof(EngineConfig);
        EngineConfig.dwFlags = CERT_CHAIN_ENABLE_CACHE_AUTO_UPDATE | CERT_CHAIN_ENABLE_SHARE_STORE;
        EngineConfig.hExclusiveRoot = hCertStore;

        CERT_CHAIN_PARA ChainPara;
        memset(&ChainPara, 0, sizeof(ChainPara));
        ChainPara.cbSize = sizeof(ChainPara);

        CERT_CHAIN_POLICY_PARA PolicyPara;
        memset(&PolicyPara, 0, sizeof(PolicyPara));
        PolicyPara.cbSize = sizeof(PolicyPara);

        CERT_CHAIN_POLICY_STATUS PolicyStatus;
        memset(&PolicyStatus, 0, sizeof(PolicyStatus));
        PolicyStatus.cbSize = sizeof(PolicyStatus);

        if (CertCreateCertificateChainEngine(&EngineConfig, &hChainEngine) != TRUE)
        {
            lastError = GetLastError();
            LogError("CertCreateCertificateChainEngine failed with error 0x%08x", lastError);
            result = MU_FAILURE;
        }
        else if (CertGetCertificateChain(hChainEngine, pCertContextToVerify, NULL, pCertContextToVerify->hCertStore, &ChainPara, 0, NULL, &pChainContextToVerify) != TRUE)
        {
            lastError = GetLastError();
            LogError("CertGetCertificateChain failed with error 0x%08x", lastError);
            result = MU_FAILURE;
        }
        else if (CertVerifyCertificateChainPolicy(CERT_CHAIN_POLICY_SSL, pChainContextToVerify, &PolicyPara, &PolicyStatus) != TRUE)
        {
            lastError = GetLastError();
            LogError("CertVerifyCertificateChainPolicy failed with error 0x%08x", lastError);
            result = MU_FAILURE;
        }
        else if (PolicyStatus.dwError != 0)
        {
            LogError("CertVerifyCertificateChainPolicy sets certificateStatus = 0x%08x", PolicyStatus.dwError);
            result = MU_FAILURE;
        }
        else
        {
            result = 0;
        }
    }

    if (pChainContextToVerify != NULL)
    {
        CertFreeCertificateChain(pChainContextToVerify);
    }

    if (hChainEngine != NULL)
    {
        CertFreeCertificateChainEngine(hChainEngine);
    }

    if (hCertStore != NULL)
    {
        CertCloseStore(hCertStore, 0);
    }

    return result;
}
