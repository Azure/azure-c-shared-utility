// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef WIN32
#error x509_schannel_unittests can only be compiled and used in Windows
#else

/*the following #defines will make "inconsistent dll linkage" warning go away (that is, it takes away declspec(dllexport) */
#define WINCRYPT32API
#define WINADVAPI

#include "windows.h"

#include <stdlib.h>
#ifdef _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

static void* my_gballoc_malloc(size_t size)
{
    return malloc(size);
}

static void my_gballoc_free(void* s)
{
    free(s);
}

/*define this symbol so that CryptDecodeObjectEx is not linked with dll linkage*/

#include <stddef.h>
#include "testrunnerswitcher.h"
#include "umock_c.h"

#include "azure_c_shared_utility/x509_schannel.h"
#include "umocktypes_charptr.h"
#include "umock_c_negative_tests.h"

#define ENABLE_MOCKS
#include "azure_c_shared_utility/gballoc.h"

#include "azure_c_shared_utility/umock_c_prod.h"
MOCKABLE_FUNCTION(WINAPI, BOOL, CryptDecodeObjectEx, 
          DWORD             ,dwCertEncodingType,
          LPCSTR            ,lpszStructType,
    const BYTE              *,pbEncoded,
          DWORD             ,cbEncoded,
          DWORD             ,dwFlags,
          PCRYPT_DECODE_PARA,pDecodePara,
          void              *,pvStructInfo,
          DWORD             *,pcbStructInfo
);

MOCKABLE_FUNCTION(WINAPI, PCCERT_CONTEXT, CertCreateCertificateContext,
          DWORD ,dwCertEncodingType,
    const BYTE  *,pbCertEncoded,
          DWORD, cbCertEncoded
);

MOCKABLE_FUNCTION(WINAPI, BOOL, CertFreeCertificateContext, 
    PCCERT_CONTEXT, pCertContext
);

MOCKABLE_FUNCTION(WINAPI, BOOL , CertSetCertificateContextProperty,
PCCERT_CONTEXT, pCertContext,
DWORD, dwPropId,
DWORD, dwFlags,
const void           *, pvData
);

MOCKABLE_FUNCTION(WINAPI, BOOL, CryptStringToBinaryA,
    LPCTSTR, pszString,
    DWORD, cchString,
    DWORD, dwFlags,
    BYTE    *, pbBinary,
    DWORD   *, pcbBinary,
    DWORD   *, pdwSkip,
    DWORD   *, pdwFlags
);

MOCKABLE_FUNCTION(WINAPI, BOOL, CryptAcquireContextA,
    HCRYPTPROV  *, phProv,
    LPCTSTR, szContainer,
    LPCTSTR, szProvider,
    DWORD, dwProvType,
    DWORD, dwFlags
);

MOCKABLE_FUNCTION(WINAPI, BOOL, CryptReleaseContext,
    HCRYPTPROV, hProv,
    DWORD, dwFlags
);

MOCKABLE_FUNCTION(WINAPI, BOOL, CryptDestroyKey,
    HCRYPTKEY, hKey
);

MOCKABLE_FUNCTION(WINAPI, BOOL, CryptImportKey,
    HCRYPTPROV, hProv,
    CONST BYTE  *, pbData,
    DWORD, dwDataLen,
    HCRYPTKEY, hPubKey,
    DWORD, dwFlags,
    HCRYPTKEY   *, phKey
);


#undef ENABLE_MOCKS

static TEST_MUTEX_HANDLE g_testByTest;
static TEST_MUTEX_HANDLE g_dllByDll;

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
(void)error_code;
ASSERT_FAIL("umock_c reported error");
}

static BOOL my_CryptStringToBinaryA(
    LPCTSTR pszString,
    DWORD cchString,
    DWORD dwFlags,
    BYTE    *pbBinary,
    DWORD   *pcbBinary,
    DWORD   *pdwSkip,
    DWORD   *pdwFlags
)
{
    (void)(pszString, cchString, dwFlags, pdwSkip, pdwFlags);
    *pcbBinary = 1; /*the binary form always has 1 byte*/
    if (pbBinary != NULL)
    {
        *pbBinary = (BYTE)'3';
    }
    return TRUE;
}

static BOOL my_CryptDecodeObjectEx(
    DWORD dwCertEncodingType,
    LPCSTR lpszStructType,
    const BYTE              * pbEncoded,
    DWORD cbEncoded,
    DWORD dwFlags,
    PCRYPT_DECODE_PARA pDecodePara,
    void              * pvStructInfo,
    DWORD             * pcbStructInfo
)
{
    (void)(pDecodePara, dwFlags, cbEncoded, pbEncoded, pvStructInfo, lpszStructType, dwCertEncodingType);
    if (pcbStructInfo != NULL)
    {
        *pcbStructInfo = 2; /*assume the decoded size is 2*/
    }

    return TRUE;
}

static BOOL my_CryptAcquireContextA(
    HCRYPTPROV  *phProv,
    LPCTSTR szContainer,
    LPCTSTR szProvider,
    DWORD dwProvType,
    DWORD dwFlags
)
{
    (void)(szContainer, szProvider, dwProvType, dwFlags);
    *phProv = (HCRYPTPROV)my_gballoc_malloc(3);
    return TRUE;
}

static BOOL my_CryptImportKey(
    HCRYPTPROV hProv,
    CONST BYTE  * pbData,
    DWORD dwDataLen,
    HCRYPTKEY hPubKey,
    DWORD dwFlags,
    HCRYPTKEY   * phKey
)
{
    (void)(hProv, pbData, dwDataLen, hPubKey, dwFlags);
    *phKey = (HCRYPTKEY)my_gballoc_malloc(4);
    return TRUE;
}

static PCCERT_CONTEXT  my_CertCreateCertificateContext(
    DWORD dwCertEncodingType,
    const BYTE  * pbCertEncoded,
    DWORD cbCertEncoded
)
{
    (void)(dwCertEncodingType, pbCertEncoded, cbCertEncoded);
    return (PCCERT_CONTEXT)my_gballoc_malloc(5);
}

static BOOL my_CryptReleaseContext(
    HCRYPTPROV hProv,
    DWORD dwFlags
)
{
    (void)dwFlags;
    my_gballoc_free((void*)hProv);
    return TRUE;
}

static BOOL my_CryptDestroyKey(
    HCRYPTKEY hKey
)
{
    my_gballoc_free((void*)hKey);
    return TRUE;
}

static BOOL my_CertFreeCertificateContext(
    PCCERT_CONTEXT pCertContext
)
{
    my_gballoc_free((void*)pCertContext);
    return TRUE;
}

static BOOL my_CertSetCertificateContextProperty(
    PCCERT_CONTEXT pCertContext,
    DWORD dwPropId,
    DWORD dwFlags,
    const void           * pvData
)
{
    (void)(pCertContext, dwPropId, dwFlags, pvData);
    return TRUE;
}

BEGIN_TEST_SUITE(x509_schannel_unittests)

    TEST_SUITE_INITIALIZE(a)
    {
        TEST_INITIALIZE_MEMORY_DEBUG(g_dllByDll);
        g_testByTest = TEST_MUTEX_CREATE();
        ASSERT_IS_NOT_NULL(g_testByTest);

        (void)umock_c_init(on_umock_c_error);

        (void)umocktypes_charptr_register_types();

        REGISTER_GLOBAL_MOCK_HOOK(gballoc_malloc, my_gballoc_malloc);
        REGISTER_GLOBAL_MOCK_FAIL_RETURN(gballoc_malloc, NULL);
        REGISTER_GLOBAL_MOCK_HOOK(gballoc_free, my_gballoc_free);

        REGISTER_UMOCK_ALIAS_TYPE(LPCTSTR, const char*);
        REGISTER_UMOCK_ALIAS_TYPE(LPCSTR, void*);
        REGISTER_UMOCK_ALIAS_TYPE(PCRYPT_DECODE_PARA, void*);
        REGISTER_UMOCK_ALIAS_TYPE(HCRYPTPROV, void*);
        REGISTER_UMOCK_ALIAS_TYPE(HCRYPTKEY, void*);
        REGISTER_UMOCK_ALIAS_TYPE(PCCERT_CONTEXT, void*);
        
        REGISTER_UMOCK_ALIAS_TYPE(DWORD, unsigned int);
        
        REGISTER_GLOBAL_MOCK_HOOK(CryptStringToBinaryA, my_CryptStringToBinaryA);
        REGISTER_GLOBAL_MOCK_FAIL_RETURN(CryptStringToBinaryA, FALSE);

        REGISTER_GLOBAL_MOCK_HOOK(CryptDecodeObjectEx, my_CryptDecodeObjectEx);
        REGISTER_GLOBAL_MOCK_FAIL_RETURN(CryptDecodeObjectEx, FALSE);
        
        REGISTER_GLOBAL_MOCK_HOOK(CryptAcquireContextA, my_CryptAcquireContextA);
        REGISTER_GLOBAL_MOCK_FAIL_RETURN(CryptAcquireContextA, FALSE);

        REGISTER_GLOBAL_MOCK_HOOK(CryptImportKey, my_CryptImportKey);
        REGISTER_GLOBAL_MOCK_FAIL_RETURN(CryptImportKey, FALSE);
        REGISTER_GLOBAL_MOCK_HOOK(CryptDestroyKey, my_CryptDestroyKey);
        REGISTER_GLOBAL_MOCK_FAIL_RETURN(CryptDestroyKey, FALSE);

        REGISTER_GLOBAL_MOCK_HOOK(CertCreateCertificateContext, my_CertCreateCertificateContext);
        REGISTER_GLOBAL_MOCK_FAIL_RETURN(CertCreateCertificateContext, NULL);

        REGISTER_GLOBAL_MOCK_HOOK(CryptReleaseContext, my_CryptReleaseContext);
        REGISTER_GLOBAL_MOCK_FAIL_RETURN(CryptReleaseContext, FALSE);

        REGISTER_GLOBAL_MOCK_HOOK(CertSetCertificateContextProperty, my_CertSetCertificateContextProperty);
        REGISTER_GLOBAL_MOCK_FAIL_RETURN(CertSetCertificateContextProperty, FALSE);

        REGISTER_GLOBAL_MOCK_HOOK(CertFreeCertificateContext, my_CertFreeCertificateContext);

    }

    TEST_SUITE_CLEANUP(TestClassCleanup)
    {
        umock_c_deinit();

        TEST_MUTEX_DESTROY(g_testByTest);
        TEST_DEINITIALIZE_MEMORY_DEBUG(g_dllByDll);
    }

    TEST_FUNCTION_INITIALIZE(initialize)
    {
        umock_c_reset_all_calls();
    }

    TEST_FUNCTION_CLEANUP(cleans)
    {

    }

    /*Tests_SRS_X509_SCHANNEL_02_001: [ If x509certificate or x509privatekey are NULL then x509_schannel_create shall fail and return NULL. ]*/
    TEST_FUNCTION(x509_schannel_create_with_NULL_x509certificate_fails)
    {
        ///arrange

        ///act
        X509_SCHANNEL_HANDLE h = x509_schannel_create(NULL, "private key");

        ///assert
        ASSERT_IS_NULL(h);

        ///cleanup
    }

    /*Tests_SRS_X509_SCHANNEL_02_001: [ If x509certificate or x509privatekey are NULL then x509_schannel_create shall fail and return NULL. ]*/
    TEST_FUNCTION(x509_schannel_create_with_NULL_x509privatekey_fails)
    {
        ///arrange

        ///act
        X509_SCHANNEL_HANDLE h = x509_schannel_create("certificate", NULL);

        ///assert
        ASSERT_IS_NULL(h);

        ///cleanup
    }

    /*Tests_SRS_X509_SCHANNEL_02_002: [ x509_schannel_create shall convert the certificate to binary form by calling CryptStringToBinaryA. ]*/
    /*Tests_SRS_X509_SCHANNEL_02_003: [ x509_schannel_create shall convert the private key to binary form by calling CryptStringToBinaryA. ]*/
    /*Tests_SRS_X509_SCHANNEL_02_004: [ x509_schannel_create shall decode the private key by calling CryptDecodeObjectEx. ]*/
    /*Tests_SRS_X509_SCHANNEL_02_005: [ x509_schannel_create shall call CryptAcquireContext. ]*/
    /*Tests_SRS_X509_SCHANNEL_02_006: [ x509_schannel_create shall import the private key by calling CryptImportKey. ]*/
    /*Tests_SRS_X509_SCHANNEL_02_007: [ x509_schannel_create shall create a cerficate context by calling CertCreateCertificateContext. ]*/
    /*Tests_SRS_X509_SCHANNEL_02_008: [ x509_schannel_create shall call set the certificate private key by calling CertSetCertificateContextProperty. ]*/
    /*Tests_SRS_X509_SCHANNEL_02_009: [ If all the operations above succeed, then x509_schannel_create shall succeeds and return a non-NULL X509_SCHANNEL_HANDLE. ]*/
    TEST_FUNCTION(x509_schannel_create_succeeds)
    {
        ///arrange

        STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG)) /*this is creating the handle storage space*/
            .IgnoreArgument_size();
        STRICT_EXPECTED_CALL(CryptStringToBinaryA("certificate", 0, CRYPT_STRING_ANY, NULL, IGNORED_PTR_ARG, NULL, NULL)) /*this is asking for "how big is the certificate binary size?"*/
            .IgnoreArgument_pcbBinary();
        STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG)) /*this is creating the binary storage for the certificate*/
            .IgnoreArgument_size();
        STRICT_EXPECTED_CALL(CryptStringToBinaryA("certificate", 0, CRYPT_STRING_ANY, IGNORED_PTR_ARG, IGNORED_PTR_ARG, NULL, NULL)) /*this is asking for "fill in the certificate in this binary buffer"*/
            .IgnoreArgument_pcbBinary() 
            .IgnoreArgument_pbBinary();

        STRICT_EXPECTED_CALL(CryptStringToBinaryA("private key", 0, CRYPT_STRING_ANY, NULL, IGNORED_PTR_ARG, NULL, NULL)) /*this is asking for "how big is the private key binary size?"*/
            .IgnoreArgument_pcbBinary();
        STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG)) /*this is creating the binary storage for the private key*/
            .IgnoreArgument_size();
        STRICT_EXPECTED_CALL(CryptStringToBinaryA("private key", 0, CRYPT_STRING_ANY, IGNORED_PTR_ARG, IGNORED_PTR_ARG, NULL, NULL)) /*this is asking for "fill in the private key in this binary buffer"*/
            .IgnoreArgument_pcbBinary()
            .IgnoreArgument_pbBinary();

        STRICT_EXPECTED_CALL(CryptDecodeObjectEx(X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, PKCS_RSA_PRIVATE_KEY, IGNORED_PTR_ARG, IGNORED_NUM_ARG, 0, NULL, NULL, IGNORED_PTR_ARG)) /*this is asking "how big is the decoded private key? (from binary)*/
            .IgnoreArgument_pbEncoded()
            .IgnoreArgument_cbEncoded()
            .IgnoreArgument_pcbStructInfo();
        STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG)) /*this is allocating space for the decoded private key*/
            .IgnoreArgument_size();
        STRICT_EXPECTED_CALL(CryptDecodeObjectEx(X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, PKCS_RSA_PRIVATE_KEY, IGNORED_PTR_ARG, IGNORED_NUM_ARG, 0, NULL, IGNORED_PTR_ARG, IGNORED_PTR_ARG)) /*this is asking "how big is the decoded private key? (from binary)*/
            .IgnoreArgument_pbEncoded()
            .IgnoreArgument_cbEncoded()
            .IgnoreArgument_pvStructInfo()
            .IgnoreArgument_pcbStructInfo();

        STRICT_EXPECTED_CALL(CryptAcquireContextA(IGNORED_PTR_ARG, NULL, MS_ENHANCED_PROV, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)) /*this is acquire a handle to a key container within a cryptographic service provider*/
            .IgnoreArgument_phProv();

        STRICT_EXPECTED_CALL(CryptImportKey((HCRYPTPROV)IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG, (HCRYPTKEY)NULL, 0, IGNORED_PTR_ARG)) /*tranferring the key from the blob to the cryptrographic key provider*/
            .IgnoreArgument_hProv()
            .IgnoreArgument_pbData()
            .IgnoreArgument_dwDataLen()
            .IgnoreArgument_phKey();

        STRICT_EXPECTED_CALL(CertCreateCertificateContext(X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, IGNORED_PTR_ARG, IGNORED_NUM_ARG)) /*create a certificate context from an encoded certificate*/
            .IgnoreArgument_cbCertEncoded()
            .IgnoreArgument_pbCertEncoded();

        STRICT_EXPECTED_CALL(CertSetCertificateContextProperty(IGNORED_PTR_ARG, CERT_KEY_PROV_HANDLE_PROP_ID, 0, IGNORED_PTR_ARG)) /*give the private key*/
            .IgnoreArgument_pCertContext()
            .IgnoreArgument_pvData();

        STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG))
            .IgnoreArgument_ptr();

        STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG))
            .IgnoreArgument_ptr();

        STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG))
            .IgnoreArgument_ptr();

        ///act
        X509_SCHANNEL_HANDLE h = x509_schannel_create("certificate", "private key");
        
        ///assert
        ASSERT_IS_NOT_NULL(h);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        x509_schannel_destroy(h);
    }

    /*Tests_SRS_X509_SCHANNEL_02_010: [ Otherwise, x509_schannel_create shall fail and return a NULL X509_SCHANNEL_HANDLE. ]*/
    TEST_FUNCTION(x509_schannel_negative_test_cases)
    {
        ///arrange

        int negativeTestsInitResult = umock_c_negative_tests_init();
        ASSERT_ARE_EQUAL(int, 0, negativeTestsInitResult);

        STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG)) /*this is creating the handle storage space*/
            .IgnoreArgument_size();
        STRICT_EXPECTED_CALL(CryptStringToBinaryA("certificate", 0, CRYPT_STRING_ANY, NULL, IGNORED_PTR_ARG, NULL, NULL)) /*this is asking for "how big is the certificate binary size?"*/
            .IgnoreArgument_pcbBinary();
        STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG)) /*this is creating the binary storage for the certificate*/
            .IgnoreArgument_size();
        STRICT_EXPECTED_CALL(CryptStringToBinaryA("certificate", 0, CRYPT_STRING_ANY, IGNORED_PTR_ARG, IGNORED_PTR_ARG, NULL, NULL)) /*this is asking for "fill in the certificate in this binary buffer"*/
            .IgnoreArgument_pcbBinary()
            .IgnoreArgument_pbBinary();

        STRICT_EXPECTED_CALL(CryptStringToBinaryA("private key", 0, CRYPT_STRING_ANY, NULL, IGNORED_PTR_ARG, NULL, NULL)) /*this is asking for "how big is the private key binary size?"*/
            .IgnoreArgument_pcbBinary();
        STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG)) /*this is creating the binary storage for the private key*/
            .IgnoreArgument_size();
        STRICT_EXPECTED_CALL(CryptStringToBinaryA("private key", 0, CRYPT_STRING_ANY, IGNORED_PTR_ARG, IGNORED_PTR_ARG, NULL, NULL)) /*this is asking for "fill in the private key in this binary buffer"*/
            .IgnoreArgument_pcbBinary()
            .IgnoreArgument_pbBinary();

        STRICT_EXPECTED_CALL(CryptDecodeObjectEx(X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, PKCS_RSA_PRIVATE_KEY, IGNORED_PTR_ARG, IGNORED_NUM_ARG, 0, NULL, NULL, IGNORED_PTR_ARG)) /*this is asking "how big is the decoded private key? (from binary)*/
            .IgnoreArgument_pbEncoded()
            .IgnoreArgument_cbEncoded()
            .IgnoreArgument_pcbStructInfo();
        STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG)) /*this is allocating space for the decoded private key*/
            .IgnoreArgument_size();
        STRICT_EXPECTED_CALL(CryptDecodeObjectEx(X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, PKCS_RSA_PRIVATE_KEY, IGNORED_PTR_ARG, IGNORED_NUM_ARG, 0, NULL, IGNORED_PTR_ARG, IGNORED_PTR_ARG)) /*this is asking "how big is the decoded private key? (from binary)*/
            .IgnoreArgument_pbEncoded()
            .IgnoreArgument_cbEncoded()
            .IgnoreArgument_pvStructInfo()
            .IgnoreArgument_pcbStructInfo();

        STRICT_EXPECTED_CALL(CryptAcquireContextA(IGNORED_PTR_ARG, NULL, MS_ENHANCED_PROV, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)) /*this is acquire a handle to a key container within a cryptographic service provider*/
            .IgnoreArgument_phProv();

        STRICT_EXPECTED_CALL(CryptImportKey((HCRYPTPROV)IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG, (HCRYPTKEY)NULL, 0, IGNORED_PTR_ARG)) /*tranferring the key from the blob to the cryptrographic key provider*/
            .IgnoreArgument_hProv()
            .IgnoreArgument_pbData()
            .IgnoreArgument_dwDataLen()
            .IgnoreArgument_phKey();

        STRICT_EXPECTED_CALL(CertCreateCertificateContext(X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, IGNORED_PTR_ARG, IGNORED_NUM_ARG)) /*create a certificate context from an encoded certificate*/
            .IgnoreArgument_cbCertEncoded()
            .IgnoreArgument_pbCertEncoded();

        STRICT_EXPECTED_CALL(CertSetCertificateContextProperty(IGNORED_PTR_ARG, CERT_KEY_PROV_HANDLE_PROP_ID, 0, IGNORED_PTR_ARG)) /*give the private key*/
            .IgnoreArgument_pCertContext()
            .IgnoreArgument_pvData();

        STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG))
            .IgnoreArgument_ptr();

        STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG))
            .IgnoreArgument_ptr();

        STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG))
            .IgnoreArgument_ptr();

        umock_c_negative_tests_snapshot();

        size_t calls_that_cannot_fail[] = {
            14, /*gballoc_free*/
            15, /*gballoc_free*/
            16, /*gballoc_free*/
            
        };
        
        for (size_t i = 0; i < umock_c_negative_tests_call_count(); i++)
        {
            ///arrange
            size_t j;

            for (j = 0;j<sizeof(calls_that_cannot_fail) / sizeof(calls_that_cannot_fail[0]);j++)
            {
                if (calls_that_cannot_fail[j] == i)
                    break;
            }

            if (j == sizeof(calls_that_cannot_fail) / sizeof(calls_that_cannot_fail[0]))
            {

                umock_c_negative_tests_reset();
                umock_c_negative_tests_fail_call(i);
                ///act
                char temp_str[128];
                (void)sprintf(temp_str, "On failed call %zu", i);
                X509_SCHANNEL_HANDLE h = x509_schannel_create("certificate", "private key");

                ///assert
                ASSERT_IS_NULL_WITH_MSG(h, temp_str);
            }
        }

        ///cleanup
        umock_c_negative_tests_deinit();
    }

    /*Tests_SRS_X509_SCHANNEL_02_011: [ If parameter x509_schannel_handle is NULL then x509_schannel_destroy shall do nothing. ]*/
    TEST_FUNCTION(x509_schannel_destroy_with_NULL_handle_does_nothing)
    {
        ///arrange
        ///act
        
        x509_schannel_destroy(NULL);

        ///assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup - none required
    }

    /*Tests_SRS_X509_SCHANNEL_02_012: [ Otherwise, x509_schannel_destroy shall free all used resources. ]*/
    TEST_FUNCTION(x509_schannel_destroy_succeeds)
    {
        ///arrange
        X509_SCHANNEL_HANDLE h = x509_schannel_create("certificate", "private key");
        umock_c_reset_all_calls();

        STRICT_EXPECTED_CALL(CryptDestroyKey((HCRYPTKEY)IGNORED_PTR_ARG))
            .IgnoreArgument_hKey();
        STRICT_EXPECTED_CALL(CryptReleaseContext((HCRYPTPROV)IGNORED_PTR_ARG, 0))
            .IgnoreArgument_hProv();
        STRICT_EXPECTED_CALL(CertFreeCertificateContext(IGNORED_PTR_ARG))
            .IgnoreArgument_pCertContext();
        STRICT_EXPECTED_CALL(gballoc_free(h));

        ///act
        x509_schannel_destroy(h);

        ///assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup - none required
    }

    /*Tests_SRS_X509_SCHANNEL_02_013: [ If parameter x509_schannel_handle is NULL then x509_schannel_get_certificate_context shall return NULL. ]*/
    TEST_FUNCTION(x509_schannel_get_certificate_context_with_NULL_handle_returns_NULL)
    {
        ///arrange

        ///act
        PCCERT_CONTEXT p = x509_schannel_get_certificate_context(NULL);

        ///assert
        ASSERT_IS_NULL(p);

        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    }

    /*Tests_SRS_X509_SCHANNEL_02_014: [ Otherwise, x509_schannel_get_certificate_context shall return a non-NULL PCCERT_CONTEXT pointer. ]*/
    TEST_FUNCTION(x509_schannel_get_certificate_context_succeeds)
    {
        ///arrange
        X509_SCHANNEL_HANDLE h = x509_schannel_create("certificate", "private key");
        umock_c_reset_all_calls();

        ///act
        PCCERT_CONTEXT p = x509_schannel_get_certificate_context(h);
       
        ///assert
        ASSERT_IS_NOT_NULL(p);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup 
        x509_schannel_destroy(h);
    }

END_TEST_SUITE(x509_schannel_unittests)

#endif /*WIN32*/
