/*
 * Copyright (c) 2021, Cisco Systems, Inc.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/cisco/libacvp/LICENSE
 */



#include <stdlib.h>
#include <openssl/rand.h>
#include "app_fips_lcl.h" /* All regular OpenSSL headers must come before here */
#include "app_lcl.h"
#include "safe_mem_lib.h"

#if OPENSSL_VERSION_NUMBER >= 0x30000000L

int app_drbg_handler(ACVP_TEST_CASE *test_case) {
    int rv = 1;
    ACVP_DRBG_TC *tc;
    ACVP_SUB_DRBG alg;
    EVP_RAND *rand = NULL;
    EVP_RAND_CTX *rctx = NULL, *test = NULL;
    OSSL_PARAM params[10] = { 0 };
    const char *alg_name = NULL, *alg_str = NULL, *param_str = NULL;
    char *tmp = NULL;
    unsigned int strength = 512;

    if (!test_case) {
        return rv;
    }

    tc = test_case->tc.drbg;

    alg = acvp_get_drbg_alg(tc->cipher);

    switch (alg) {
    case ACVP_SUB_DRBG_HASH:
        alg_name = "HASH-DRBG";
        param_str = "digest";
        break;
    case ACVP_SUB_DRBG_HMAC:
        alg_name = "HMAC-DRBG";
        param_str = "digest";
        break;
    case ACVP_SUB_DRBG_CTR:
        alg_name = "CTR-DRBG";
        param_str = "cipher";
        break;
    default:
        printf("Invalid DRBG cipher value\n");
        goto err;
    }

    switch (tc->mode) {
    case ACVP_DRBG_SHA_1:
        alg_str = "SHA-1";
        break;
    case ACVP_DRBG_SHA_224:
        alg_str = "SHA2-224";
        break;
    case ACVP_DRBG_SHA_256:
        alg_str = "SHA2-256";
        break;
    case ACVP_DRBG_SHA_384:
        alg_str = "SHA2-384";
        break;
    case ACVP_DRBG_SHA_512:
        alg_str = "SHA2-512";
        break;
    case ACVP_DRBG_SHA_512_224:
        alg_str = "SHA2-512/224";
        break;
    case ACVP_DRBG_SHA_512_256:
        alg_str = "SHA2-512/256";
        break;
    case ACVP_DRBG_AES_128:
        alg_str = "AES-128-CTR";
        break;
    case ACVP_DRBG_AES_192:
        alg_str = "AES-192-CTR";
        break;
    case ACVP_DRBG_AES_256:
        alg_str = "AES-256-CTR";
        break;
    case ACVP_DRBG_TDES:
    default:
        printf("Invalid mode given for DRBG\n");
        goto err;
    }

    tmp = remove_str_const(alg_str);
    if (!tmp) {
        printf ("Unexpected error copying string in DRBG\n");
        goto err;
    }

    /* NOTE ABOUT DRBG in 3.X:
    * TEST-RAND is an "unapproved" algorithm that exists inside the FIPS module. It cannot be used with
    * the property "fips=yes", which we use in the default library context. It has to be used with
    * fips=no in order to run it. Do NOT run this outside of the context of testing in any situation.
    */
    rand = EVP_RAND_fetch(NULL, "TEST-RAND", "fips=no");

    test = EVP_RAND_CTX_new(rand, NULL);
    if (rand) EVP_RAND_free(rand);
    if (!test) {
        printf("Error creating test CTX in DRBG\n");
        goto err;
    }

    params[0] = OSSL_PARAM_construct_uint("strength", &strength);
    params[1] = OSSL_PARAM_construct_end();
    params[2] = OSSL_PARAM_construct_end(); //for hmac
    if (EVP_RAND_CTX_set_params(test, params) != 1) {
        printf("Error setting test ctx params in DRBG\n");
        goto err;
    }

    rand = EVP_RAND_fetch(NULL, alg_name, NULL);
    rctx = EVP_RAND_CTX_new(rand, test);
    if (!rctx) {
        printf("Error creating DRBG ctx\n");
        goto err;
    }
    strength = EVP_RAND_get_strength(rctx);

    params[0] = OSSL_PARAM_construct_utf8_string(param_str, tmp, 0);
    params[1] = OSSL_PARAM_construct_utf8_string("mac", "HMAC", 0); //ignored if irrelevant
    if (EVP_RAND_CTX_set_params(rctx, params) != 1) {
        printf("Error setting algorithm for DRBG\n");
        goto err;
    }

    params[0] = OSSL_PARAM_construct_octet_string("test_entropy", tc->entropy, tc->entropy_len);
    params[1] = OSSL_PARAM_construct_octet_string("test_nonce", tc->nonce, tc->nonce_len);
    if (EVP_RAND_CTX_set_params(test, params) != 1) {
        printf("Error setting initial entropy/nonce for DRBG\n");
        goto err;
    }
    if (EVP_RAND_instantiate(rctx, strength, tc->pred_resist_enabled, tc->perso_string,
                              tc->perso_string_len, NULL) != 1) {
        printf("Error performing RAND instantiate\n");
        goto err;
    }

    params[0] = OSSL_PARAM_construct_octet_string("test_entropy", tc->entropy_input_pr_1, tc->entropy_len);
    if (EVP_RAND_CTX_set_params(test, params) != 1) {
        printf("Error setting params for DRBG (1)\n");
        goto err;
    }

    if (EVP_RAND_generate(rctx, tc->drb, tc->drb_len, strength,
                           tc->pred_resist_enabled,
                           tc->additional_input_1, tc->additional_input_len) != 1) {
        printf("Error performing rand generate (1)\n");
        goto err;
     }

    params[0] = OSSL_PARAM_construct_octet_string("test_entropy", tc->entropy_input_pr_2, tc->entropy_len);
    if (EVP_RAND_CTX_set_params(test, params) != 1) {
        printf("Error setting params for DRBG (2)\n");
        goto err;
    }

    if (EVP_RAND_generate(rctx, tc->drb, tc->drb_len, strength,
                           tc->pred_resist_enabled,
                           tc->additional_input_2, tc->additional_input_len) != 1) {
        printf("Error performing rand generate (2)\n");
        goto err;
     }

    rv = 0;
err:
    if (test) EVP_RAND_CTX_free(test);
    if (rctx) EVP_RAND_CTX_free(rctx);
    if (rand) EVP_RAND_free(rand);
    if (tmp) free (tmp);
    return rv;
}
#else
int app_drbg_handler(ACVP_TEST_CASE *test_case) {
    if (!test_case) {
        return -1;
    }
    return 1;
}

#endif

