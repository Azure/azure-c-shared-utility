// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "openssl/opensslv.h"

#if !defined(OPENSSL_VERSION_NUMBER)
#error Fatal: OPENSSL_VERSION_NUMBER not defined.
#elif (OPENSSL_VERSION_NUMBER >> 12) == 0x10002
#define USE_OPENSSL_1_0_2 1
#define USE_OPENSSL_1_1_0_OR_UP 0
#elif ((OPENSSL_VERSION_NUMBER >> 12) == 0x10100 || \
       (OPENSSL_VERSION_NUMBER >> 12) == 0x10101)
#define USE_OPENSSL_1_0_2 0
#define USE_OPENSSL_1_1_0_OR_UP 1
#else
#error Fatal: unexpected OPENSSL_VERSION_NUMBER; OpenSSL 1.0.2, 1.1.0, or 1.1.1 is required.
// Note: version >= 0x20000000L and < 0x30000000L would be the FIPS-enabled one.
// Also explicitly ignored here.
#endif

#if USE_OPENSSL_1_1_0_OR_UP
#define OPENSSL_API_COMPAT 0x10100000L
#else
#define OPENSSL_API_COMPAT 0x10000000L
#endif

#include "openssl/ssl.h"
#include "openssl/err.h"
#include "openssl/crypto.h"
#undef OCSP_REQUEST
#undef OCSP_RESPONSE
#include "openssl/ocsp.h"
#include "openssl/x509v3.h"
#include "openssl/asn1.h"
#include "openssl/err.h"
#include "openssl/bio.h"
#include "openssl/rsa.h"
#include "openssl/x509.h"
#include "openssl/pem.h"
#include "openssl/err.h"

#if defined(USE_OPENSSL_DYNAMIC)

#define FOR_ALL_OPENSSL_FUNCTIONS \
    REQUIRED_FUNCTION(ASN1_GENERALIZEDTIME_free) \
    REQUIRED_FUNCTION_1_0_2(ASN1_STRING_data) \
    REQUIRED_FUNCTION_1_1_0(ASN1_STRING_get0_data) \
    REQUIRED_FUNCTION(ASN1_STRING_length) \
    REQUIRED_FUNCTION(ASN1_TIME_to_generalizedtime) \
    REQUIRED_FUNCTION(ASN1_TIME_diff) \
    REQUIRED_FUNCTION(BIO_ctrl) \
    REQUIRED_FUNCTION(BIO_ctrl_pending) \
    REQUIRED_FUNCTION(BIO_f_base64) \
    REQUIRED_FUNCTION(BIO_free) \
    REQUIRED_FUNCTION(BIO_free_all) \
    REQUIRED_FUNCTION(BIO_new) \
    REQUIRED_FUNCTION(BIO_new_connect) \
    REQUIRED_FUNCTION(BIO_new_mem_buf) \
    REQUIRED_FUNCTION(BIO_pop) \
    REQUIRED_FUNCTION(BIO_push) \
    REQUIRED_FUNCTION(BIO_puts) \
    REQUIRED_FUNCTION(BIO_read) \
    REQUIRED_FUNCTION(BIO_s_file) \
    REQUIRED_FUNCTION(BIO_s_mem) \
    REQUIRED_FUNCTION(BIO_set_flags) \
    REQUIRED_FUNCTION(BIO_write) \
    REQUIRED_FUNCTION_1_0_2(CRYPTO_cleanup_all_ex_data) \
    REQUIRED_FUNCTION(CRYPTO_free) \
    REQUIRED_FUNCTION_1_0_2(CRYPTO_num_locks) \
    REQUIRED_FUNCTION_1_0_2(CRYPTO_set_dynlock_create_callback) \
    REQUIRED_FUNCTION_1_0_2(CRYPTO_set_dynlock_destroy_callback) \
    REQUIRED_FUNCTION_1_0_2(CRYPTO_set_dynlock_lock_callback) \
    REQUIRED_FUNCTION_1_0_2(CRYPTO_set_id_callback) \
    REQUIRED_FUNCTION_1_0_2(CRYPTO_set_locking_callback) \
    REQUIRED_FUNCTION(DIST_POINT_free) \
    REQUIRED_FUNCTION(ERR_clear_error) \
    REQUIRED_FUNCTION(ERR_error_string) \
    REQUIRED_FUNCTION_1_0_2(ERR_free_strings) \
    REQUIRED_FUNCTION(ERR_get_error) \
    REQUIRED_FUNCTION_1_0_2(ERR_load_BIO_strings) \
    REQUIRED_FUNCTION(ERR_peek_error) \
    REQUIRED_FUNCTION(ERR_peek_last_error) \
    REQUIRED_FUNCTION_1_0_2(ERR_remove_thread_state) \
    REQUIRED_FUNCTION(EVP_PKEY_free) \
    REQUIRED_FUNCTION(EVP_PKEY_get1_RSA) \
    REQUIRED_FUNCTION(EVP_PKEY_id) \
    REQUIRED_FUNCTION_1_0_2(EVP_cleanup) \
    REQUIRED_FUNCTION_1_0_2(FIPS_mode_set) \
    REQUIRED_FUNCTION(GENERAL_NAME_get0_value) \
    REQUIRED_FUNCTION(OCSP_REQ_CTX_add1_header) \
    REQUIRED_FUNCTION(OCSP_REQ_CTX_free) \
    REQUIRED_FUNCTION(OCSP_REQ_CTX_http) \
    REQUIRED_FUNCTION(OCSP_REQ_CTX_new) \
    REQUIRED_FUNCTION(OCSP_parse_url) \
    REQUIRED_FUNCTION(OCSP_set_max_response_length) \
    REQUIRED_FUNCTION_1_0_2(OPENSSL_add_all_algorithms_noconf) \
    REQUIRED_FUNCTION_1_1_0(OPENSSL_sk_free) \
    REQUIRED_FUNCTION_1_1_0(OPENSSL_sk_new_null) \
    REQUIRED_FUNCTION_1_1_0(OPENSSL_sk_num) \
    REQUIRED_FUNCTION_1_1_0(OPENSSL_sk_pop_free) \
    REQUIRED_FUNCTION_1_1_0(OPENSSL_sk_push) \
    REQUIRED_FUNCTION_1_1_0(OPENSSL_sk_value) \
    REQUIRED_FUNCTION_1_1_0(OpenSSL_version) \
    REQUIRED_FUNCTION_1_1_0(OpenSSL_version_num) \
    REQUIRED_FUNCTION(PEM_read_bio_PrivateKey) \
    REQUIRED_FUNCTION(PEM_read_bio_X509) \
    REQUIRED_FUNCTION(PEM_read_bio_X509_AUX) \
    REQUIRED_FUNCTION(PEM_read_bio_X509_CRL) \
    REQUIRED_FUNCTION(PEM_write_bio_X509_CRL) \
    REQUIRED_FUNCTION(RSA_free) \
    REQUIRED_FUNCTION_1_0_2(SSL_COMP_free_compression_methods) \
    REQUIRED_FUNCTION(SSL_CTX_ctrl) \
    REQUIRED_FUNCTION(SSL_CTX_free) \
    REQUIRED_FUNCTION(SSL_CTX_get_cert_store) \
    REQUIRED_FUNCTION(SSL_CTX_new) \
    REQUIRED_FUNCTION(SSL_CTX_set_cert_verify_callback) \
    REQUIRED_FUNCTION(SSL_CTX_set_default_verify_paths) \
    REQUIRED_FUNCTION(SSL_CTX_set_verify) \
    REQUIRED_FUNCTION(SSL_CTX_use_PrivateKey) \
    REQUIRED_FUNCTION(SSL_CTX_use_RSAPrivateKey) \
    REQUIRED_FUNCTION(SSL_CTX_use_certificate) \
    REQUIRED_FUNCTION(SSL_ctrl) \
    REQUIRED_FUNCTION(SSL_do_handshake) \
    REQUIRED_FUNCTION(SSL_free) \
    REQUIRED_FUNCTION(SSL_get_error) \
    REQUIRED_FUNCTION_1_0_2(SSL_library_init) \
    REQUIRED_FUNCTION_1_0_2(SSL_load_error_strings) \
    REQUIRED_FUNCTION(SSL_new) \
    REQUIRED_FUNCTION(SSL_read) \
    REQUIRED_FUNCTION(SSL_set_bio) \
    REQUIRED_FUNCTION(SSL_set_connect_state) \
    REQUIRED_FUNCTION(SSL_write) \
    REQUIRED_FUNCTION_1_1_0(TLS_method) \
    REQUIRED_FUNCTION_1_0_2(SSLeay) \
    REQUIRED_FUNCTION_1_0_2(SSLeay_version) \
    REQUIRED_FUNCTION_1_0_2(TLSv1_1_method) \
    REQUIRED_FUNCTION_1_0_2(TLSv1_2_method) \
    REQUIRED_FUNCTION_1_0_2(TLSv1_method) \
    REQUIRED_FUNCTION(X509_CRL_free) \
    REQUIRED_FUNCTION_1_1_0(X509_CRL_get0_nextUpdate) \
    REQUIRED_FUNCTION_1_1_0(X509_CRL_get_issuer) \
    REQUIRED_FUNCTION(X509_CRL_http_nbio) \
    REQUIRED_FUNCTION_1_1_0(X509_CRL_up_ref) \
    REQUIRED_FUNCTION(X509_NAME_cmp) \
    REQUIRED_FUNCTION(X509_NAME_hash) \
    REQUIRED_FUNCTION(X509_STORE_CTX_get_current_cert) \
    REQUIRED_FUNCTION(X509_STORE_add_cert) \
    REQUIRED_FUNCTION_1_1_0(X509_STORE_get0_param) \
    REQUIRED_FUNCTION(X509_STORE_set_flags) \
    REQUIRED_FUNCTION_1_1_0(X509_STORE_set_lookup_crls) \
    REQUIRED_FUNCTION_1_0_2(X509_STORE_set_lookup_crls_cb) \
    REQUIRED_FUNCTION(X509_VERIFY_PARAM_get_flags) \
    REQUIRED_FUNCTION(X509_free) \
    REQUIRED_FUNCTION(X509_get_ext_d2i) \
    REQUIRED_FUNCTION(X509_get_issuer_name) \
    REQUIRED_FUNCTION(X509_get_subject_name) \
    REQUIRED_FUNCTION_1_0_2(sk_free) \
    REQUIRED_FUNCTION_1_0_2(sk_new_null) \
    REQUIRED_FUNCTION_1_0_2(sk_num) \
    REQUIRED_FUNCTION_1_0_2(sk_pop_free) \
    REQUIRED_FUNCTION_1_0_2(sk_push) \
    REQUIRED_FUNCTION_1_0_2(sk_value) \
    REQUIRED_FUNCTION(d2i_X509_CRL_bio) \
    REQUIRED_FUNCTION(i2d_X509_CRL_bio)

#if USE_OPENSSL_1_1_0_OR_UP
#define REQUIRED_FUNCTION_1_1_0 REQUIRED_FUNCTION
#define REQUIRED_FUNCTION_1_0_2(fn)
#else
#define REQUIRED_FUNCTION_1_0_2 REQUIRED_FUNCTION
#define REQUIRED_FUNCTION_1_1_0(fn)
#endif

// Declare all function pointers.
#define REQUIRED_FUNCTION(fn) extern __typeof(fn)* fn##_ptr;
FOR_ALL_OPENSSL_FUNCTIONS
#undef REQUIRED_FUNCTION

// Now add defines for the dynamic functions.
#define ASN1_GENERALIZEDTIME_free ASN1_GENERALIZEDTIME_free_ptr
#define ASN1_STRING_length ASN1_STRING_length_ptr
#define ASN1_TIME_to_generalizedtime ASN1_TIME_to_generalizedtime_ptr
#define ASN1_TIME_diff ASN1_TIME_diff_ptr
#define BIO_ctrl BIO_ctrl_ptr
#define BIO_ctrl_pending BIO_ctrl_pending_ptr
#define BIO_f_base64 BIO_f_base64_ptr
#define BIO_free BIO_free_ptr
#define BIO_free_all BIO_free_all_ptr
#define BIO_new BIO_new_ptr
#define BIO_new_connect BIO_new_connect_ptr
#define BIO_new_mem_buf BIO_new_mem_buf_ptr
#define BIO_pop BIO_pop_ptr
#define BIO_push BIO_push_ptr
#define BIO_puts BIO_puts_ptr
#define BIO_read BIO_read_ptr
#define BIO_s_file BIO_s_file_ptr
#define BIO_s_mem BIO_s_mem_ptr
#define BIO_set_flags BIO_set_flags_ptr
#define BIO_write BIO_write_ptr
#define CRYPTO_free CRYPTO_free_ptr
#define DIST_POINT_free DIST_POINT_free_ptr
#define ERR_clear_error ERR_clear_error_ptr
#define ERR_error_string ERR_error_string_ptr
#define ERR_get_error ERR_get_error_ptr
#define ERR_peek_error ERR_peek_error_ptr
#define ERR_peek_last_error ERR_peek_last_error_ptr
#define EVP_PKEY_free EVP_PKEY_free_ptr
#define EVP_PKEY_get1_RSA EVP_PKEY_get1_RSA_ptr
#define EVP_PKEY_id EVP_PKEY_id_ptr
#define GENERAL_NAME_get0_value GENERAL_NAME_get0_value_ptr
#define OCSP_REQ_CTX_add1_header OCSP_REQ_CTX_add1_header_ptr
#define OCSP_REQ_CTX_free OCSP_REQ_CTX_free_ptr
#define OCSP_REQ_CTX_http OCSP_REQ_CTX_http_ptr
#define OCSP_REQ_CTX_new OCSP_REQ_CTX_new_ptr
#define OCSP_parse_url OCSP_parse_url_ptr
#define OCSP_set_max_response_length OCSP_set_max_response_length_ptr
#define PEM_read_bio_PrivateKey PEM_read_bio_PrivateKey_ptr
#define PEM_read_bio_X509 PEM_read_bio_X509_ptr
#define PEM_read_bio_X509_AUX PEM_read_bio_X509_AUX_ptr
#define PEM_read_bio_X509_CRL PEM_read_bio_X509_CRL_ptr
#define PEM_write_bio_X509_CRL PEM_write_bio_X509_CRL_ptr
#define RSA_free RSA_free_ptr
#define SSL_CTX_ctrl SSL_CTX_ctrl_ptr
#define SSL_CTX_free SSL_CTX_free_ptr
#define SSL_CTX_get_cert_store SSL_CTX_get_cert_store_ptr
#define SSL_CTX_new SSL_CTX_new_ptr
#define SSL_CTX_set_cert_verify_callback SSL_CTX_set_cert_verify_callback_ptr
#define SSL_CTX_set_default_verify_paths SSL_CTX_set_default_verify_paths_ptr
#define SSL_CTX_set_verify SSL_CTX_set_verify_ptr
#define SSL_CTX_use_PrivateKey SSL_CTX_use_PrivateKey_ptr
#define SSL_CTX_use_RSAPrivateKey SSL_CTX_use_RSAPrivateKey_ptr
#define SSL_CTX_use_certificate SSL_CTX_use_certificate_ptr
#define SSL_ctrl SSL_ctrl_ptr
#define SSL_do_handshake SSL_do_handshake_ptr
#define SSL_free SSL_free_ptr
#define SSL_get_error SSL_get_error_ptr
#define SSL_new SSL_new_ptr
#define SSL_read SSL_read_ptr
#define SSL_set_bio SSL_set_bio_ptr
#define SSL_set_connect_state SSL_set_connect_state_ptr
#define SSL_write SSL_write_ptr
#define X509_CRL_free X509_CRL_free_ptr
#define X509_CRL_http_nbio X509_CRL_http_nbio_ptr
#define X509_NAME_cmp X509_NAME_cmp_ptr
#define X509_NAME_hash X509_NAME_hash_ptr
#define X509_STORE_CTX_get_current_cert X509_STORE_CTX_get_current_cert_ptr
#define X509_STORE_add_cert X509_STORE_add_cert_ptr
#define X509_STORE_set_flags X509_STORE_set_flags_ptr
#define X509_VERIFY_PARAM_get_flags X509_VERIFY_PARAM_get_flags_ptr
#define X509_free X509_free_ptr
#define X509_get_ext_d2i X509_get_ext_d2i_ptr
#define X509_get_issuer_name X509_get_issuer_name_ptr
#define X509_get_subject_name X509_get_subject_name_ptr

#if USE_OPENSSL_1_0_2
#define ASN1_STRING_data ASN1_STRING_data_ptr
#define CRYPTO_cleanup_all_ex_data CRYPTO_cleanup_all_ex_data_ptr
#define CRYPTO_num_locks CRYPTO_num_locks_ptr
#define CRYPTO_set_dynlock_create_callback CRYPTO_set_dynlock_create_callback_ptr
#define CRYPTO_set_dynlock_destroy_callback CRYPTO_set_dynlock_destroy_callback_ptr
#define CRYPTO_set_dynlock_lock_callback CRYPTO_set_dynlock_lock_callback_ptr
#define CRYPTO_set_id_callback CRYPTO_set_id_callback_ptr
#define CRYPTO_set_locking_callback CRYPTO_set_locking_callback_ptr
#define ERR_free_strings ERR_free_strings_ptr
#define ERR_load_BIO_strings ERR_load_BIO_strings_ptr
#define ERR_remove_thread_state ERR_remove_thread_state_ptr
#define EVP_cleanup EVP_cleanup_ptr
#define FIPS_mode_set FIPS_mode_set_ptr
#define OPENSSL_add_all_algorithms_noconf OPENSSL_add_all_algorithms_noconf_ptr
#define SSL_COMP_free_compression_methods SSL_COMP_free_compression_methods_ptr
#define SSL_library_init SSL_library_init_ptr
#define SSL_load_error_strings SSL_load_error_strings_ptr
#define SSLeay SSLeay_ptr
#define SSLeay_version SSLeay_version_ptr
#define TLSv1_1_method TLSv1_1_method_ptr
#define TLSv1_2_method TLSv1_2_method_ptr
#define TLSv1_method TLSv1_method_ptr
#define X509_STORE_set_lookup_crls_cb X509_STORE_set_lookup_crls_cb_ptr
#define sk_free sk_free_ptr
#define sk_new_null sk_new_null_ptr
#define sk_num sk_num_ptr
#define sk_pop_free sk_pop_free_ptr
#define sk_push sk_push_ptr
#define sk_value sk_value_ptr
// TODO
#define d2i_X509_CRL_bio d2i_X509_CRL_bio_ptr
#define i2d_X509_CRL_bio i2d_X509_CRL_bio_ptr
#endif

#if USE_OPENSSL_1_1_0_OR_UP
#define ASN1_STRING_get0_data ASN1_STRING_get0_data_ptr
#define OPENSSL_sk_free OPENSSL_sk_free_ptr
#define OPENSSL_sk_new_null OPENSSL_sk_new_null_ptr
#define OPENSSL_sk_num OPENSSL_sk_num_ptr
#define OPENSSL_sk_pop_free OPENSSL_sk_pop_free_ptr
#define OPENSSL_sk_push OPENSSL_sk_push_ptr
#define OPENSSL_sk_value OPENSSL_sk_value_ptr
#define OpenSSL_version OpenSSL_version_ptr
#define OpenSSL_version_num OpenSSL_version_num_ptr
#define TLS_method TLS_method_ptr
#define X509_CRL_get0_nextUpdate X509_CRL_get0_nextUpdate_ptr
#define X509_CRL_get_issuer X509_CRL_get_issuer_ptr
#define X509_CRL_up_ref X509_CRL_up_ref_ptr
#define X509_STORE_get0_param X509_STORE_get0_param_ptr
#define X509_STORE_set_lookup_crls X509_STORE_set_lookup_crls_ptr
#define d2i_X509_CRL_bio d2i_X509_CRL_bio_ptr
#define i2d_X509_CRL_bio i2d_X509_CRL_bio_ptr

// Stack functions have been already defined w/o the above define's in place.
// Redefine the ones that we are using.

#define sk_GENERAL_NAME_num(stack) OPENSSL_sk_num((const OPENSSL_STACK*)(1 ? stack : (const STACK_OF(GENERAL_NAME)*)0))
#define sk_DIST_POINT_num(stack) OPENSSL_sk_num((const OPENSSL_STACK*)(1 ? stack : (const STACK_OF(DIST_POINT)*)0))

#define sk_X509_CRL_new_null() (STACK_OF(X509_CRL)*)OPENSSL_sk_new_null()

#define sk_GENERAL_NAME_value(stack, idx) (GENERAL_NAME*)OPENSSL_sk_value((const OPENSSL_STACK*)(1 ? stack : (const STACK_OF(GENERAL_NAME)*)0), idx)
#define sk_DIST_POINT_value(stack, idx) (DIST_POINT*)OPENSSL_sk_value((const OPENSSL_STACK*)(1 ? stack : (const STACK_OF(DIST_POINT)*)0), idx)

#define sk_X509_CRL_free(stack) OPENSSL_sk_free((OPENSSL_STACK*)(1 ? stack : (STACK_OF(X509_CRL)*)0))
#define sk_X509_free(stack) OPENSSL_sk_free((OPENSSL_STACK*)(1 ? stack : (STACK_OF(X509)*)0))

#define sk_X509_CRL_push(stack,value) OPENSSL_sk_push((OPENSSL_STACK*)(1 ? stack : (STACK_OF(X509_CRL)*)0), (const void*)(1 ? value : (X509_CRL*)0))
#define sk_DIST_POINT_push(stack,value) OPENSSL_sk_push((OPENSSL_STACK*)(1 ? stack : (STACK_OF(DIST_POINT)*)0), (const void*)(1 ? value : (DIST_POINT*)0))

#define sk_DIST_POINT_pop_free(stack, freefunc) OPENSSL_sk_pop_free((OPENSSL_STACK*)(1 ? stack : (STACK_OF(DIST_POINT)*)0), (OPENSSL_sk_freefunc)(1 ? freefunc : (sk_DIST_POINT_freefunc)0))

#endif

// Returns 0 on success, != 0 otherwise.
extern int load_libssl();

#endif
