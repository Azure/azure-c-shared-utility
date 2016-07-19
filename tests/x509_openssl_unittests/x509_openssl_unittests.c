// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.


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

#include <stddef.h>
#include "testrunnerswitcher.h"
#include "umock_c.h"

#include "openssl/ssl.h"
#include "openssl/x509.h"
#include "openssl/err.h"
#include "openssl/opensslv.h"
#include "azure_c_shared_utility/x509_openssl.h"
#include "umocktypes_charptr.h"
#include "umock_c_negative_tests.h"

#define ENABLE_MOCKS
#include "azure_c_shared_utility/gballoc.h"

#include "azure_c_shared_utility/umock_c_prod.h"

/*from openssl/bio.h*/
MOCKABLE_FUNCTION(,int, BIO_free, BIO *,a);

/*the below function has different signatures on different versions of OPENSSL*/
#if OPENSSL_VERSION_NUMBER >= 0x1000207fL
/*known to work on 1.0.2g-fips 1 Mar 2016*/
MOCKABLE_FUNCTION(,BIO *,BIO_new_mem_buf, const void *,buf, int, len);
#else
/*known to work on 0x1000207fL - OpenSSL 1.0.2d 9 Jul 2015*/
MOCKABLE_FUNCTION(, BIO *, BIO_new_mem_buf, void *, buf, int, len);
#endif

/*from openssl/rsa.h*/
MOCKABLE_FUNCTION(,void, RSA_free, RSA *,rsa);

/*from openssl/x509.h*/
MOCKABLE_FUNCTION(,void, X509_free, X509 *, a);

/*from  openssl/pem.h*/
MOCKABLE_FUNCTION(, X509 *,PEM_read_bio_X509, BIO *, bp, X509 **, x, pem_password_cb *, cb, void *, u);
MOCKABLE_FUNCTION(,RSA *,PEM_read_bio_RSAPrivateKey, BIO *,bp, RSA **,x, pem_password_cb *,cb, void *,u);

/*from openssl/ssl.h*/
MOCKABLE_FUNCTION(,int, SSL_CTX_use_RSAPrivateKey, SSL_CTX *,ctx, RSA *,rsa);
MOCKABLE_FUNCTION(,int, SSL_CTX_use_certificate, SSL_CTX *,ctx, X509*, x);

/*from openssl/err.h*/
MOCKABLE_FUNCTION(,unsigned long, ERR_get_error);
MOCKABLE_FUNCTION(, char *,ERR_error_string, unsigned long, e, char *,buf);

#undef ENABLE_MOCKS

/*the below function has different signatures on different versions of OPENSSL*/
#if OPENSSL_VERSION_NUMBER >= 0x1000207fL
/*known to work on 1.0.2g-fips 1 Mar 2016*/
static BIO* my_BIO_new_mem_buf(const void * buf, int len)
#else
/*known to work on 0x1000207fL - OpenSSL 1.0.2d 9 Jul 2015*/
static BIO* my_BIO_new_mem_buf(void * buf, int len)
#endif
{
    (void)(len,buf);
    return (BIO*)my_gballoc_malloc(sizeof(BIO));
}

static int my_BIO_free(BIO * a)
{
    my_gballoc_free(a);
    return 0;
}

static void my_RSA_free(RSA * rsa)
{
    my_gballoc_free(rsa);
}

static void my_X509_free(X509 * a)
{
    my_gballoc_free(a);
}

static X509 * my_PEM_read_bio_X509(BIO * bp, X509 ** x, pem_password_cb * cb, void * u)
{
    (void)(u,cb, x, bp);
    return (X509*)my_gballoc_malloc(sizeof(X509));
}

static RSA * my_PEM_read_bio_RSAPrivateKey(BIO * bp, RSA ** x, pem_password_cb * cb, void * u)
{
    (void)(u, cb, x, bp);
    return (RSA*)my_gballoc_malloc(sizeof(RSA));
}

static TEST_MUTEX_HANDLE g_testByTest;
static TEST_MUTEX_HANDLE g_dllByDll;

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    (void)error_code;
    ASSERT_FAIL("umock_c reported error");
}


#define TEST_SSL_CTX ((SSL_CTX*)(0x42))

BEGIN_TEST_SUITE(x509_openssl_unittests)

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

        REGISTER_GLOBAL_MOCK_HOOK(BIO_new_mem_buf, my_BIO_new_mem_buf);
        REGISTER_GLOBAL_MOCK_FAIL_RETURN(BIO_new_mem_buf, NULL);

        REGISTER_GLOBAL_MOCK_HOOK(PEM_read_bio_X509, my_PEM_read_bio_X509);
        REGISTER_GLOBAL_MOCK_FAIL_RETURN(PEM_read_bio_X509, NULL);

        REGISTER_GLOBAL_MOCK_HOOK(PEM_read_bio_RSAPrivateKey, my_PEM_read_bio_RSAPrivateKey);
        REGISTER_GLOBAL_MOCK_FAIL_RETURN(PEM_read_bio_X509, NULL);
        
        REGISTER_GLOBAL_MOCK_RETURNS(SSL_CTX_use_certificate, 1, 0);

        REGISTER_GLOBAL_MOCK_RETURNS(SSL_CTX_use_RSAPrivateKey, 1, 0);

        REGISTER_GLOBAL_MOCK_HOOK(BIO_free, my_BIO_free);
        REGISTER_GLOBAL_MOCK_HOOK(RSA_free, my_RSA_free);
        REGISTER_GLOBAL_MOCK_HOOK(X509_free, my_X509_free);
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

    /*Tests_SRS_X509_OPENSSL_02_001: [ If any argument is NULL then x509_openssl_add_credentials shall fail and return a non-zero value. ]*/
    TEST_FUNCTION(x509_openssl_add_credentials_with_NULL_SSL_CTX_fails)
    {
        ///arrange

        ///act
        int result = x509_openssl_add_credentials(NULL, "certificate", "privatekey");

        ///assert
        ASSERT_ARE_NOT_EQUAL(int, 0, result);

        ///cleanup
    }

    /*Tests_SRS_X509_OPENSSL_02_001: [ If any argument is NULL then x509_openssl_add_credentials shall fail and return a non-zero value. ]*/
    TEST_FUNCTION(x509_openssl_add_credentials_with_NULL_certificate_fails)
    {
        ///arrange

        ///act
        int result = x509_openssl_add_credentials(TEST_SSL_CTX, NULL, "privatekey");

        ///assert
        ASSERT_ARE_NOT_EQUAL(int, 0, result);

        ///cleanup
    }

    /*Tests_SRS_X509_OPENSSL_02_001: [ If any argument is NULL then x509_openssl_add_credentials shall fail and return a non-zero value. ]*/
    TEST_FUNCTION(x509_openssl_add_credentials_with_NULL_privatekey_fails)
    {
        ///arrange

        ///act
        int result = x509_openssl_add_credentials(TEST_SSL_CTX, "certificate", NULL);

        ///assert
        ASSERT_ARE_NOT_EQUAL(int, 0, result);

        ///cleanup
    }

    #ifndef VALIDATED_PTR_ARG
    #define VALIDATED_PTR_ARG NULL
    #endif

    #ifndef VALIDATED_NUM_ARG
    #define VALIDATED_NUM_ARG 0
    #endif
    /*Tests_SRS_X509_OPENSSL_02_002: [ x509_openssl_add_credentials shall use BIO_new_mem_buf to create a memory BIO from the x509 certificate. ] */
    /*Tests_SRS_X509_OPENSSL_02_003: [ x509_openssl_add_credentials shall use PEM_read_bio_X509 to read the x509 certificate. ] */
    /*Tests_SRS_X509_OPENSSL_02_004: [ x509_openssl_add_credentials shall use BIO_new_mem_buf to create a memory BIO from the x509 privatekey. ] */
    /*Tests_SRS_X509_OPENSSL_02_005: [ x509_openssl_add_credentials shall use PEM_read_bio_RSAPrivateKey to read the x509 private key. ] */
    /*Tests_SRS_X509_OPENSSL_02_006: [ x509_openssl_add_credentials shall use SSL_CTX_use_certificate to load the certicate into the SSL context. ] */
    /*Tests_SRS_X509_OPENSSL_02_007: [ x509_openssl_add_credentials shall use SSL_CTX_use_RSAPrivateKey to load the private key into the SSL context. ]*/
    /*Tests_SRS_X509_OPENSSL_02_008: [ If no error occurs, then x509_openssl_add_credentials shall succeed and return 0. ] */
    TEST_FUNCTION(x509_openssl_add_credentials_happy_path)
    {
        
        ///arrange
        char* certificateText = (char*)"certificate";
        char* privatekeyText = (char*)"privatekeyText";
        BIO* certificate;
        BIO* privatekey;
        X509* certificateAsX509;
        RSA* privatekeyasRSA;
        STRICT_EXPECTED_CALL(BIO_new_mem_buf(certificateText, -1))
            .CaptureReturn(&certificate);
        STRICT_EXPECTED_CALL(PEM_read_bio_X509(VALIDATED_PTR_ARG, NULL, 0, NULL))
            .ValidateArgumentValue_bp(&certificate)
            .CaptureReturn(&certificateAsX509)
            ;
        STRICT_EXPECTED_CALL(BIO_new_mem_buf(privatekeyText, -1))
            .CaptureReturn(&privatekey);
        STRICT_EXPECTED_CALL(PEM_read_bio_RSAPrivateKey(VALIDATED_PTR_ARG, NULL, 0, NULL))
            .ValidateArgumentValue_bp(&privatekey)
            .CaptureReturn(&privatekeyasRSA);

        STRICT_EXPECTED_CALL(SSL_CTX_use_certificate(TEST_SSL_CTX, VALIDATED_PTR_ARG))
            .ValidateArgumentValue_x(&certificateAsX509);
        STRICT_EXPECTED_CALL(SSL_CTX_use_RSAPrivateKey(TEST_SSL_CTX, VALIDATED_PTR_ARG))
            .ValidateArgumentValue_rsa(&privatekeyasRSA);

        STRICT_EXPECTED_CALL(RSA_free(VALIDATED_PTR_ARG))
            .ValidateArgumentValue_rsa(&privatekeyasRSA);

        STRICT_EXPECTED_CALL(BIO_free(VALIDATED_PTR_ARG))
            .ValidateArgumentValue_a(&privatekey);

        STRICT_EXPECTED_CALL(X509_free(VALIDATED_PTR_ARG))
            .ValidateArgumentValue_a(&certificateAsX509);

        STRICT_EXPECTED_CALL(BIO_free(VALIDATED_PTR_ARG))
            .ValidateArgumentValue_a(&certificate);

        ///act
        int result = x509_openssl_add_credentials(TEST_SSL_CTX, certificateText, privatekeyText);

        ///assert
        ASSERT_ARE_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
    }

    /*Tests_SRS_X509_OPENSSL_02_009: [ Otherwise x509_openssl_add_credentials shall fail and return a non-zero number. ]*/
    TEST_FUNCTION(x509_openssl_add_credentials_unhappy_paths)
    {

        ///arrange
        (void)umock_c_negative_tests_init();

        size_t calls_that_cannot_fail[] =
        {
            6, /*RSA_free*/
            7, /*BIO_free*/
            8, /*X509_free*/
            9 /*BIO_free*/
        };

        char* certificateText = (char*)"certificate";
        char* privatekeyText = (char*)"privatekeyText";
        BIO* certificate;
        BIO* privatekey;
        X509* certificateAsX509;
        RSA* privatekeyasRSA;
        STRICT_EXPECTED_CALL(BIO_new_mem_buf(certificateText, -1))
            .CaptureReturn(&certificate);
        STRICT_EXPECTED_CALL(PEM_read_bio_X509(VALIDATED_PTR_ARG, NULL, 0, NULL))
            .ValidateArgumentValue_bp(&certificate)
            .CaptureReturn(&certificateAsX509)
            ;
        STRICT_EXPECTED_CALL(BIO_new_mem_buf(privatekeyText, -1))
            .CaptureReturn(&privatekey);
        STRICT_EXPECTED_CALL(PEM_read_bio_RSAPrivateKey(VALIDATED_PTR_ARG, NULL, 0, NULL))
            .ValidateArgumentValue_bp(&privatekey)
            .CaptureReturn(&privatekeyasRSA);

        STRICT_EXPECTED_CALL(SSL_CTX_use_certificate(TEST_SSL_CTX, VALIDATED_PTR_ARG))
            .ValidateArgumentValue_x(&certificateAsX509);
        STRICT_EXPECTED_CALL(SSL_CTX_use_RSAPrivateKey(TEST_SSL_CTX, VALIDATED_PTR_ARG))
            .ValidateArgumentValue_rsa(&privatekeyasRSA);

        STRICT_EXPECTED_CALL(RSA_free(VALIDATED_PTR_ARG))
            .ValidateArgumentValue_rsa(&privatekeyasRSA);

        STRICT_EXPECTED_CALL(BIO_free(VALIDATED_PTR_ARG))
            .ValidateArgumentValue_a(&privatekey);

        STRICT_EXPECTED_CALL(X509_free(VALIDATED_PTR_ARG))
            .ValidateArgumentValue_a(&certificateAsX509);

        STRICT_EXPECTED_CALL(BIO_free(VALIDATED_PTR_ARG))
            .ValidateArgumentValue_a(&certificate);

        umock_c_negative_tests_snapshot();

        ///act

        for (size_t i = 0; i < umock_c_negative_tests_call_count(); i++)
        {
            size_t j;
            umock_c_negative_tests_reset();

            for (j = 0;j<sizeof(calls_that_cannot_fail) / sizeof(calls_that_cannot_fail[0]);j++) /*not running the tests that have failed that cannot fail*/
            {
                if (calls_that_cannot_fail[j] == i)
                    break;
            }

            if (j == sizeof(calls_that_cannot_fail) / sizeof(calls_that_cannot_fail[0]))
            {

                umock_c_negative_tests_fail_call(i);
                char temp_str[128];
                sprintf(temp_str, "On failed call %zu", i);

                ///act
                int result = x509_openssl_add_credentials(TEST_SSL_CTX, certificateText, privatekeyText);

                ///assert
                ASSERT_ARE_NOT_EQUAL_WITH_MSG(int, 0, result, temp_str);
            }
        }

        ///cleanup
        umock_c_negative_tests_deinit();
    }

END_TEST_SUITE(x509_openssl_unittests)


