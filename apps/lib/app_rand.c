/*
 * Copyright 1995-2018 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

#include "apps.h"
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#include <openssl/conf.h>

static char *save_rand_file;
static char *files_to_load;

void app_RAND_load_conf(CONF *c, const char *section)
{
    const char *randfile = NCONF_get_string(c, section, "RANDFILE");

    if (randfile == NULL) {
        ERR_clear_error();
        return;
    }
    if (RAND_load_file(randfile, -1) < 0) {
        BIO_printf(bio_err, "Can't load %s into RNG\n", randfile);
        ERR_print_errors(bio_err);
    }
    if (save_rand_file == NULL)
        save_rand_file = OPENSSL_strdup(randfile);
}

int app_RAND_load(void)
{
    char *p, *save;
    int last, ret = 1;

    if (files_to_load == NULL)
        return 1;

    save = files_to_load;
    for ( ; ; ) {
        last = 0;
        for (p = files_to_load; *p != '\0' && *p != LIST_SEPARATOR_CHAR; p++)
            continue;
        if (*p == '\0')
            last = 1;
        *p = '\0';
        if (RAND_load_file(files_to_load, -1) < 0) {
            BIO_printf(bio_err, "Can't load %s into RNG\n", files_to_load);
            ERR_print_errors(bio_err);
            ret = 0;
        }
        if (last)
            break;
        files_to_load = p + 1;
        if (*files_to_load == '\0')
            break;
    }
    files_to_load = NULL;
    OPENSSL_free(save);
    return ret;
}

void app_RAND_write(void)
{
    if (save_rand_file == NULL)
        return;
    if (RAND_write_file(save_rand_file) == -1) {
        BIO_printf(bio_err, "Cannot write random bytes:\n");
        ERR_print_errors(bio_err);
    }
    OPENSSL_free(save_rand_file);
    save_rand_file =  NULL;
}


/*
 * See comments in opt_verify for explanation of this.
 */
enum r_range { OPT_R_ENUM };

int opt_rand(int opt)
{
    switch ((enum r_range)opt) {
    case OPT_R__FIRST:
    case OPT_R__LAST:
        break;
    case OPT_R_RAND:
        files_to_load = opt_arg();
        break;
    case OPT_R_WRITERAND:
        OPENSSL_free(save_rand_file);
        save_rand_file = OPENSSL_strdup(opt_arg());
        break;
    }
    return 1;
}
