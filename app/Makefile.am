tmp_sources = app_main.c \
              app_aes.c \
              app_cli.c \
              app_cmac.c \
              app_des.c \
              app_drbg.c \
              app_dsa.c \
              app_ecdsa.c \
              app_eddsa.c \
              app_hmac.c \
              app_kas.c \
              app_kdf.c \
              app_kda.c \
              app_kmac.c \
              app_rsa.c \
              app_sha.c \
              app_lms.c \
              app_utils.c \
              app_fips_lcl.h \
              app_fips_init_lcl.h \
              app_lcl.h \
              ketopt.h

if !BUILD_APP_AS_LIB
bin_PROGRAMS = acvp_app

acvp_app_includedir=$(includedir)/acvp
acvp_app_SOURCES = ${tmp_sources}
acvp_app_CFLAGS = -g -fPIE $(LIBACVP_CFLAGS) $(SSL_CFLAGS) $(FOM_CFLAGS) $(SAFEC_CFLAGS) $(COND_ALG_CFLAGS)
acvp_app_LDFLAGS = $(LIBACVP_LDFLAGS) $(SSL_LDFLAGS) $(FOM_LDFLAGS)
acvp_app_LDADD = $(ADDL_LIB_DEPENDENCIES)

if FORCE_STATIC
acvp_app_LDFLAGS += -all-static
endif

if ! BUILDING_OFFLINE
acvp_app_LDFLAGS += $(LIBCURL_LDFLAGS)
acvp_app_CFLAGS += $(LIBCURL_CFLAGS)
endif

if USE_FOM_OBJ
acvp_app_LDADD += $(FOM_OBJ_DIR)/fipscanister.o
endif

else
lib_LTLIBRARIES = libacvp_app.la
libacvp_app_includedir=${includedir}/acvp
libacvp_app_la_SOURCES = ${tmp_sources}

AM_CFLAGS = -g -fPIE $(LIBACVP_CFLAGS) $(SSL_CFLAGS) $(FOM_CFLAGS) $(SAFEC_CFLAGS) $(COND_ALG_CFLAGS) -DACVP_APP_LIB_WRAPPER
libacvp_app_la_LIBADD = $(LIBACVP_LDFLAGS) $(SSL_LDFLAGS) $(FOM_LDFLAGS)

libacvp_app_include_HEADERS = $(top_srcdir)/app/libacvp_app.h

if USE_FOM_OBJ
libacvp_app_la_LIBADD += $(FOM_OBJ_DIR)/fipscanister.o
endif
endif
