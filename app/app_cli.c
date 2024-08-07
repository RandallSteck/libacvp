/*
 * Copyright (c) 2024, Cisco Systems, Inc.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/cisco/libacvp/LICENSE
 */


#include <stdio.h>
#include "ketopt.h"
#include "app_lcl.h"
#include "acvp/acvp.h"
#include "safe_lib.h"

#include <openssl/crypto.h>
#include <openssl/evp.h>

#define ACVP_APP_HELP_MSG "Use acvp_app --help for more information."

static void print_usage(int code) {
    if (code == -1) {
        printf("\nInvalid usage...\n");
    } else {
        printf("\n===========================");
        printf("\n===== ACVP_APP USAGE ======");
        printf("\n===========================\n");
    }
    printf("To output version of library and of ACVP spec:\n");
    printf("      --version\n");
    printf("      -v\n");
    printf("Logging level decides the amount of information output by the library. Logging level\n");
    printf("can be controlled using:\n");
    printf("      --none\n");
    printf("      --error\n");
    printf("      --warn\n");
    printf("      --status(default)\n");
    printf("      --info\n");
    printf("      --verbose\n");
    printf("\n");
    if (code >= ACVP_LOG_LVL_VERBOSE) {
        printf("-The warn logging level logs events that should be acted upon but do not halt\n");
        printf("the progress of the application running.\n");
        printf("-The default logging level provides basic information about the progress of the test\n");
        printf("session or the task being performed. This includes the possibility of logging large\n");
        printf("amounts of data IF the data is specifically requested.\n");
        printf("-The info logging level provides more information about the information being\n");
        printf("exchanged, including HTTP actions (get, put, etc). Data in/from these actions is\n");
        printf("logged but usually truncated.\n");
        printf("-The verbose logging level is substantially more detailed than even info level, and\n");
        printf("includes information about each vector set, test group,and even test case being\n");
        printf("processed. it also will automatically fetch the results of all test cases of a\n");
        printf("vector set in the event of it failing.\n");
        printf("\n");
        printf("For any activity requiring the creation of a test session and/or the processing\n");
        printf("of test cases, acvp_app requires the specification of at least one algorithm\n");
        printf("suite. Algorithm suites are enabled or disabled at build time depending on the\n");
        printf("capabilities of the provided cryptographic library.\n\n");
    }
    printf("Algorithm Test Suites:\n");
    printf("Note: not all suites are supported by all supported modules\n");
    printf("      --all_algs (or -a, Enable all of the suites supported by the crypto module)\n");
    printf("      --aes\n");
    printf("      --tdes\n");
    printf("      --hash\n");
    printf("      --cmac\n");
    printf("      --hmac\n");
    printf("      --kdf\n");
    printf("      --dsa\n");
    printf("      --kas_ffc\n");
    printf("      --safe_primes\n");
    printf("      --rsa\n");
    printf("      --ecdsa\n");
    printf("      --eddsa\n");
    printf("      --drbg\n");
    printf("      --kas_ecc\n");
    printf("      --kas_ifc\n");
    printf("      --kda\n");
    printf("      --kts_ifc\n");
    printf("\n");

    printf("      If running hash, a maximum size for large data testing (LDT) may be required on specific\n");
    printf("      memory-limited platforms. This can be set (in GiB) using:\n");
    printf("            --set_max_hash_size <GiB value>\n");
    printf("      Setting 0 will disable LDT and only use the typical hash message sizes in the KiB range.\n");
    printf("\n");

    if (code >= ACVP_LOG_LVL_VERBOSE) {
        printf("libacvp generates a file containing information that can be used for various tasks regarding\n");
        printf("a test session. By default, this is usually placed in the folder of the executable utilizing\n");
        printf("libacvp, though this can be different on some OS. The name, by default, is\n");
        printf("testSession_(ID number).json. The path and prefix can be controlled using ACV_SESSION_SAVE_PATH\n");
        printf("and ACV_SESSION_SAVE_PREFIX in your environment, respectively. Any tasks listed below that use\n");
        printf("<session_file> are in reference to this file.\n");
        printf("\n");
    }
    printf("Perform a FIPS Validation for this testSession:\n");
    printf("      --fips_validation <full metadata file>\n");
    printf("\n");
    printf("To specify a cert number associated with all prerequistes:\n");
    printf("      --certnum <string>\n");
    printf("\n");
    printf("To register manually using a JSON file instead of application settings use:\n");
    printf("      --manual_registration <file>\n");
    printf("\n");
    printf("To retreive and output the JSON form of the currently registered capabilities:\n");
    printf("      --get_registration\n");
    printf("\n");
    printf("To register and save the vectors to file:\n");
    printf("      --vector_req <file>\n");
    printf("      -r <file>\n");
    printf("\n");
    printf("To process saved vectors and write results/responses to file:\n");
    printf("      --vector_req <file>\n");
    printf("      --vector_rsp <file>\n");
    printf("      OR\n");
    printf("      -r <file>\n");
    printf("      -p <file>\n");
    printf("\n");
    printf("To upload vector responses from file:\n");
    printf("      --vector_upload <file>\n");
    printf("      -u <file>\n");
    printf("\n");
    printf("Note: --resume_session and --get_results use the test session info file created automatically by the library as input\n");
    printf("\n");
    printf("To resume a previous test session that was interupted:\n");
    printf("      --resume_session <session_file>\n");
    printf("            Note: this does not save your arguments from your initial run and you MUST include them\n");
    printf("            again (e.x. --aes,  --vector_req and --fips_validation)\n");
    printf("\n");
    printf("To cancel a test session that was previously initiated:\n");
    printf("      --cancel_session <session_file>\n");
    printf("            Note: This will request the server to halt all processing and delete all info related to the\n");
    printf("            test session - It is not recoverable\n");
    printf("To get the results of a previous test session:\n");
    printf("      --get_results <session_file>\n");
    printf("\n");
    printf("To GET status of request, such as validation or metadata:\n");
    printf("      --get <request string URL including ID>\n");
    printf("\n");
    printf("To POST metadata for vendor, person, etc.:\n");
    printf("      --post <metadata file>\n");
    printf("\n");
    printf("To PUT(modify)  metadata for vendor, person, etc. or PUT for validation:\n");
    printf("      --put <metadata file>\n");
    printf("\n");
    printf("To request to DELETE a resource you have created on the server:\n");
    printf("      --delete <url>\n");
    printf("If you are running a sample registration (querying for correct answers\n");
    printf("in addition to the normal registration flow) use:\n");
    printf("      --sample\n");
    printf("\n");
    printf("To get the expected results of a sample test session:\n");
    printf("      --get_expected_results <session_file>\n");
    printf("\n");
    printf("Some other options may support outputting to log OR saving to a file. To save to a file:\n");
    printf("      --save_to <file>\n");
    printf("      -s <file>\n");
    printf("\n");
#if OPENSSL_VERSION_NUMBER >= 0x30000000L
    printf("To disable FIPS mode for this run (Note, a warning will be issued):\n");
    printf("      -disable_fips\n");
    printf("\n");
#endif
    printf("In addition some options are passed to acvp_app using\n");
    printf("environment variables.  The following variables can be set:\n\n");
    printf("    ACV_SERVER (when not set, defaults to %s)\n", DEFAULT_SERVER);
    printf("    ACV_PORT (when not set, defaults to %d)\n", DEFAULT_PORT);
    printf("    ACV_URI_PREFIX (when not set, defaults to %s)\n", DEFAULT_URI_PREFIX);
    printf("    ACV_TOTP_SEED (when not set, client will not use Two-factor authentication)\n");
    printf("    ACV_CA_FILE\n");
    printf("    ACV_CERT_FILE\n");
    printf("    ACV_KEY_FILE\n");
    printf("The CA certificates, cert and key should be PEM encoded. There should be no\n");
    printf("password on the key file.\n\n");
    printf("Some options can be passed to the library itself with environment variables:\n\n");
    printf("    ACV_SESSION_SAVE_PATH (Location where test session info files are saved)\n");
    printf("    ACV_SESSION_SAVE_PREFIX (Determines file name of info file, followed by ID number\n");
    printf("    The following are used by the library for an HTTP user-agent string, only when\n");
    printf("    the information cannot be automatically collected:\n");
    printf("        ACV_OE_OSNAME\n");
    printf("        ACV_OE_OSVERSION\n");
    printf("        ACV_OE_ARCHITECTURE\n");
    printf("        ACV_OE_PROCESSOR\n");
    printf("        ACV_OE_COMPILER\n\n");
}



static ko_longopt_t longopts[] = {
    { "version", ko_no_argument, 301 },
    { "help", ko_optional_argument, 302 },
    { "info", ko_no_argument, 303 },
    { "status", ko_no_argument, 304 },
    { "warn", ko_no_argument, 305 },
    { "error", ko_no_argument, 306 },
    { "verbose", ko_no_argument, 307 },
    { "none", ko_no_argument, 308 },
    { "sample", ko_no_argument, 309 },
    { "aes", ko_no_argument, 310 },
    { "tdes", ko_no_argument, 311 },
    { "hash", ko_no_argument, 312 },
    { "cmac", ko_no_argument, 313 },
    { "hmac", ko_no_argument, 314 },
    { "kdf", ko_no_argument, 315 },
    { "dsa", ko_no_argument, 316 },
    { "rsa", ko_no_argument, 317 },
    { "drbg", ko_no_argument, 318 },
    { "ecdsa", ko_no_argument, 319 },
    { "kas_ecc", ko_no_argument, 320 },
    { "kas_ffc", ko_no_argument, 321 },
    { "safe_primes", ko_no_argument, 322 },
    { "kas_ifc", ko_no_argument, 323 },
    { "kts_ifc", ko_no_argument, 324 },
    { "kda", ko_no_argument, 325 },
    { "kmac", ko_no_argument, 326 },
    { "lms", ko_no_argument, 327 },
    { "eddsa", ko_no_argument, 328 },
    { "all_algs", ko_no_argument, 350 },
    { "manual_registration", ko_required_argument, 400 },
    { "fips_validation", ko_required_argument, 402 },
    { "vector_req", ko_required_argument, 403 },
    { "vector_rsp", ko_required_argument, 404 },
    { "vector_upload", ko_required_argument, 405 },
    { "get", ko_required_argument, 406 },
    { "post", ko_required_argument, 407 },
    { "put", ko_required_argument, 408 },
    { "get_results", ko_required_argument, 409},
    { "certnum", ko_required_argument, 410 },
    { "resume_session", ko_required_argument, 411 },
    { "get_expected_results", ko_required_argument, 412 },
    { "save_to", ko_required_argument, 413 },
    { "delete", ko_required_argument, 414 },
    { "cancel_session", ko_required_argument, 415 },
    { "cost", ko_no_argument, 416 },
    { "debug", ko_no_argument, 417 },
    { "get_registration", ko_no_argument, 418 },
    { "set_max_hash_size", ko_required_argument, 419 },
#if OPENSSL_VERSION_NUMBER >= 0x30000000L
    { "disable_fips", ko_no_argument, 500 },
#endif
    { NULL, 0, 0 }
};


static void default_config(APP_CONFIG *cfg) {
    cfg->level = ACVP_LOG_LVL_STATUS;
    cfg->max_ldt_size = 8; /* Max in spec is 8 right now, do all by default */
}

static void enable_all_algorithms(APP_CONFIG *cfg) {
    cfg->aes = 1;
    cfg->tdes = 1;
    cfg->hash = 1;
    cfg->cmac = 1;
    cfg->hmac = 1;
    cfg->kmac = 1;
    cfg->dsa = 1;
    cfg->kas_ffc = 1;
    cfg->safe_primes = 1;
    cfg->rsa = 1;
    cfg->drbg = 1;
    cfg->ecdsa = 1;
    cfg->eddsa = 1;
    cfg->kas_ecc = 1;
    cfg->kas_ifc = 1;
    cfg->kda = 1;
    cfg->kts_ifc = 1;
    cfg->kdf = 1;
    cfg->lms = 1;
}

static const char* lookup_arg_name(int c) {
    int i = 0;
    int arrlen = sizeof(longopts) / sizeof(ko_longopt_t);
    for (i = 0; i < arrlen; i++) {
        if (longopts[i].val == c) {
            return longopts[i].name;
        }
    }
    return NULL;
}

//return 0 if fails check, 1 if passes
static int check_option_length(const char *opt, int c, int maxAllowed) {
    if ((int)strnlen_s(opt, maxAllowed + 1) > maxAllowed) {
        const char *argName = lookup_arg_name(c);
        printf(ANSI_COLOR_RED "Command error... "ANSI_COLOR_RESET
                "\nThe argument given for option %s is too long."
                "\nMax length allowed: %d"
                "\n%s\n", argName, maxAllowed, ACVP_APP_HELP_MSG);
        return 0;
    }
    return 1;
}

int ingest_cli(APP_CONFIG *cfg, int argc, char **argv) {
    ketopt_t opt = KETOPT_INIT;
    int c = 0, diff = 0, len = 0, print_ver = 0, ldt_manually_set = 0;

    cfg->empty_alg = 1;

    /* Set the default configuration values */
    default_config(cfg);

    while ((c = ketopt(&opt, argc, argv, 1, "vhas:u:r:p:", longopts)) >= 0) {
        diff = 1;

        switch (c) {
        case 'v':
        case 301:
            /* Print version info AFTER other args are read, so we can see module runtime info better */
            print_ver = 1;
            break;
        case 'h':
        case 302:
            if (opt.arg) {
                len = strnlen_s(opt.arg, JSON_FILENAME_LENGTH + 1);
                if (len > JSON_FILENAME_LENGTH || len <= 0) {
                    printf("invalid help option length\n");
                    return 1;
                }
                strncmp_s(opt.arg, len, "--verbose", 9, &diff);
                if (!diff) {
                    print_usage(ACVP_LOG_LVL_VERBOSE);
                } else {
                    print_usage(0);
                }
            } else { 
                print_usage(0);
            }
            return 1;
        case 303:
            cfg->level = ACVP_LOG_LVL_INFO;
            break;
        case 304:
            cfg->level = ACVP_LOG_LVL_STATUS;
            break;
        case 305:
            cfg->level = ACVP_LOG_LVL_WARN;
            break;
        case 306:
            cfg->level = ACVP_LOG_LVL_ERR;
            break;
        case 307:
            cfg->level = ACVP_LOG_LVL_VERBOSE;
            break;
        case 308:
            cfg->level = ACVP_LOG_LVL_NONE;
            break;
        case 309:
            cfg->sample = 1;
            break;
        case 310:
            cfg->aes = 1;
            cfg->empty_alg = 0;
            break;
        case 311:
            cfg->tdes = 1;
            cfg->empty_alg = 0;
            break;
        case 312:
            cfg->hash = 1;
            cfg->empty_alg = 0;
            break;
        case 313:
            cfg->cmac = 1;
            cfg->empty_alg = 0;
            break;
        case 314:
            cfg->hmac = 1;
            cfg->empty_alg = 0;
            break;
        case 315:
            cfg->kdf = 1;
            cfg->empty_alg = 0;
            break;
        case 316:
            cfg->dsa = 1;
            cfg->empty_alg = 0;
            break;
        case 317:
            cfg->rsa = 1;
            cfg->empty_alg = 0;
            break;
        case 318:
            cfg->drbg = 1;
            cfg->empty_alg = 0;
            break;
        case 319:
            cfg->ecdsa = 1;
            cfg->empty_alg = 0;
            break;
        case 320:
            cfg->kas_ecc = 1;
            cfg->empty_alg = 0;
            break;
        case 321:
            cfg->kas_ffc = 1;
            cfg->empty_alg = 0;
            break;
        case 322:
            cfg->safe_primes = 1;
            cfg->empty_alg = 0;
            break;
        case 323:
            cfg->kas_ifc = 1;
            cfg->empty_alg = 0;
            break;
        case 324:
            cfg->kts_ifc = 1;
            cfg->empty_alg = 0;
            break;
        case 325:
            cfg->kda = 1;
            cfg->empty_alg = 0;
            break;
        case 326:
            cfg->kmac = 1;
            cfg->empty_alg = 0;
            break;
        case 327:
            cfg->lms = 1;
            cfg->empty_alg = 0;
            break;
        case 328:
            cfg->eddsa = 1;
            cfg->empty_alg = 0;
            break;
        case 'a':
        case 350:
            enable_all_algorithms(cfg);
            cfg->empty_alg = 0;
            cfg->testall = 1;
            break;

        case 400:
            cfg->manual_reg = 1;
            if (!check_option_length(opt.arg, c, JSON_FILENAME_LENGTH)) {
                return 1;
            }
            strcpy_s(cfg->reg_file, JSON_FILENAME_LENGTH + 1, opt.arg);
            break;

        case 402:
            cfg->fips_validation = 1;
            if (!check_option_length(opt.arg, c, JSON_FILENAME_LENGTH)) {
                return 1;
            }
            strcpy_s(cfg->validation_metadata_file, JSON_FILENAME_LENGTH + 1, opt.arg);
            break;

        case 'r':
        case 403:
            cfg->vector_req = 1;
            if (!check_option_length(opt.arg, c, JSON_FILENAME_LENGTH)) {
                return 1;
            }
            strcpy_s(cfg->vector_req_file, JSON_FILENAME_LENGTH + 1, opt.arg);
            break;

        case 'p':
        case 404:
            cfg->vector_rsp = 1;
            if (!check_option_length(opt.arg, c, JSON_FILENAME_LENGTH)) {
                return 1;
            }

            strcpy_s(cfg->vector_rsp_file, JSON_FILENAME_LENGTH + 1, opt.arg);
            break;

        case 'u':
        case 405:
            cfg->vector_upload = 1;
            if (!check_option_length(opt.arg, c, JSON_FILENAME_LENGTH)) {
                return 1;
            }
            strcpy_s(cfg->vector_upload_file, JSON_FILENAME_LENGTH + 1, opt.arg);
            break;

        case 406:
            cfg->get = 1;
            if (!check_option_length(opt.arg, c, JSON_REQUEST_LENGTH)) {
                return 1;
            }
            strcpy_s(cfg->get_string, JSON_REQUEST_LENGTH + 1, opt.arg);
            break;

        case 407:
            cfg->post = 1;
            if (!check_option_length(opt.arg, c, JSON_FILENAME_LENGTH)) {
                return 1;
            }
            strcpy_s(cfg->post_filename, JSON_FILENAME_LENGTH + 1, opt.arg);
            break;

        case 408:
            cfg->put = 1;
            if (!check_option_length(opt.arg, c, JSON_FILENAME_LENGTH)) {
                return 1;
            }
            strcpy_s(cfg->put_filename, JSON_FILENAME_LENGTH + 1, opt.arg);
            break;

        case 409:
            cfg->get_results = 1;
            if (!check_option_length(opt.arg, c, JSON_FILENAME_LENGTH)) {
                return 1;
            }
            strcpy_s(cfg->session_file, JSON_FILENAME_LENGTH + 1, opt.arg);
            break;

        case 410:
            if (!check_option_length(opt.arg, c, JSON_STRING_LENGTH)) {
                return 1;
            }
            strcpy_s(value, JSON_STRING_LENGTH, opt.arg);
            break;

        case 411:
            cfg->resume_session = 1;
            if (!check_option_length(opt.arg, c, JSON_FILENAME_LENGTH)) {
                return 1;
            }
            strcpy_s(cfg->session_file, JSON_FILENAME_LENGTH + 1, opt.arg);
            break;

        case 412:
            cfg->get_expected = 1;
            if (!check_option_length(opt.arg, c, JSON_FILENAME_LENGTH)) {
                return 1;
            }
            strcpy_s(cfg->session_file, JSON_FILENAME_LENGTH + 1, opt.arg);
            break;

        case 's':
        case 413:
            cfg->save_to = 1;
            if (!check_option_length(opt.arg, c, JSON_FILENAME_LENGTH)) {
                return 1;
            }
            strcpy_s(cfg->save_file, JSON_FILENAME_LENGTH + 1, opt.arg);
            break;

        case 414:
            cfg->delete = 1;
            if (!check_option_length(opt.arg, c, JSON_REQUEST_LENGTH)) {
                return 1;
            }
            strcpy_s(cfg->delete_url, JSON_REQUEST_LENGTH + 1, opt.arg);
            break;

        case 415:
            cfg->cancel_session = 1;
            if (!check_option_length(opt.arg, c, JSON_FILENAME_LENGTH)) {
                return 1;
            }
            strcpy_s(cfg->session_file, JSON_FILENAME_LENGTH + 1, opt.arg);
            break;

        case 416:
            cfg->get_cost = 1;
            break;

        case 417:
            cfg->level = ACVP_LOG_LVL_DEBUG;
            break;

        case 418:
            cfg->get_reg = 1;
            break;

        case 419:
            len = 0;
            if (sscanf(opt.arg, "%d", &len) == 0) {
                printf("Error reading in max hash size: invalid argument provided\n");
                return 1;
            }
            if (len < 0 || len > 8) {
                printf("Provided max LDT size invalid (must be > 0 and <= 8)\n");
                return 1;
            }
            cfg->max_ldt_size = len;
            ldt_manually_set = 1;
            break;

#if OPENSSL_VERSION_NUMBER >= 0x30000000L
        case 500:
            cfg->disable_fips = 1;
            break;
#endif
        case '?':
            printf(ANSI_COLOR_RED "unknown option: %s\n"ANSI_COLOR_RESET, *(argv + opt.ind - !(opt.pos > 0)));
            printf("%s\n", ACVP_APP_HELP_MSG);
            return 1;

        case ':':
            printf(ANSI_COLOR_RED "option missing arg: %s\n"ANSI_COLOR_RESET, *(argv + opt.ind - 1));
            printf("%s\n", ACVP_APP_HELP_MSG);
            return 1;

        default:
            printf("An unknown error occurred while parsing arguments.\n");
            break;
        }
    }

    //If there are still arguments that were not permuted, they are invalid
    if (opt.ind < argc) {
        for (c = opt.ind; c < argc; c++) {
            printf(ANSI_COLOR_RED "unknown option: %s\n" ANSI_COLOR_RESET, argv[c]);
        }
        printf("%s\n", ACVP_APP_HELP_MSG);
        return 1;
    }

    if (print_ver) {
        print_version_info(cfg->disable_fips == 0);
        return 1;
    }

    if (ldt_manually_set && !cfg->hash) {
        printf("Warning: max hash LDT size specified, but hash not enabled. Ignoring provided value...\n");
        acvp_sleep(2);
    }

    //Many args do not need an alg specified. Todo: make cleaner
    if (cfg->empty_alg && !cfg->post && !cfg->get && !cfg->put && !cfg->get_results
            && !cfg->get_expected && !cfg->manual_reg && !cfg->vector_upload
            && !cfg->delete && !cfg->cancel_session && !(cfg->resume_session && 
            cfg->vector_req)) {
        /* The user needs to select at least 1 algorithm */
        printf(ANSI_COLOR_RED "Requires at least 1 Algorithm Test Suite\n"ANSI_COLOR_RESET);
        printf("%s\n", ACVP_APP_HELP_MSG);
        return 1;
    }

    printf("\n");

    return 0;
}

