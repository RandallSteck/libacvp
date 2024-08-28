/*
 * Copyright (c) 2024, Cisco Systems, Inc.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/cisco/libacvp/LICENSE
 */

#ifndef ACVP_APP_IUT_H
#define ACVP_APP_IUT_H

#include "acvp/acvp.h"

#include <openssl/opensslv.h>
#include <openssl/evp.h>

/* Versions for OpenSSL FPs that match the format generated by APIs in iut_utils */
#define OPENSSL_FIPS_300 3000000
#define OPENSSL_FIPS_308 3000008
#define OPENSSL_FIPS_309 3000009
#define OPENSSL_FIPS_312 3010002
#define OPENSSL_FIPS_DEV 3040000

#define ENGID1 "800002B805123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456"
#define ENGID2 "000002b87766554433221100"

#define FIPS_PROVIDER_LOOKUP_NAME "OpenSSL FIPS Provider"

extern int fips_ver;
extern int max_ldt_size;

ACVP_RESULT fips_sanity_check(void);
const char *get_string_from_oid(unsigned char *oid, int oid_len);
const char *get_ed_instance_param(ACVP_ED_CURVE curve, int is_prehash, int has_context);
const char *get_ed_curve_string(ACVP_ED_CURVE curve);
const char *get_provider_version(const char *provider_name);
int provider_ver_str_to_int(const char *str);

int get_nid_for_curve(ACVP_EC_CURVE curve);
const EVP_MD *get_md_for_hash_alg(ACVP_HASH_ALG alg);
const char *get_md_string_for_hash_alg(ACVP_HASH_ALG alg, int *md_size);
char *ec_point_to_pub_key(unsigned char *x, int x_len, unsigned char *y, int y_len, int *key_len);

int app_aes_handler(ACVP_TEST_CASE *test_case);
int app_aes_handler_aead(ACVP_TEST_CASE *test_case);
int app_aes_keywrap_handler(ACVP_TEST_CASE *test_case);
int app_des_handler(ACVP_TEST_CASE *test_case);
int app_sha_handler(ACVP_TEST_CASE *test_case);
int app_hmac_handler(ACVP_TEST_CASE *test_case);
int app_cmac_handler(ACVP_TEST_CASE *test_case);
int app_kmac_handler(ACVP_TEST_CASE *test_case);
int app_kdf135_ssh_handler(ACVP_TEST_CASE *test_case);
int app_kdf108_handler(ACVP_TEST_CASE *test_case);
int app_kdf135_x942_handler(ACVP_TEST_CASE *test_case);
int app_kdf135_x963_handler(ACVP_TEST_CASE *test_case);
int app_pbkdf_handler(ACVP_TEST_CASE *test_case);
int app_kdf_tls12_handler(ACVP_TEST_CASE *test_case);
int app_kdf_tls13_handler(ACVP_TEST_CASE *test_case);
int app_dsa_handler(ACVP_TEST_CASE *test_case);
int app_kas_ecc_handler(ACVP_TEST_CASE *test_case);
int app_kas_ffc_handler(ACVP_TEST_CASE *test_case);
int app_kas_ifc_handler(ACVP_TEST_CASE *test_case);
int app_kda_hkdf_handler(ACVP_TEST_CASE *test_case);
int app_kda_onestep_handler(ACVP_TEST_CASE *test_case);
int app_kda_twostep_handler(ACVP_TEST_CASE *test_case);
int app_kts_ifc_handler(ACVP_TEST_CASE *test_case);
int app_rsa_keygen_handler(ACVP_TEST_CASE *test_case);
int app_rsa_sig_handler(ACVP_TEST_CASE *test_case);
int app_rsa_sigprim_handler(ACVP_TEST_CASE *test_case);
int app_rsa_decprim_handler(ACVP_TEST_CASE *test_case);
int app_ecdsa_handler(ACVP_TEST_CASE *test_case);
int app_eddsa_handler(ACVP_TEST_CASE *test_case);
int app_drbg_handler(ACVP_TEST_CASE *test_case);
int app_safe_primes_handler(ACVP_TEST_CASE *test_case);
int app_aes_handler_gmac(ACVP_TEST_CASE *test_case);

void app_aes_cleanup(void);
void app_des_cleanup(void);
void app_dsa_cleanup(void);
void app_rsa_cleanup(void);
void app_ecdsa_cleanup(void);
void app_eddsa_cleanup(void);

/* Registration APIs for differenet OpenSSL providers */
ACVP_RESULT register_capabilities_fp_30X(ACVP_CTX *ctx, APP_CONFIG *cfg);
ACVP_RESULT register_capabilities_fp_312(ACVP_CTX *ctx, APP_CONFIG *cfg);
ACVP_RESULT register_capabilities_non_fips(ACVP_CTX *ctx, APP_CONFIG *cfg);
ACVP_RESULT register_capabilities_fp_dev(ACVP_CTX *ctx, APP_CONFIG *cfg);

#endif // ACVP_APP_IUT_H

