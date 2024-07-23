/** @file */
/*
 * Copyright (c) 2024, Cisco Systems, Inc.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/cisco/libacvp/LICENSE
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "acvp.h"
#include "acvp_lcl.h"
#include "parson.h"
#include "safe_lib.h"

/*
 * After the test case has been processed by the DUT, the results
 * need to be JSON formated to be included in the vector set results
 * file that will be uploaded to the server.  This routine handles
 * the JSON processing for a single test case.
 */
static ACVP_RESULT acvp_ml_dsa_output_tc(ACVP_CTX *ctx, ACVP_CIPHER cipher, ACVP_ML_DSA_TC *stc, JSON_Object *tc_rsp) {
    ACVP_RESULT rv;
    ACVP_SUB_ML_DSA mode;
    char *tmp = NULL;

    mode = acvp_get_ml_dsa_alg(cipher);
    if (!mode) {
        return ACVP_INTERNAL_ERR;
    }

    tmp = calloc(ACVP_ML_DSA_TMP_BYTE_MAX + 1, sizeof(char));
    if (!tmp) {
        ACVP_LOG_ERR("Error allocating memory to output ML-DSA test case");
        rv = ACVP_MALLOC_FAIL;
        goto end;
    }

    switch (mode) {
    case ACVP_SUB_ML_DSA_KEYGEN:
        memzero_s(tmp, ACVP_ML_DSA_TMP_BYTE_MAX);
        rv = acvp_bin_to_hexstr(stc->pub_key, stc->pub_key_len, tmp, ACVP_ML_DSA_TMP_BYTE_MAX);
        if (rv != ACVP_SUCCESS) {
            ACVP_LOG_ERR("hex conversion failure (pk)");
            goto end;
        }
        json_object_set_string(tc_rsp, "pk", tmp);

        memzero_s(tmp, ACVP_ML_DSA_TMP_BYTE_MAX);
        rv = acvp_bin_to_hexstr(stc->secret_key, stc->secret_key_len, tmp, ACVP_ML_DSA_TMP_BYTE_MAX);
        if (rv != ACVP_SUCCESS) {
            ACVP_LOG_ERR("hex conversion failure (sk)");
            goto end;
        }
        json_object_set_string(tc_rsp, "sk", tmp);
        break;
    case ACVP_SUB_ML_DSA_SIGGEN:
        /* This also needs pk in the test group response for GDT, handled elsewhere */
        rv = acvp_bin_to_hexstr(stc->sig, stc->sig_len, tmp, ACVP_ML_DSA_TMP_BYTE_MAX);
        if (rv != ACVP_SUCCESS) {
            ACVP_LOG_ERR("hex conversion failure (signature)");
            goto end;
        }
        json_object_set_string(tc_rsp, "signature", tmp);
        break;
    case ACVP_SUB_ML_DSA_SIGVER:
        json_object_set_boolean(tc_rsp, "testPassed", stc->ver_disposition);
        rv = ACVP_SUCCESS;
        break;
    default:
        rv = ACVP_INTERNAL_ERR;
        break;
    }

end:
    if (tmp) free(tmp);
    return rv;
}

/*
 * This function simply releases the data associated with
 * a test case.
 */

static ACVP_RESULT acvp_ml_dsa_release_tc(ACVP_ML_DSA_TC *stc) {
    if (stc->pub_key) free(stc->pub_key);
    if (stc->secret_key) free(stc->secret_key);
    if (stc->seed) free(stc->seed);
    if (stc->rnd) free(stc->rnd);
    if (stc->msg) free(stc->msg);
    if (stc->sig) free(stc->sig);
    memzero_s(stc, sizeof(ACVP_ML_DSA_TC));

    return ACVP_SUCCESS;
}

static ACVP_RESULT acvp_ml_dsa_init_tc(ACVP_CTX *ctx,
                                    ACVP_ML_DSA_TC *stc,
                                    ACVP_CIPHER cipher,
                                    int tc_id,
                                    int tg_id,
                                    ACVP_ML_DSA_TESTTYPE type,
                                    ACVP_ML_DSA_PARAM_SET param_set,
                                    const char *pub_key,
                                    const char *secret_key,
                                    const char *seed,
                                    const char *rnd,
                                    const char *msg,
                                    const char *sig,
                                    int is_deterministic) {
    ACVP_RESULT rv = ACVP_SUCCESS;

    memzero_s(stc, sizeof(ACVP_ML_DSA_TC));

    stc->tc_id = tc_id;
    stc->tg_id = tg_id;
    stc->cipher = cipher;
    stc->type = type;
    stc->param_set = param_set;
    stc->is_deterministic = is_deterministic;

    stc->pub_key = calloc(ACVP_ML_DSA_TMP_BYTE_MAX, sizeof(unsigned char));
    if (!stc->pub_key) {
        goto err;
    }
    if (cipher == ACVP_ML_DSA_SIGVER) {
        rv = acvp_hexstr_to_bin(pub_key, stc->pub_key, ACVP_ML_DSA_TMP_BYTE_MAX, &(stc->pub_key_len));
        if (rv != ACVP_SUCCESS) {
            ACVP_LOG_ERR("Hex conversion failure (pk)");
            return rv;
        }
    }

    stc->secret_key = calloc(ACVP_ML_DSA_TMP_BYTE_MAX, sizeof(unsigned char));
    if (!stc->secret_key) {
        goto err;
    }
    if (cipher == ACVP_ML_DSA_SIGGEN && secret_key) {
        rv = acvp_hexstr_to_bin(secret_key, stc->secret_key, ACVP_ML_DSA_TMP_BYTE_MAX, &(stc->secret_key_len));
        if (rv != ACVP_SUCCESS) {
            ACVP_LOG_ERR("Hex conversion failure (sk)");
            return rv;
        }
    }

    if (cipher == ACVP_ML_DSA_SIGGEN && rnd) {
        stc->rnd = calloc(ACVP_ML_DSA_TMP_BYTE_MAX, sizeof(unsigned char));
        if (!stc->rnd) {
            goto err;
        }
        rv = acvp_hexstr_to_bin(rnd, stc->rnd, ACVP_ML_DSA_TMP_BYTE_MAX, &(stc->rnd_len));
        if (rv != ACVP_SUCCESS) {
            ACVP_LOG_ERR("Hex conversion failure (rnd)");
            return rv;
        }
    }

    if (cipher == ACVP_ML_DSA_KEYGEN) {
        stc->seed = calloc(ACVP_ML_DSA_TMP_BYTE_MAX, sizeof(unsigned char));
        if (!stc->seed) {
            goto err;
        }
        rv = acvp_hexstr_to_bin(seed, stc->seed, ACVP_ML_DSA_TMP_BYTE_MAX, &(stc->seed_len));
        if (rv != ACVP_SUCCESS) {
            ACVP_LOG_ERR("Hex conversion failure (seed)");
            return rv;
        }
    }

    if (cipher == ACVP_ML_DSA_SIGGEN || cipher == ACVP_ML_DSA_SIGVER) {
        stc->msg = calloc(ACVP_ML_DSA_TMP_BYTE_MAX, sizeof(unsigned char));
        if (!stc->msg) {
            goto err;
        }
        rv = acvp_hexstr_to_bin(msg, stc->msg, ACVP_ML_DSA_TMP_BYTE_MAX, &(stc->msg_len));
        if (rv != ACVP_SUCCESS) {
            ACVP_LOG_ERR("Hex conversion failure (msg)");
            return rv;
        }
    }

    if (cipher == ACVP_ML_DSA_SIGGEN || cipher == ACVP_ML_DSA_SIGVER) {
        stc->sig = calloc(ACVP_ML_DSA_TMP_BYTE_MAX, sizeof(unsigned char));
        if (!stc->sig) {
            goto err;
        }
        if (cipher == ACVP_ML_DSA_SIGVER) {
            rv = acvp_hexstr_to_bin(sig, stc->sig, ACVP_ML_DSA_TMP_BYTE_MAX, &(stc->sig_len));
            if (rv != ACVP_SUCCESS) {
                ACVP_LOG_ERR("Hex conversion failure (sig)");
                return rv;
            }
        }
    }

    return ACVP_SUCCESS;

err:
    ACVP_LOG_ERR("Failed to allocate buffer in ML-DSA test case");
    return ACVP_MALLOC_FAIL;
}


static ACVP_ML_DSA_TESTTYPE read_test_type(const char *str) {
    int diff = 1;

    strcmp_s("AFT", 3, str, &diff);
    if (!diff) return ACVP_ML_DSA_TESTTYPE_AFT;

    strcmp_s("GDT", 3, str, &diff);
    if (!diff) return ACVP_ML_DSA_TESTTYPE_GDT;

    return 0;
}

ACVP_RESULT acvp_ml_dsa_kat_handler(ACVP_CTX *ctx, JSON_Object *obj) {
    unsigned int tc_id = 0, tg_id = 0;
    JSON_Value *groupval;
    JSON_Object *groupobj = NULL;
    JSON_Value *testval;
    JSON_Object *testobj = NULL;
    JSON_Array *groups;
    JSON_Array *tests;

    JSON_Value *reg_arry_val = NULL;
    JSON_Object *reg_obj = NULL;
    JSON_Array *reg_arry = NULL;

    int i, g_cnt;
    int j, t_cnt;

    JSON_Value *r_vs_val = NULL;
    JSON_Object *r_vs = NULL;
    JSON_Array *r_tarr = NULL, *r_garr = NULL;  /* Response testarray, grouparray */
    JSON_Value *r_tval = NULL, *r_gval = NULL;  /* Response testval, groupval */
    JSON_Object *r_tobj = NULL, *r_gobj = NULL; /* Response testobj, groupobj */
    ACVP_CAPS_LIST *cap = NULL;
    ACVP_ML_DSA_TC stc;
    ACVP_TEST_CASE tc;
    ACVP_RESULT rv;

    ACVP_CIPHER alg_id;
    char *json_result = NULL;

    ACVP_ML_DSA_TESTTYPE type = 0;
    ACVP_ML_DSA_PARAM_SET param_set = 0;
    const char *alg_str = NULL, *mode_str = NULL, *type_str = NULL, *param_set_str = NULL,  *pub_str = NULL;
    const char *seed_str = NULL, *msg_str = NULL, *sig_str = NULL, *secret_str = NULL, *rnd_str = NULL;
    int is_deterministic = 0;

    if (!ctx) {
        ACVP_LOG_ERR("No ctx for handler operation");
        return ACVP_NO_CTX;
    }

    alg_str = json_object_get_string(obj, "algorithm");
    if (!alg_str) {
        ACVP_LOG_ERR("ERROR: unable to parse 'algorithm' from JSON");
        return ACVP_MALFORMED_JSON;
    }

    memzero_s(&stc, sizeof(ACVP_ML_DSA_TC));
    tc.tc.ml_dsa = &stc;
    mode_str = json_object_get_string(obj, "mode");
    if (!mode_str) {
        ACVP_LOG_ERR("Server JSON missing 'mode'");
        return ACVP_MALFORMED_JSON;
    }

    alg_id = acvp_lookup_cipher_w_mode_index(alg_str, mode_str);
    if (!alg_id) {
        ACVP_LOG_ERR("Server JSON invalid algorithm or mode");
        return ACVP_TC_INVALID_DATA;
    }

    cap = acvp_locate_cap_entry(ctx, alg_id);
    if (!cap) {
        ACVP_LOG_ERR("ERROR: ACVP server requesting unsupported capability");
        return ACVP_UNSUPPORTED_OP;
    }
    ACVP_LOG_VERBOSE("    ML-DSA mode: %s", mode_str);

    /* Create ACVP array for response */
    rv = acvp_create_array(&reg_obj, &reg_arry_val, &reg_arry);
    if (rv != ACVP_SUCCESS) {
        ACVP_LOG_ERR("ERROR: Failed to create JSON response struct. ");
        return rv;
    }

    /* Start to build the JSON response */
    rv = acvp_setup_json_rsp_group(&ctx, &reg_arry_val, &r_vs_val, &r_vs, alg_str, &r_garr);
    if (rv != ACVP_SUCCESS) {
        ACVP_LOG_ERR("Failed to setup json response");
        return rv;
    }
    json_object_set_string(r_vs, "mode", mode_str);

    groups = json_object_get_array(obj, "testGroups");
    if (!groups) {
        ACVP_LOG_ERR("Missing testGroups from server JSON");
        rv = ACVP_MALFORMED_JSON;
        goto err;
    }
    g_cnt = json_array_get_count(groups);

    for (i = 0; i < g_cnt; i++) {

        groupval = json_array_get_value(groups, i);
        groupobj = json_value_get_object(groupval);

        /*
         * Create a new group in the response with the tgid
         * and an array of tests
         */
        r_gval = json_value_init_object();
        r_gobj = json_value_get_object(r_gval);
        tg_id = json_object_get_number(groupobj, "tgId");
        if (!tg_id) {
            ACVP_LOG_ERR("Missing tgid from server JSON groub obj");
            rv = ACVP_MISSING_ARG;
            goto err;
        }
        json_object_set_number(r_gobj, "tgId", tg_id);
        json_object_set_value(r_gobj, "tests", json_value_init_array());
        r_tarr = json_object_get_array(r_gobj, "tests");

        type_str = json_object_get_string(groupobj, "testType");
        if (!type_str) {
            ACVP_LOG_ERR("Server JSON missing 'testType'");
            rv = ACVP_MISSING_ARG;
            goto err;
        }
        type = read_test_type(type_str);
        if (!type) {
            ACVP_LOG_ERR("invalid testType from server JSON");
            rv = ACVP_INVALID_ARG;
            goto err;
        }

        param_set_str = json_object_get_string(groupobj, "parameterSet");
        if (!param_set_str) {
            ACVP_LOG_ERR("Server JSON missing 'parameterSet'");
            rv = ACVP_MISSING_ARG;
            goto err;
        }
        param_set = acvp_lookup_ml_dsa_param_set(param_set_str);
        if (!param_set) {
            ACVP_LOG_ERR("Server JSON invalid 'parameterSet'");
            rv = ACVP_INVALID_ARG;
            goto err;
        }

        if (alg_id == ACVP_ML_DSA_SIGGEN) {
            if (!json_object_has_value(groupobj, "deterministic")) {
                ACVP_LOG_ERR("Server JSON missing 'deterministic'");
                rv = ACVP_MISSING_ARG;
                goto err;
            }
            is_deterministic = json_object_get_boolean(groupobj, "deterministic");
        }

        if (alg_id == ACVP_ML_DSA_SIGVER) {
            pub_str = json_object_get_string(groupobj, "pk");
            if (!pub_str) {
                ACVP_LOG_ERR("Server JSON missing 'pk'");
                rv = ACVP_MISSING_ARG;
                goto err;
            }
        }

        ACVP_LOG_VERBOSE("           Test group: %d", i);
        ACVP_LOG_VERBOSE("            Test type: %s", type_str);
        if (param_set_str) {
            ACVP_LOG_VERBOSE("            param set: %s", param_set_str);
        }
        if (pub_str) {
            ACVP_LOG_VERBOSE("                   pk: %s", pub_str);
        }

        tests = json_object_get_array(groupobj, "tests");
        t_cnt = json_array_get_count(tests);
        if (!t_cnt) {
            ACVP_LOG_ERR("Test array count is zero");
            rv = ACVP_MISSING_ARG;
            goto err;
        }

        for (j = 0; j < t_cnt; j++) {
            ACVP_LOG_VERBOSE("Found new ML-DSA test vector...");
            testval = json_array_get_value(tests, j);
            testobj = json_value_get_object(testval);
            tc_id = json_object_get_number(testobj, "tcId");

            if (alg_id == ACVP_ML_DSA_KEYGEN) {
                seed_str = json_object_get_string(testobj, "seed");
                if (!seed_str) {
                    ACVP_LOG_ERR("Server JSON missing 'seed'");
                    rv = ACVP_MISSING_ARG;
                    goto err;
                }
            } else {
                msg_str = json_object_get_string(testobj, "message");
                if (!msg_str) {
                    ACVP_LOG_ERR("Server JSON missing 'message'");
                    rv = ACVP_MISSING_ARG;
                    goto err;
                }
            }

            if (alg_id == ACVP_ML_DSA_SIGVER) {
                sig_str = json_object_get_string(testobj, "signature");
                if (!sig_str) {
                    ACVP_LOG_ERR("Server JSON missing 'signature'");
                    rv = ACVP_MISSING_ARG;
                    goto err;
                }
            }

            if (alg_id== ACVP_ML_DSA_SIGGEN && type == ACVP_ML_DSA_TESTTYPE_AFT) {
                secret_str = json_object_get_string(testobj, "sk");
                if (!secret_str) {
                    ACVP_LOG_ERR("Server JSON missing 'sk'");
                    rv = ACVP_MISSING_ARG;
                    goto err;
                }

                if (!is_deterministic) {
                    rnd_str = json_object_get_string(testobj, "rnd");
                    if (!rnd_str) {
                        ACVP_LOG_ERR("Server JSON missing 'rnd'");
                        rv = ACVP_MISSING_ARG;
                        goto err;
                    }
                }
            }

            ACVP_LOG_VERBOSE("        Test case: %d", j);
            ACVP_LOG_VERBOSE("             tcId: %d", tc_id);
            if (seed_str) {
                ACVP_LOG_VERBOSE("             seed: %s", seed_str);
            }
            if (msg_str) {
                ACVP_LOG_VERBOSE("          message: %s", msg_str);
            }
            if (sig_str) {
                ACVP_LOG_VERBOSE("        signature: %s", seed_str);
            }
            if (secret_str) {
                ACVP_LOG_VERBOSE("               sk: %s", secret_str);
            }
            if (rnd_str) {
                ACVP_LOG_VERBOSE("              rnd: %s", rnd_str);
            }

            /* Create a new test case in the response */
            r_tval = json_value_init_object();
            r_tobj = json_value_get_object(r_tval);

            json_object_set_number(r_tobj, "tcId", tc_id);

            rv = acvp_ml_dsa_init_tc(ctx, &stc, alg_id, tc_id, tg_id, type, param_set, pub_str,
                                  secret_str, seed_str, rnd_str, msg_str, sig_str, is_deterministic);

            /* Process the current test vector... */
            if (rv == ACVP_SUCCESS) {
                if ((cap->crypto_handler)(&tc)) {
                    ACVP_LOG_ERR("ERROR: crypto module failed the operation");
                    rv = ACVP_CRYPTO_MODULE_FAIL;
                    json_value_free(r_tval);
                    goto err;
                }
            } else {
                ACVP_LOG_ERR("Failed to initialize ML-DSA test case");
                json_value_free(r_tval);
                goto err;
            }

            /* Output the test case results using JSON */

            /* For siggen, we need a public key for the test group object, grab from first TC for group */
            if (alg_id == ACVP_ML_DSA_SIGGEN && type == ACVP_ML_DSA_TESTTYPE_GDT && !j) {
                char *tmp = calloc(ACVP_ML_DSA_TMP_BYTE_MAX + 1, sizeof(char));
                rv = acvp_bin_to_hexstr(stc.pub_key, stc.pub_key_len, tmp, ACVP_ML_DSA_TMP_BYTE_MAX);
                if (rv != ACVP_SUCCESS) {
                    ACVP_LOG_ERR("hex conversion failure (pub_key)");
                    free(tmp);
                    json_value_free(r_tval);
                    goto err;
                }
                json_object_set_string(r_gobj, "pk", (const char *)tmp);
                memzero_s(tmp, ACVP_ML_DSA_TMP_BYTE_MAX);
                free(tmp);
            }
            rv = acvp_ml_dsa_output_tc(ctx, alg_id, &stc, r_tobj);
            if (rv != ACVP_SUCCESS) {
                ACVP_LOG_ERR("ERROR: JSON output failure in hash module");
                json_value_free(r_tval);
                goto err;
            }

            /* Append the test response value to array */
            json_array_append_value(r_tarr, r_tval);

            /* Release all the memory associated with the test case */
            acvp_ml_dsa_release_tc(&stc);
        }
        json_array_append_value(r_garr, r_gval);
    }

    json_array_append_value(reg_arry, r_vs_val);

    json_result = json_serialize_to_string_pretty(ctx->kat_resp, NULL);
    ACVP_LOG_VERBOSE("\n\n%s\n\n", json_result);
    json_free_serialized_string(json_result);
    rv = ACVP_SUCCESS;

err:
    if (rv != ACVP_SUCCESS) {
        acvp_ml_dsa_release_tc(&stc);
        acvp_release_json(r_vs_val, r_gval);
    }
    return rv;
}