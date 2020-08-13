// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifdef __cplusplus
#include <cstdlib>
#else
#include <stdlib.h>
#endif

static void* my_gballoc_malloc(size_t size)
{
    return malloc(size);
}

static void my_gballoc_free(void* s)
{
    free(s);
}

extern void x509_openssl_test_reset(void);

#ifdef __cplusplus
#include <cstddef>
#else
#include <stddef.h>
#endif

#include "testrunnerswitcher.h"
#include "umock_c/umock_c.h"

#include "openssl/ssl.h"
#include "openssl/x509.h"
#include "openssl/err.h"
#include "openssl/opensslv.h"
#include "openssl/pem.h"
#include "openssl/bio.h"
#include "openssl/rsa.h"
#include "openssl/evp.h"

#include "azure_c_shared_utility/x509_openssl.h"
#include "umock_c/umocktypes_charptr.h"
#include "umock_c/umock_c_negative_tests.h"

#define ENABLE_MOCKS
#include "azure_c_shared_utility/gballoc.h"

#include "umock_c/umock_c_prod.h"

#ifndef VALIDATED_PTR_ARG
#define VALIDATED_PTR_ARG NULL
#endif

#ifndef VALIDATED_NUM_ARG
#define VALIDATED_NUM_ARG 0
#endif

/*from openssl/bio.h*/
/*the below function has different signatures on different versions of OPENSSL*/
/*
|openssl version (number) | openssl string                   | BIO_new_mem_buf prototype                         | Observations                    |
|-------------------------|----------------------------------|---------------------------------------------------|---------------------------------|
| 0x1000106fL             | OpenSSL 1.0.1f 6 Jan 2014        | BIO *BIO_new_mem_buf(void *buf, int len);         | Ubuntu 14.04                    |
| 0x1000204fL             | OpenSSL 1.0.2d 9 Jul 2015        | BIO *BIO_new_mem_buf(void *buf, int len);         | Ubuntu 15.10                    |
| 0x1000207fL             | OpenSSL 1.0.2g-fips  1 Mar 2016  | BIO *BIO_new_mem_buf(const void *buf, int len);   | Ubuntu 16.10                    |
| 0x1010007fL             | OpenSSL 1.0.2g-fips  1 Mar 2016  | BIO *BIO_new(const BIO_METHOD *type);             | Ubuntu 18.04                    |
*/

MOCKABLE_FUNCTION(,int, BIO_free, BIO *,a);
#if OPENSSL_VERSION_NUMBER >= 0x1010007fL
MOCKABLE_FUNCTION(, BIO *, BIO_new, const BIO_METHOD *, type);
MOCKABLE_FUNCTION(, const BIO_METHOD *, BIO_s_mem);
#else
MOCKABLE_FUNCTION(, BIO *, BIO_new, BIO_METHOD *, type);
MOCKABLE_FUNCTION(, BIO_METHOD *, BIO_s_mem);
#endif

MOCKABLE_FUNCTION(, BIO *, BIO_mem, BIO_METHOD *, type);
MOCKABLE_FUNCTION(, int, BIO_puts, BIO *, bp, const char *, buf);

#if OPENSSL_VERSION_NUMBER >= 0x1000207fL
MOCKABLE_FUNCTION(,BIO *,BIO_new_mem_buf, const void *,buf, int, len);
#else
MOCKABLE_FUNCTION(, BIO *, BIO_new_mem_buf, void *, buf, int, len);
#endif

/*from openssl/bn.h*/
MOCKABLE_FUNCTION(, BIGNUM*, BN_dup, const BIGNUM*, a);

/*from openssl/crypto.h*/
MOCKABLE_FUNCTION(, int, CRYPTO_get_ex_new_index, int, class_index, long, argl, void*, argp, CRYPTO_EX_new*, new_func, CRYPTO_EX_dup*, dup_func, CRYPTO_EX_free*, free_func);

/*from openssl/evp.h*/
MOCKABLE_FUNCTION(, EVP_PKEY*, EVP_PKEY_new);
MOCKABLE_FUNCTION(, int, EVP_PKEY_set1_RSA, EVP_PKEY*, pkey, struct rsa_st*, key);
MOCKABLE_FUNCTION(, struct rsa_st*, EVP_PKEY_get0_RSA, EVP_PKEY*, pkey);

/*from openssl/rsa.h*/
MOCKABLE_FUNCTION(, void, RSA_free, RSA *,rsa);
MOCKABLE_FUNCTION(, void*, RSA_get_ex_data, const RSA *,r, int, idx);
MOCKABLE_FUNCTION(, int, RSA_set_ex_data, RSA *, r, int, idx, void*, arg);
#if OPENSSL_VERSION_NUMBER >= 0x10100003L
MOCKABLE_FUNCTION(, int, RSA_set0_key, RSA *,r, BIGNUM*, n, BIGNUM*, e, BIGNUM*, d);
MOCKABLE_FUNCTION(, void, RSA_get0_key, const RSA *,r, const BIGNUM**, n, const BIGNUM**, e, const BIGNUM**, d);
#endif
#if OPENSSL_VERSION_NUMBER >= 0x10100005L
MOCKABLE_FUNCTION(, RSA_METHOD*, RSA_meth_dup, const RSA_METHOD *, meth);
MOCKABLE_FUNCTION(, int, RSA_meth_set1_name, RSA_METHOD *, meth, const char*, name);
MOCKABLE_FUNCTION(, int, RSA_meth_set_flags, RSA_METHOD *, meth, int, flags);
MOCKABLE_FUNCTION(, RSA*, RSA_new);
MOCKABLE_FUNCTION(, const RSA_METHOD*, RSA_get_default_method);
MOCKABLE_FUNCTION(, int, RSA_set_method, RSA*, rsa, const RSA_METHOD*, meth);

typedef int (*openssl_rsa_priv_dec_t) (int, const unsigned char*, unsigned char*, RSA*, int);
MOCKABLE_FUNCTION(, int, RSA_meth_set_priv_dec, RSA_METHOD *, meth, openssl_rsa_priv_dec_t, priv_dec);

typedef int (*openssl_rsa_sign_t) (int, const unsigned char*, unsigned int, unsigned char*, unsigned int*, const RSA*);
MOCKABLE_FUNCTION(, int, RSA_meth_set_sign, RSA_METHOD *, rsa, openssl_rsa_sign_t, sign);

typedef int (*openssl_rsa_finish_t) (RSA*);
MOCKABLE_FUNCTION(, int, RSA_meth_set_finish, RSA_METHOD *, rsa, openssl_rsa_finish_t, finish);
#endif

/*from openssl/x509.h*/
MOCKABLE_FUNCTION(, void, X509_free, X509 *, a);
MOCKABLE_FUNCTION(, EVP_PKEY*, X509_get_pubkey, X509 *, x);

/*from  openssl/pem.h*/
MOCKABLE_FUNCTION(, X509 *, PEM_read_bio_X509, BIO *, bp, X509 **, x, pem_password_cb *, cb, void *, u);
MOCKABLE_FUNCTION(, RSA *, PEM_read_bio_RSAPrivateKey, BIO *,bp, RSA **,x, pem_password_cb *,cb, void *,u);

MOCKABLE_FUNCTION(, RSA*, EVP_PKEY_get1_RSA, EVP_PKEY*, pkey);

/*from openssl/ssl.h*/
MOCKABLE_FUNCTION(,int, SSL_CTX_use_RSAPrivateKey, SSL_CTX *,ctx, RSA *,rsa);
MOCKABLE_FUNCTION(,int, SSL_CTX_use_certificate, SSL_CTX *,ctx, X509*, x);
MOCKABLE_FUNCTION(, X509_STORE *, SSL_CTX_get_cert_store, const SSL_CTX *, ssl_ctx);

/*from openssl/err.h*/
MOCKABLE_FUNCTION(,unsigned long, ERR_get_error);
MOCKABLE_FUNCTION(, char *,ERR_error_string, unsigned long, e, char *,buf);
MOCKABLE_FUNCTION(, unsigned long, ERR_peek_error);

/*from openssl/x509_vfy.h*/
MOCKABLE_FUNCTION(,int, X509_STORE_add_cert, X509_STORE *, ctx, X509 *, x);

typedef void (*x509_FREE_FUNC)(void*);
MOCKABLE_FUNCTION(, void, sk_pop_free, _STACK*, st, x509_FREE_FUNC, free_func);
MOCKABLE_FUNCTION(, void, EVP_PKEY_free, EVP_PKEY*, pkey);
MOCKABLE_FUNCTION(, X509*, PEM_read_bio_X509_AUX, BIO*, bp, X509**, x, pem_password_cb*, cb, void*, u);
MOCKABLE_FUNCTION(, EVP_PKEY*, PEM_read_bio_PrivateKey, BIO*, bp, EVP_PKEY**, x, pem_password_cb*, cb, void*, u);
MOCKABLE_FUNCTION(, int, SSL_CTX_use_PrivateKey, SSL_CTX*, ctx, EVP_PKEY*, pkey);
MOCKABLE_FUNCTION(, long, SSL_CTX_ctrl, SSL_CTX*, ctx, int, cmd, long, larg, void*, parg);
MOCKABLE_FUNCTION(, unsigned long, ERR_peek_last_error);
MOCKABLE_FUNCTION(, void, ERR_clear_error);

#ifndef __APPLE__
MOCKABLE_FUNCTION(, int, EVP_PKEY_id, const EVP_PKEY*, pkey);
#endif

#undef ENABLE_MOCKS

/*the below function has different signatures on different versions of OPENSSL*/
#if OPENSSL_VERSION_NUMBER >= 0x1000207fL
static BIO* my_BIO_new_mem_buf(const void * buf, int len)
#else
static BIO* my_BIO_new_mem_buf(void * buf, int len)
#endif
{
    (void)len, (void)buf;
    return (BIO*)my_gballoc_malloc(1);
}

static int my_BIO_free(BIO * a)
{
    my_gballoc_free(a);
    return 0;
}

static void my_RSA_get0_key(const RSA * r, const BIGNUM** n, const BIGNUM** e, const BIGNUM** d) {
    (void) r;
    if (n) {
        *n =  (BIGNUM*)my_gballoc_malloc(1);
    }

    if (e) {
        *e =  (BIGNUM*)my_gballoc_malloc(1);
    }

    if (d) {
        *d =  (BIGNUM*)my_gballoc_malloc(1);
    }
}

static const RSA_METHOD* my_RSA_get_default_method(void) {
    return (RSA_METHOD*)my_gballoc_malloc(1);
}


#if OPENSSL_VERSION_NUMBER >= 0x1010007fL
static BIO *my_BIO_new(const BIO_METHOD *type)
#else
static BIO *my_BIO_new(BIO_METHOD *type)
#endif
{
    (void)type;
    return (BIO*)my_gballoc_malloc(1);
}

static void my_RSA_free(RSA * rsa)
{
    my_gballoc_free(rsa);
}

static void my_X509_free(X509 * a)
{
    my_gballoc_free(a);
}

static X509* my_PEM_read_bio_X509_AUX(BIO* bp, X509** x, pem_password_cb* cb, void* u)
{
    (void)u, (void)cb, (void)x, (void)bp;
    return (X509*)my_gballoc_malloc(1);
}

static long my_SSL_CTX_ctrl(SSL_CTX* ctx, int cmd, long larg, void* parg)
{
    (void)ctx;
    (void)cmd;
    (void)larg;
    my_gballoc_free(parg);
    return 1;
}

static RSA* my_EVP_PKEY_get1_RSA(EVP_PKEY* pkey)
{
    (void)pkey;
    return (RSA*)my_gballoc_malloc(1);
}

static struct rsa_st* my_EVP_PKEY_get0_RSA(EVP_PKEY* pkey)
{
    (void)pkey;
    return (struct rsa_st*)my_gballoc_malloc(1);
}

static X509 * my_PEM_read_bio_X509(BIO * bp, X509 ** x, pem_password_cb * cb, void * u)
{
    (void)u, (void)cb, (void)x, (void)bp;
    return (X509*)my_gballoc_malloc(1);
}

static RSA * my_PEM_read_bio_RSAPrivateKey(BIO * bp, RSA ** x, pem_password_cb * cb, void * u)
{
    (void)u, (void)cb, (void)x, (void)bp;
    return (RSA*)my_gballoc_malloc(1);
}

static RSA * my_RSA_new()
{
    return (RSA*)my_gballoc_malloc(1);
}

static EVP_PKEY * my_EVP_PKEY_new()
{
    return (EVP_PKEY*)my_gballoc_malloc(1);
}

static RSA_METHOD* my_RSA_meth_dup(const RSA_METHOD* meth)
{
    (void)meth;
    return (RSA_METHOD*)my_gballoc_malloc(1);
}

static int my_RSA_meth_set1_name(RSA_METHOD* meth, const char* name)
{
    (void)meth;
    (void)name;
    return 1;
}

static int my_RSA_meth_set_flags(RSA_METHOD* meth, int flags)
{
    (void)meth;
    (void)flags;
    return 1;
}

static int my_RSA_meth_set_sign(RSA_METHOD* meth, openssl_rsa_sign_t sign)
{
    (void)meth;
    (void)sign;
    return 1;
}

static int my_RSA_meth_set_priv_dec(RSA_METHOD* meth, openssl_rsa_priv_dec_t priv_dec)
{
    (void)meth;
    (void)priv_dec;
    return 1;
}

static int my_RSA_meth_set_finish(RSA_METHOD* meth, openssl_rsa_finish_t finish)
{
    (void)meth;
    (void)finish;
    return 1;
}

static int my_RSA_set_method(RSA* rsa, const RSA_METHOD* meth)
{
    (void)rsa;
    (void)meth;
    return 1;
}

static int my_RSA_set_ex_data(RSA* rsa, int idx, void* arg)
{
    (void)rsa;
    (void)idx;
    (void)arg;
    return 1;
}

static int my_CRYPTO_get_ex_new_index(int class_index, long argl, void *argp,
                                      CRYPTO_EX_new *new_func, CRYPTO_EX_dup *dup_func,
                                      CRYPTO_EX_free *free_func) {
    (void) class_index;
    (void) argl;
    (void) argp;
    (void) new_func;
    (void) dup_func;
    (void) free_func;

    return 1;
}
static EVP_PKEY* my_X509_get_pubkey(X509* x) {
    (void) x;
    return (EVP_PKEY*)my_gballoc_malloc(1);
}
static TEST_MUTEX_HANDLE g_testByTest;
static TEST_MUTEX_HANDLE g_dllByDll;

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    (void)error_code;
    ASSERT_FAIL("umock_c reported error");
}

typedef struct SSL_TEST_CTX_tag
{
    void* extra_certs;
} SSL_TEST_CTX;

typedef struct replace_evp_pkey_st_tag
{
    int type;
} replace_evp_pkey_st;

#define TEST_SSL_CTX ((SSL_CTX*)(0x42))
#define TEST_CERTIFICATE_1 "one certificate"
#define TEST_X509_STORE (X509_STORE *)"le store"
#define TEST_BIO_METHOD (BIO_METHOD*)"le method"
#define TEST_BIO (BIO*)"le bio"

static const char* TEST_PUBLIC_CERTIFICATE = "PUBLIC CERTIFICATE";
static const char* TEST_PRIVATE_CERTIFICATE = "PRIVATE KEY";
static BIO* TEST_BIO_CERT = (BIO*)0x11;
static X509* TEST_X509 = (X509*)0x13;
static SSL_CTX* TEST_SSL_CTX_STRUCTURE;
static SSL_TEST_CTX g_replace_ctx;
static EVP_PKEY* g_evp_pkey;
static replace_evp_pkey_st g_replace_evp_key;

static void func_ptr_free(void* ptr) {
    (void) ptr;
}

static int func_ptr_copy(void* dest, const void* source) {
    *((void**)dest) = *((void**)source);
    return 0;
}

static int func_ptr_equal(const void* left, const void* right) {
    return *((void**)left) == *((void**)right);
}

static char* func_ptr_stringify(const void* value) {
    char *buf = (char*) my_gballoc_malloc(32);
    snprintf(buf, 32, "%p", *((void**)value));
    return buf;
}

BEGIN_TEST_SUITE(x509_openssl_unittests)

    TEST_SUITE_INITIALIZE(a)
    {
        g_evp_pkey = (EVP_PKEY*)&g_replace_evp_key;

        g_testByTest = TEST_MUTEX_CREATE();
        ASSERT_IS_NOT_NULL(g_testByTest);

        (void)umock_c_init(on_umock_c_error);

        (void)umocktypes_charptr_register_types();
        umocktypes_register_type("openssl_rsa_sign_t", func_ptr_stringify, func_ptr_equal, func_ptr_copy, func_ptr_free);
        umocktypes_register_type("openssl_rsa_priv_dec_t", func_ptr_stringify, func_ptr_equal, func_ptr_copy, func_ptr_free);
        umocktypes_register_type("openssl_rsa_finish_t", func_ptr_stringify, func_ptr_equal, func_ptr_copy, func_ptr_free);

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
        REGISTER_GLOBAL_MOCK_RETURNS(BIO_s_mem, TEST_BIO_METHOD, NULL);

        REGISTER_GLOBAL_MOCK_HOOK(BIO_new, my_BIO_new);
        REGISTER_GLOBAL_MOCK_FAIL_RETURN(BIO_new, NULL);
        REGISTER_GLOBAL_MOCK_RETURNS(BIO_puts, strlen(TEST_CERTIFICATE_1), strlen(TEST_CERTIFICATE_1)-1);

        REGISTER_GLOBAL_MOCK_RETURNS(SSL_CTX_get_cert_store, TEST_X509_STORE, NULL);
        REGISTER_GLOBAL_MOCK_RETURNS(X509_STORE_add_cert, __LINE__, 0);

        REGISTER_GLOBAL_MOCK_RETURNS(SSL_CTX_use_RSAPrivateKey, 1, 0);

        REGISTER_GLOBAL_MOCK_HOOK(BIO_free, my_BIO_free);
        REGISTER_GLOBAL_MOCK_HOOK(RSA_free, my_RSA_free);
        REGISTER_GLOBAL_MOCK_HOOK(X509_free, my_X509_free);
        REGISTER_GLOBAL_MOCK_HOOK(EVP_PKEY_get1_RSA, my_EVP_PKEY_get1_RSA);
        REGISTER_GLOBAL_MOCK_FAIL_RETURN(EVP_PKEY_get1_RSA, NULL);
        REGISTER_GLOBAL_MOCK_HOOK(EVP_PKEY_get0_RSA, my_EVP_PKEY_get0_RSA);
        REGISTER_GLOBAL_MOCK_FAIL_RETURN(EVP_PKEY_get0_RSA, NULL);

        REGISTER_GLOBAL_MOCK_RETURNS(PEM_read_bio_PrivateKey, g_evp_pkey, NULL);

        REGISTER_GLOBAL_MOCK_RETURNS(BIO_new_mem_buf, TEST_BIO_CERT, NULL);
        REGISTER_GLOBAL_MOCK_HOOK(PEM_read_bio_X509_AUX, my_PEM_read_bio_X509_AUX);
        REGISTER_GLOBAL_MOCK_FAIL_RETURN(PEM_read_bio_X509_AUX, NULL);
        REGISTER_GLOBAL_MOCK_RETURNS(SSL_CTX_use_PrivateKey, 1, 0);
        REGISTER_GLOBAL_MOCK_HOOK(SSL_CTX_ctrl, my_SSL_CTX_ctrl);
#if OPENSSL_VERSION_NUMBER >= 0x10100005L
        REGISTER_GLOBAL_MOCK_HOOK(RSA_meth_dup, my_RSA_meth_dup);
        REGISTER_GLOBAL_MOCK_FAIL_RETURN(RSA_meth_dup, NULL);
        REGISTER_GLOBAL_MOCK_HOOK(RSA_meth_set1_name, my_RSA_meth_set1_name);
        REGISTER_GLOBAL_MOCK_HOOK(RSA_meth_set_flags, my_RSA_meth_set_flags);
        REGISTER_GLOBAL_MOCK_HOOK(RSA_meth_set_sign, my_RSA_meth_set_sign);
        REGISTER_GLOBAL_MOCK_HOOK(RSA_meth_set_priv_dec, my_RSA_meth_set_priv_dec);
        REGISTER_GLOBAL_MOCK_HOOK(RSA_meth_set_finish, my_RSA_meth_set_finish);
#endif
        REGISTER_GLOBAL_MOCK_HOOK(RSA_get_default_method, my_RSA_get_default_method);
        REGISTER_GLOBAL_MOCK_HOOK(RSA_new, my_RSA_new);
        REGISTER_GLOBAL_MOCK_FAIL_RETURN(RSA_new, NULL);
        REGISTER_GLOBAL_MOCK_HOOK(EVP_PKEY_new, my_EVP_PKEY_new);
        REGISTER_GLOBAL_MOCK_FAIL_RETURN(EVP_PKEY_new, NULL);
        REGISTER_GLOBAL_MOCK_HOOK(RSA_set_method, my_RSA_set_method);
        REGISTER_GLOBAL_MOCK_HOOK(RSA_set_ex_data, my_RSA_set_ex_data);
        REGISTER_GLOBAL_MOCK_HOOK(CRYPTO_get_ex_new_index, my_CRYPTO_get_ex_new_index);
        REGISTER_GLOBAL_MOCK_FAIL_RETURN(CRYPTO_get_ex_new_index, -1);
        REGISTER_GLOBAL_MOCK_HOOK(X509_get_pubkey, my_X509_get_pubkey);
        REGISTER_GLOBAL_MOCK_FAIL_RETURN(X509_get_pubkey, NULL);
        REGISTER_GLOBAL_MOCK_HOOK(RSA_get0_key, my_RSA_get0_key);
    }

    TEST_SUITE_CLEANUP(TestClassCleanup)
    {
        umock_c_deinit();

        TEST_MUTEX_DESTROY(g_testByTest);
    }

    TEST_FUNCTION_INITIALIZE(initialize)
    {
        umock_c_reset_all_calls();

        memset(&g_replace_ctx, 0, sizeof(SSL_TEST_CTX) );
        TEST_SSL_CTX_STRUCTURE = (SSL_CTX*)&g_replace_ctx;

        memset(&g_replace_evp_key, 0, sizeof(replace_evp_pkey_st));
        g_evp_pkey = (EVP_PKEY*)&g_replace_evp_key;
    }

    TEST_FUNCTION_CLEANUP(cleans)
    {
    }

    static int should_skip_index(size_t current_index, const size_t skip_array[], size_t length)
    {
        int result = 0;
        for (size_t index = 0; index < length; index++)
        {
            if (current_index == skip_array[index])
            {
                result = __LINE__;
                break;
            }
        }
        return result;
    }

    static void setup_load_alias_key_cert_mocks(bool is_rsa_cert, bool is_cryptodev)
    {
        if (is_rsa_cert)
        {
            g_replace_evp_key.type = EVP_PKEY_RSA;
            if (is_cryptodev)
            {
                STRICT_EXPECTED_CALL(EVP_PKEY_get1_RSA(IGNORED_PTR_ARG));
            }
            else
            {
                STRICT_EXPECTED_CALL(EVP_PKEY_get1_RSA(g_evp_pkey));
            }
            STRICT_EXPECTED_CALL(SSL_CTX_use_RSAPrivateKey(TEST_SSL_CTX_STRUCTURE, IGNORED_PTR_ARG));
            STRICT_EXPECTED_CALL(RSA_free(IGNORED_PTR_ARG) );
        }
        else
        {
            g_replace_evp_key.type = EVP_PKEY_EC;
            if (is_cryptodev)
            {
                STRICT_EXPECTED_CALL(SSL_CTX_use_PrivateKey(TEST_SSL_CTX_STRUCTURE, IGNORED_PTR_ARG));
            }
            else
            {
                STRICT_EXPECTED_CALL(SSL_CTX_use_PrivateKey(TEST_SSL_CTX_STRUCTURE, g_evp_pkey));
            }
        }
    }

    static void setup_load_certificate_chain_mocks()
    {
        STRICT_EXPECTED_CALL(BIO_new_mem_buf((void*)TEST_PUBLIC_CERTIFICATE, -1));
        STRICT_EXPECTED_CALL(PEM_read_bio_X509_AUX(IGNORED_PTR_ARG, NULL, NULL, NULL));
        STRICT_EXPECTED_CALL(SSL_CTX_use_certificate(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
#if (OPENSSL_VERSION_NUMBER >= 0x10100000L) && (OPENSSL_VERSION_NUMBER < 0x20000000L)
        //STRICT_EXPECTED_CALL(SSL_CTX_clear_extra_chain_certs(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(SSL_CTX_ctrl(TEST_SSL_CTX_STRUCTURE, SSL_CTRL_CLEAR_EXTRA_CHAIN_CERTS, 0, NULL));
#endif
        STRICT_EXPECTED_CALL(PEM_read_bio_X509(IGNORED_PTR_ARG, NULL, NULL, NULL));
        STRICT_EXPECTED_CALL(SSL_CTX_ctrl(TEST_SSL_CTX_STRUCTURE, SSL_CTRL_EXTRA_CHAIN_CERT, IGNORED_NUM_ARG, IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(PEM_read_bio_X509(IGNORED_PTR_ARG, NULL, NULL, NULL))
            .SetReturn(NULL); // Needed because the x509 needs not to be free
        STRICT_EXPECTED_CALL(X509_free(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(BIO_free(IGNORED_PTR_ARG));
    }

    static void setup_add_credentials(bool is_rsa_cert, bool is_cryptodev)
    {
        if (is_cryptodev)
        {
            STRICT_EXPECTED_CALL(RSA_get_default_method());
#if OPENSSL_VERSION_NUMBER >= 0x10100005L
            STRICT_EXPECTED_CALL(RSA_meth_dup(IGNORED_PTR_ARG));
            STRICT_EXPECTED_CALL(RSA_meth_set1_name(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
            STRICT_EXPECTED_CALL(RSA_meth_set_flags(IGNORED_PTR_ARG, 0));
            STRICT_EXPECTED_CALL(RSA_meth_set_sign(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
            STRICT_EXPECTED_CALL(RSA_meth_set_priv_dec(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
            STRICT_EXPECTED_CALL(RSA_meth_set_finish(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
#endif
            STRICT_EXPECTED_CALL(RSA_new());
            STRICT_EXPECTED_CALL(RSA_set_method(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
            STRICT_EXPECTED_CALL(CRYPTO_get_ex_new_index(CRYPTO_EX_INDEX_RSA, 0, NULL, NULL, NULL, NULL));
            STRICT_EXPECTED_CALL(RSA_set_ex_data(IGNORED_PTR_ARG, 1, IGNORED_PTR_ARG));
            STRICT_EXPECTED_CALL(BIO_new_mem_buf((char*)TEST_PUBLIC_CERTIFICATE, -1));
            STRICT_EXPECTED_CALL(PEM_read_bio_X509_AUX(IGNORED_PTR_ARG, NULL, NULL, NULL));
            STRICT_EXPECTED_CALL(BIO_free(IGNORED_PTR_ARG));
            STRICT_EXPECTED_CALL(X509_get_pubkey(IGNORED_PTR_ARG));
            STRICT_EXPECTED_CALL(X509_free(IGNORED_PTR_ARG));
#if OPENSSL_VERSION_NUMBER >= 0x10100003L
            STRICT_EXPECTED_CALL(EVP_PKEY_get0_RSA(IGNORED_PTR_ARG));
            STRICT_EXPECTED_CALL(RSA_get0_key(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, NULL));
#endif
            STRICT_EXPECTED_CALL(BN_dup(IGNORED_PTR_ARG));
            STRICT_EXPECTED_CALL(BN_dup(IGNORED_PTR_ARG));
#if OPENSSL_VERSION_NUMBER >= 0x10100003L
            STRICT_EXPECTED_CALL(RSA_set0_key(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, NULL));
#endif

            STRICT_EXPECTED_CALL(EVP_PKEY_new());
            STRICT_EXPECTED_CALL(EVP_PKEY_set1_RSA(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
        }
        else
        {
            STRICT_EXPECTED_CALL(BIO_new_mem_buf((char*)TEST_PRIVATE_CERTIFICATE, -1));
            STRICT_EXPECTED_CALL(PEM_read_bio_PrivateKey(IGNORED_PTR_ARG, NULL, NULL, NULL));
            STRICT_EXPECTED_CALL(BIO_free(IGNORED_PTR_ARG));
        }
#ifndef __APPLE__
        STRICT_EXPECTED_CALL(EVP_PKEY_id(IGNORED_PTR_ARG)).SetReturn(is_rsa_cert ? EVP_PKEY_RSA : EVP_PKEY_EC);
#endif
        setup_load_alias_key_cert_mocks(is_rsa_cert, is_cryptodev);
        if (is_cryptodev)
        {
            STRICT_EXPECTED_CALL(EVP_PKEY_free(IGNORED_PTR_ARG));
        }
        else
        {
            STRICT_EXPECTED_CALL(EVP_PKEY_free(g_evp_pkey));
        }
        setup_load_certificate_chain_mocks();
    }

    /*Tests_SRS_X509_OPENSSL_02_001: [ If any argument is NULL then x509_openssl_add_credentials shall fail and return a non-zero value. ]*/
    TEST_FUNCTION(x509_openssl_add_credentials_with_NULL_SSL_CTX_fails)
    {
        //arrange

        //act
        int result = x509_openssl_add_credentials(NULL, "certificate", "privatekey");

        //assert
        ASSERT_ARE_NOT_EQUAL(int, 0, result);

        //cleanup
    }

    /*Tests_SRS_X509_OPENSSL_02_001: [ If any argument is NULL then x509_openssl_add_credentials shall fail and return a non-zero value. ]*/
    TEST_FUNCTION(x509_openssl_add_credentials_with_NULL_certificate_fails)
    {
        //arrange

        //act
        int result = x509_openssl_add_credentials(TEST_SSL_CTX, NULL, "privatekey");

        //assert
        ASSERT_ARE_NOT_EQUAL(int, 0, result);

        //cleanup
    }

    /*Tests_SRS_X509_OPENSSL_02_001: [ If any argument is NULL then x509_openssl_add_credentials shall fail and return a non-zero value. ]*/
    TEST_FUNCTION(x509_openssl_add_credentials_with_NULL_privatekey_fails)
    {
        //arrange

        //act
        int result = x509_openssl_add_credentials(TEST_SSL_CTX, "certificate", NULL);

        //assert
        ASSERT_ARE_NOT_EQUAL(int, 0, result);

        //cleanup
    }

    /*Tests_SRS_X509_OPENSSL_02_002: [ x509_openssl_add_credentials shall use BIO_new_mem_buf to create a memory BIO from the x509 certificate. ] */
    /*Tests_SRS_X509_OPENSSL_02_003: [ x509_openssl_add_credentials shall use PEM_read_bio_X509 to read the x509 certificate. ] */
    /*Tests_SRS_X509_OPENSSL_02_004: [ x509_openssl_add_credentials shall use BIO_new_mem_buf to create a memory BIO from the x509 privatekey. ] */
    /*Tests_SRS_X509_OPENSSL_02_005: [ x509_openssl_add_credentials shall use PEM_read_bio_RSAPrivateKey to read the x509 private key. ] */
    /*Tests_SRS_X509_OPENSSL_02_006: [ x509_openssl_add_credentials shall use SSL_CTX_use_certificate to load the certicate into the SSL context. ] */
    /*Tests_SRS_X509_OPENSSL_02_007: [ x509_openssl_add_credentials shall use SSL_CTX_use_RSAPrivateKey to load the private key into the SSL context. ]*/
    /*Tests_SRS_X509_OPENSSL_02_008: [ If no error occurs, then x509_openssl_add_credentials shall succeed and return 0. ] */
    TEST_FUNCTION(x509_openssl_add_credentials_happy_path)
    {
        setup_add_credentials(true, false);

        //act
        int result = x509_openssl_add_credentials(TEST_SSL_CTX_STRUCTURE, TEST_PUBLIC_CERTIFICATE, TEST_PRIVATE_CERTIFICATE);

        //assert
        ASSERT_ARE_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        //cleanup
    }

    TEST_FUNCTION(x509_openssl_add_credentials_ecc_happy_path)
    {
        setup_add_credentials(false, false);

        //act
        int result = x509_openssl_add_credentials(TEST_SSL_CTX_STRUCTURE, TEST_PUBLIC_CERTIFICATE, TEST_PRIVATE_CERTIFICATE);

        //assert
        ASSERT_ARE_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        //cleanup
    }

    /*Tests_SRS_X509_OPENSSL_02_009: [ Otherwise x509_openssl_add_credentials shall fail and return a non-zero number. ]*/
    TEST_FUNCTION(x509_openssl_add_credentials_fails)
    {
        //arrange
        umock_c_reset_all_calls();

        int negativeTestsInitResult = umock_c_negative_tests_init();
        ASSERT_ARE_EQUAL(int, 0, negativeTestsInitResult);

        setup_add_credentials(true, false);

        umock_c_negative_tests_snapshot();

#if (OPENSSL_VERSION_NUMBER >= 0x10100000L) && (OPENSSL_VERSION_NUMBER < 0x20000000L)
    #ifdef __APPLE__
            size_t calls_cannot_fail[] = { 2, 5, 6, 10, 11, 12, 13, 14, 15 };
    #else
            size_t calls_cannot_fail[] = { 2, 3, 6, 7, 11, 12, 13, 14, 15, 16 };
    #endif
#else
    #ifdef __APPLE__
            size_t calls_cannot_fail[] = { 2, 5, 6, 11, 12, 13, 14, 15 };
    #else
            size_t calls_cannot_fail[] = { 2, 3, 6, 7, 11, 12, 13, 14, 15 };
    #endif
#endif
        //act
        int result;
        size_t count = umock_c_negative_tests_call_count();
        for (size_t index = 0; index < count; index++)
        {
            if (should_skip_index(index, calls_cannot_fail, sizeof(calls_cannot_fail) / sizeof(calls_cannot_fail[0])) != 0)
            {
                continue;
            }

            umock_c_negative_tests_reset();
            umock_c_negative_tests_fail_call(index);

            char tmp_msg[128];
            sprintf(tmp_msg, "x509_openssl_add_credentials failure in test %lu/%lu", (unsigned long)index, (unsigned long)count);

            g_replace_ctx.extra_certs = NULL;

            result = x509_openssl_add_credentials(TEST_SSL_CTX_STRUCTURE, TEST_PUBLIC_CERTIFICATE, TEST_PRIVATE_CERTIFICATE);

            //assert
            ASSERT_ARE_NOT_EQUAL(int, 0, result, tmp_msg);
        }

        //cleanup
        umock_c_negative_tests_deinit();
    }

    /*Tests_SRS_X509_OPENSSL_02_010: [ If ssl_ctx is NULL then x509_openssl_add_certificates shall fail and return a non-zero value. ]*/
    TEST_FUNCTION(x509_openssl_add_certificates_with_NULL_ssl_ctx_fails)
    {
        //arrange

        //act
        int result = x509_openssl_add_certificates(NULL, "a");

        //assert
        ASSERT_ARE_NOT_EQUAL(int, 0, result);

        //clean
    }

    /*Tests_SRS_X509_OPENSSL_02_011: [ If certificates is NULL then x509_openssl_add_certificates shall fail and return a non-zero value. ]*/
    TEST_FUNCTION(x509_openssl_add_certificates_with_NULL_certificates_fails)
    {
        //arrange

        //act
        int result = x509_openssl_add_certificates(TEST_SSL_CTX, NULL);

        //assert
        ASSERT_ARE_NOT_EQUAL(int, 0, result);

        //clean
    }

    /*Tests_SRS_X509_OPENSSL_02_012: [ x509_openssl_add_certificates shall get the memory BIO method function by calling BIO_s_mem. ]*/
    /*Tests_SRS_X509_OPENSSL_02_013: [ x509_openssl_add_certificates shall create a new memory BIO by calling BIO_new. ]*/
    /*Tests_SRS_X509_OPENSSL_02_014: [ x509_openssl_add_certificates shall load certificates into the memory BIO by a call to BIO_puts. ]*/
    /*Tests_SRS_X509_OPENSSL_02_015: [ x509_openssl_add_certificates shall retrieve each certificate by a call to PEM_read_bio_X509. ]*/
    /*Tests_SRS_X509_OPENSSL_02_016: [ x509_openssl_add_certificates shall add the certificate to the store by a call to X509_STORE_add_cert. ]*/
    /*Tests_SRS_X509_OPENSSL_02_019: [ Otherwise, x509_openssl_add_certificates shall succeed and return 0. ]*/
    TEST_FUNCTION(x509_openssl_add_certificates_1_certificate_happy_path)
    {
        //arrange
        int result;

        STRICT_EXPECTED_CALL(SSL_CTX_get_cert_store(TEST_SSL_CTX));
        STRICT_EXPECTED_CALL(BIO_s_mem());
        STRICT_EXPECTED_CALL(BIO_new(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(BIO_puts(IGNORED_PTR_ARG, TEST_CERTIFICATE_1));
        STRICT_EXPECTED_CALL(PEM_read_bio_X509(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(X509_STORE_add_cert(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(X509_free(IGNORED_PTR_ARG));

        STRICT_EXPECTED_CALL(PEM_read_bio_X509(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
            .SetReturn(NULL);

        //act
        result = x509_openssl_add_certificates(TEST_SSL_CTX, TEST_CERTIFICATE_1);

        //assert
        ASSERT_ARE_EQUAL(int, 0, result);

        //clean
    }

    void x509_openssl_add_certificates_1_certificate_which_exists_inert_path(void)
    {
        STRICT_EXPECTED_CALL(SSL_CTX_get_cert_store(TEST_SSL_CTX));
        STRICT_EXPECTED_CALL(BIO_s_mem());
        STRICT_EXPECTED_CALL(BIO_new(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(BIO_puts(IGNORED_PTR_ARG, TEST_CERTIFICATE_1));
        STRICT_EXPECTED_CALL(PEM_read_bio_X509(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(X509_STORE_add_cert(IGNORED_PTR_ARG, IGNORED_PTR_ARG))
            .SetReturn(0);
        STRICT_EXPECTED_CALL(ERR_peek_error())
            .SetReturn(X509_R_CERT_ALREADY_IN_HASH_TABLE);
        STRICT_EXPECTED_CALL(X509_free(IGNORED_PTR_ARG));

        STRICT_EXPECTED_CALL(PEM_read_bio_X509(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
            .SetReturn(NULL);
    }

    /*Tests_SRS_X509_OPENSSL_02_017: [ If X509_STORE_add_cert returns with error and that error is X509_R_CERT_ALREADY_IN_HASH_TABLE then x509_openssl_add_certificates shall ignore it as the certificate is already in the store. ]*/
    TEST_FUNCTION(x509_openssl_add_certificates_1_certificate_which_exists_happy_path)
    {
        //arrange
        int result;

        x509_openssl_add_certificates_1_certificate_which_exists_inert_path();

        //act
        result = x509_openssl_add_certificates(TEST_SSL_CTX, TEST_CERTIFICATE_1);

        //assert
        ASSERT_ARE_EQUAL(int, 0, result);

        //clean
    }

    /*Tests_SRS_X509_OPENSSL_02_018: [ In case of any failure x509_openssl_add_certificates shall fail and return a non-zero value. ]*/
    TEST_FUNCTION(x509_openssl_add_certificates_1_certificate_which_exists_unhappy_paths)
    {
        //arrange
        umock_c_reset_all_calls();

        int negativeTestsInitResult = umock_c_negative_tests_init();
        ASSERT_ARE_EQUAL(int, 0, negativeTestsInitResult);

        x509_openssl_add_certificates_1_certificate_which_exists_inert_path();
        umock_c_negative_tests_snapshot();

        size_t calls_cannot_fail[] = { 4, 5, 7, 8 };

        int result;
        size_t count = umock_c_negative_tests_call_count();
        for (size_t index = 0; index < count; index++)
        {
            if (should_skip_index(index, calls_cannot_fail, sizeof(calls_cannot_fail) / sizeof(calls_cannot_fail[0])) != 0)
            {
                continue;
            }

            umock_c_negative_tests_reset();
            umock_c_negative_tests_fail_call(index);

            char tmp_msg[128];
            sprintf(tmp_msg, "x509_openssl_add_credentials failure in test %lu/%lu", (unsigned long)index, (unsigned long)count);

            //act
            result = x509_openssl_add_certificates(TEST_SSL_CTX, TEST_CERTIFICATE_1);

            //assert
            ASSERT_ARE_NOT_EQUAL(int, 0, result, tmp_msg);
        }

        //clean
        umock_c_negative_tests_deinit();
    }

    /*Tests_SRS_X509_OPENSSL_02_020: [ If any argument is NULL then x509_openssl_add_credentials_cryptodev shall fail and return a non-zero value. ]*/
    TEST_FUNCTION(x509_openssl_add_credentials_cryptodev_with_NULL_SSL_CTX_fails)
    {
        //arrange

        //act
        TLSIO_CRYPTODEV_PKEY pkey = {NULL, NULL, NULL, TLSIO_CRYPTODEV_PKEY_TYPE_RSA, NULL};
        int result = x509_openssl_add_credentials_cryptodev(NULL, "certificate", &pkey);

        //assert
        ASSERT_ARE_NOT_EQUAL(int, 0, result);

        //cleanup
    }

    /*Tests_SRS_X509_OPENSSL_02_020: [ If any argument is NULL then x509_openssl_add_credentials_cryptodev shall fail and return a non-zero value. ]*/
    TEST_FUNCTION(x509_openssl_add_credentials_cryptodev_with_NULL_certificate_fails)
    {
        //arrange

        //act
        TLSIO_CRYPTODEV_PKEY pkey = {NULL, NULL, NULL, TLSIO_CRYPTODEV_PKEY_TYPE_RSA, NULL};
        int result = x509_openssl_add_credentials_cryptodev(TEST_SSL_CTX, NULL, &pkey);

        //assert
        ASSERT_ARE_NOT_EQUAL(int, 0, result);

        //cleanup
    }

    /*Tests_SRS_X509_OPENSSL_02_020: [ If any argument is NULL then x509_openssl_add_credentials_cryptodev shall fail and return a non-zero value. ]*/
    TEST_FUNCTION(x509_openssl_add_credentials_cryptodev_with_NULL_privatekey_fails)
    {
        //arrange

        //act
        int result = x509_openssl_add_credentials_cryptodev(TEST_SSL_CTX, "certificate", NULL);

        //assert
        ASSERT_ARE_NOT_EQUAL(int, 0, result);

        //cleanup
    }

    /*Tests_SRS_X509_OPENSSL_02_021: [ x509_openssl_add_credentials_cryptodev shall use BIO_new_mem_buf to create a memory BIO from the x509 certificate. ] */
    /*Tests_SRS_X509_OPENSSL_02_022: [ x509_openssl_add_credentials_cryptodev shall use PEM_read_bio_X509 to read the x509 certificate. ] */
    /*Tests_SRS_X509_OPENSSL_02_023: [ x509_openssl_add_credentials_cryptodev shall use RSA_set_method to set method on an RSA private key. ] */
    /*Tests_SRS_X509_OPENSSL_02_024: [ x509_openssl_add_credentials_cryptodev shall use EVP_PKEY_set1_RSA to create an RSA private key.  ] */
    /*Tests_SRS_X509_OPENSSL_02_025: [ x509_openssl_add_credentials_cryptodev shall use SSL_CTX_use_certificate to load the certicate into the SSL context. ] */
    /*Tests_SRS_X509_OPENSSL_02_026: [ x509_openssl_add_credentials_cryptodev shall use SSL_CTX_use_RSAPrivateKey to load the private key into the SSL context. ]*/
    /*Tests_SRS_X509_OPENSSL_02_027: [ If no error occurs, then x509_openssl_add_credentials_cryptodev shall succeed and return 0. ] */
    TEST_FUNCTION(x509_openssl_add_credentials_cryptodev_happy_path)
    {
        setup_add_credentials(true, true);

        //act
        x509_openssl_test_reset();
        TLSIO_CRYPTODEV_PKEY pkey = {NULL, NULL, NULL, TLSIO_CRYPTODEV_PKEY_TYPE_RSA, NULL};
        int result = x509_openssl_add_credentials_cryptodev(TEST_SSL_CTX_STRUCTURE, TEST_PUBLIC_CERTIFICATE, &pkey);

        //assert
        ASSERT_ARE_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        //cleanup
    }

    /*Tests_SRS_X509_OPENSSL_02_028: [ Otherwise x509_openssl_add_credentials_cryptodev shall fail and return a non-zero number. ]*/
    TEST_FUNCTION(x509_openssl_add_credentials_cryptodev_fails)
    {
        //arrange
        umock_c_reset_all_calls();

        int negativeTestsInitResult = umock_c_negative_tests_init();
        ASSERT_ARE_EQUAL(int, 0, negativeTestsInitResult);

        setup_add_credentials(true, true);

        umock_c_negative_tests_snapshot();

#if (OPENSSL_VERSION_NUMBER >= 0x10100000L) && (OPENSSL_VERSION_NUMBER < 0x10100003L)
    #ifdef __APPLE__
            size_t calls_cannot_fail[] = { 0, 2, 4, 7, 9, 10, 11, 13, 16, 17, 21, 22, 23, 24, 25, 26};
    #else
            size_t calls_cannot_fail[] = { 0, 2, 4, 7, 9, 10, 11, 13, 14, 17, 18, 22, 23, 24, 25, 26, 27};
    #endif
#elif(OPENSSL_VERSION_NUMBER >= 0x10100003L) && (OPENSSL_VERSION_NUMBER < 0x10100005L)
    #ifdef __APPLE__
            size_t calls_cannot_fail[] = { 0, 2, 4, 7, 9, 11, 12, 13, 14, 16, 19, 20, 24, 25, 26, 27, 28, 29};
    #else
            size_t calls_cannot_fail[] = { 0, 2, 4, 7, 9, 11, 12, 13, 14, 16, 17, 20, 21, 25, 26, 27, 28, 29, 30};
    #endif
#elif(OPENSSL_VERSION_NUMBER >= 0x10100005L) && (OPENSSL_VERSION_NUMBER < 0x20000000L)
    #ifdef __APPLE__
            size_t calls_cannot_fail[] = { 0, 2, 3, 4, 5, 6, 8, 10, 13, 15, 17, 18, 19, 20, 22, 25, 26, 30, 31, 32, 33, 34, 35};
    #else
            size_t calls_cannot_fail[] = { 0, 2, 3, 4, 5, 6, 8, 10, 13, 15, 17, 18, 19, 20, 22, 23, 26, 27, 31, 32, 33, 34, 35, 36};
    #endif
#else
    #ifdef __APPLE__
            size_t calls_cannot_fail[] = { 0, 2, 3, 4, 5, 6, 8, 10, 13, 15, 17, 18, 19, 20, 22, 25, 26, 30, 31, 32, 33, 34};
    #else
            size_t calls_cannot_fail[] = { 0, 2, 3, 4, 5, 6, 8, 10, 13, 15, 17, 18, 19, 20, 22, 23, 26, 27, 31, 32, 33, 34, 35};
    #endif
#endif

        //act
        int result;
        size_t count = umock_c_negative_tests_call_count();
        for (size_t index = 0; index < count; index++)
        {
            if (should_skip_index(index, calls_cannot_fail, sizeof(calls_cannot_fail) / sizeof(calls_cannot_fail[0])) != 0)
            {
                continue;
            }

            umock_c_negative_tests_reset();
            umock_c_negative_tests_fail_call(index);

            char tmp_msg[128];
            sprintf(tmp_msg, "x509_openssl_add_credentials_cryptodev failure in test %lu/%lu", (unsigned long)index, (unsigned long)count);

            g_replace_ctx.extra_certs = NULL;

            x509_openssl_test_reset();
            TLSIO_CRYPTODEV_PKEY pkey = {NULL, NULL, NULL, TLSIO_CRYPTODEV_PKEY_TYPE_RSA, NULL};
            result = x509_openssl_add_credentials_cryptodev(TEST_SSL_CTX_STRUCTURE, TEST_PUBLIC_CERTIFICATE, &pkey);

            //assert
            ASSERT_ARE_NOT_EQUAL(int, 0, result, tmp_msg);
        }

        //cleanup
        umock_c_negative_tests_deinit();
    }
END_TEST_SUITE(x509_openssl_unittests)
