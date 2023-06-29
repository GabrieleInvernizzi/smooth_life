#include "config.h"

#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#include "tomlc99/toml.h"

#include "defaults.h"


static inline char* str_to_lower(char* s) {
    for(size_t i = 0; s[i]; i++)
        s[i] = tolower(s[i]);
    return s;
}

#define PARSE_CONF_ERRBUF_SIZE (128)

#define PARSE_CONF_CHECK(dataval, val_str) \
    if (!dataval.ok) {\
        fprintf(stderr, "Error parsing '" val_str "'.\n");\
        toml_free((toml_table_t*)table);\
        return -1;\
    }

static int parse_conf(SMConfig* conf, FILE* f) {
    toml_datum_t current_val;
    char errbuf[PARSE_CONF_ERRBUF_SIZE];

    const toml_table_t* table = toml_parse_file(f, errbuf, sizeof(errbuf));
    if (!conf) {
        fprintf(stderr, "Error. Can't parse the file - %s\n", errbuf);
        return -1;
    }

    if (toml_key_exists(table, "execution_policy")) {
        current_val = toml_string_in(table, "execution_policy");
        PARSE_CONF_CHECK(current_val, "execution_policy")
        str_to_lower(current_val.u.s);
        if (strcmp(current_val.u.s, "thread_pool") == 0 || strcmp(current_val.u.s, "thread pool") == 0)
            conf->ex_policy = SM_THREAD_POOL;
        else if (strcmp(current_val.u.s, "singlethreaded") == 0)
            conf->ex_policy = SM_SINGLETHREADED;
        else if (strcmp(current_val.u.s, "omp") == 0 || strcmp(current_val.u.s, "openmp") == 0)
            conf->ex_policy = SM_OMP;
        else {
            fprintf(stderr, "No execution policies named: '%s'", current_val.u.s);
            toml_free((toml_table_t*)table);
            free(current_val.u.s);
            return -1;
        }
        free(current_val.u.s);
    }


    if (toml_key_exists(table, "n_threads")) {
        current_val = toml_int_in(table, "n_threads");
        PARSE_CONF_CHECK(current_val, "n_threads")
        conf->n_threads = (size_t)current_val.u.i;
    }

    if (toml_key_exists(table, "init_percent_x")) {
        current_val = toml_double_in(table, "init_percent_x");
        PARSE_CONF_CHECK(current_val, "init_percent_x");
        conf->init_percent_x = (float)current_val.u.d;
    }

    if (toml_key_exists(table, "init_percent_y")) {
        current_val = toml_double_in(table, "init_percent_y");
        PARSE_CONF_CHECK(current_val, "init_percent_y");
        conf->init_percent_y = (float)current_val.u.d;
    }

    if (toml_key_exists(table, "ra")) {
        current_val = toml_double_in(table, "ra");
        PARSE_CONF_CHECK(current_val, "ra");
        conf->ra = (float)current_val.u.d;
    }

    if (toml_key_exists(table, "ri")) {
        current_val = toml_double_in(table, "ri");
        PARSE_CONF_CHECK(current_val, "ri");
        conf->ri = (float)current_val.u.d;
    }

    if (toml_key_exists(table, "b1")) {
        current_val = toml_double_in(table, "b1");
        PARSE_CONF_CHECK(current_val, "b1");
        conf->b1 = (float)current_val.u.d;
    }

    if (toml_key_exists(table, "d1")) {
        current_val = toml_double_in(table, "d1");
        PARSE_CONF_CHECK(current_val, "d1");
        conf->d1 = (float)current_val.u.d;
    }

    if (toml_key_exists(table, "b2")) {
        current_val = toml_double_in(table, "b2");
        PARSE_CONF_CHECK(current_val, "b2");
        conf->b2 = (float)current_val.u.d;
    }

    if (toml_key_exists(table, "d2")) {
        current_val = toml_double_in(table, "d2");
        PARSE_CONF_CHECK(current_val, "d2");
        conf->d2 = (float)current_val.u.d;
    }
    
    if (toml_key_exists(table, "alpha_n")) {
        current_val = toml_double_in(table, "alpha_n");
        PARSE_CONF_CHECK(current_val, "alpha_n");
        conf->alpha_n = (float)current_val.u.d;
    }

    if (toml_key_exists(table, "alpha_m")) {
        current_val = toml_double_in(table, "alpha_m");
        PARSE_CONF_CHECK(current_val, "alpha_m");
        conf->alpha_m = (float)current_val.u.d;
    }

    if (toml_key_exists(table, "dt")) {
        current_val = toml_double_in(table, "dt");
        PARSE_CONF_CHECK(current_val, "dt");
        conf->dt = (float)current_val.u.d;
    }

    return 0;
}

#undef PARSE_CONF_CHECK

int gen_config(Config* conf, int argc, const char** argv) {
    FILE* file = NULL;
    
    // Init conf to defualt
    conf->max_fps = DEFAULT_MAX_FPS;
    conf->sm_conf.ex_policy = DEFAULT_SM_EX_POLICY;
    conf->sm_conf.n_threads = DEFAULT_SM_N_THREADS;
    conf->sm_conf.width = 0;
    conf->sm_conf.height = 0;
    conf->sm_conf.init_percent_x = DEFAULT_SM_INIT_PERCENT_X;
    conf->sm_conf.init_percent_y = DEFAULT_SM_INIT_PERCENT_Y;
    conf->sm_conf.ra = DEFAULT_SM_RA;
    conf->sm_conf.ri = DEFAULT_SM_RI;
    conf->sm_conf.b1 = DEFUALT_SM_B1;
    conf->sm_conf.d1 = DEFUALT_SM_D1;
    conf->sm_conf.b2 = DEFUALT_SM_B2;
    conf->sm_conf.d2 = DEFUALT_SM_D2;
    conf->sm_conf.alpha_m = DEFAULT_SM_ALPHA_M;
    conf->sm_conf.alpha_n = DEFAULT_SM_ALPHA_N;
    conf->sm_conf.dt = DEFAULT_SM_DT;

    if (argc == 1)
        return 0;

    if (argc > 2) {
        fprintf(stderr, "Too many arguments.\n");
        return -1;
    }

    file = fopen(argv[1], "r");
    if (!file) {
        fprintf(stderr, "Cannot open the file: '%s'\n", argv[1]);      // TODO: better errors
        return -1;
    }
    int parse_res = parse_conf(&conf->sm_conf, file);
    fclose(file);
    if (parse_res == -1)
        return -1;


    return 0;
}