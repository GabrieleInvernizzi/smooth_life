#include "config.h"

#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#include "tomlc99/toml.h"


/*
    SMConfig conf = {
        .ex_policy = SM_THREAD_POOL,
        .n_threads = 0,
        .width = 173, .height = 41,
        .init_percent_x = 0.4, .init_percent_y = 0.4,
        .ri = -1.0f, .ra = 7.0f, 
        .b1 = .278f, .d1 = .267f, 
        .b2 = .365f, .d2 = .445f,
        .alpha_m = .147f, .alpha_n = .028f,
        .dt = .05f
    };
*/


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

    current_val = toml_string_in(table, "execution_policy");
    PARSE_CONF_CHECK(current_val, "execution_policy")
    if (strcmp(str_to_lower(current_val.u.s), "thread_pool") == 0)
        conf->ex_policy = SM_THREAD_POOL;
    else if (strcmp(str_to_lower(current_val.u.s), "singlethreaded") == 0)
        conf->ex_policy = SM_SINGLETHREADED;
    else if (strcmp(str_to_lower(current_val.u.s), "omp") == 0)
        conf->ex_policy = SM_OMP;
    else {
        fprintf(stderr, "No execution policies named: '%s'", current_val.u.s);
        toml_free((toml_table_t*)table);
        free(current_val.u.s);
        return -1;
    }
    free(current_val.u.s);

    current_val = toml_int_in(table, "n_threads");
    PARSE_CONF_CHECK(current_val, "n_threads")
    conf->n_threads = (size_t)current_val.u.i;

    current_val = toml_double_in(table, "init_percent_x");
    PARSE_CONF_CHECK(current_val, "init_percent_x");
    conf->init_percent_x = (float)current_val.u.d;

    current_val = toml_double_in(table, "init_percent_y");
    PARSE_CONF_CHECK(current_val, "init_percent_y");
    conf->init_percent_y = (float)current_val.u.d;

    current_val = toml_double_in(table, "ra");
    PARSE_CONF_CHECK(current_val, "ra");
    conf->ra = (float)current_val.u.d;

    current_val = toml_double_in(table, "ri");
    PARSE_CONF_CHECK(current_val, "ri");
    conf->ri = (float)current_val.u.d;

    current_val = toml_double_in(table, "b1");
    PARSE_CONF_CHECK(current_val, "b1");
    conf->b1 = (float)current_val.u.d;

    current_val = toml_double_in(table, "d1");
    PARSE_CONF_CHECK(current_val, "d1");
    conf->d1 = (float)current_val.u.d;

    current_val = toml_double_in(table, "b2");
    PARSE_CONF_CHECK(current_val, "b2");
    conf->b2 = (float)current_val.u.d;

    current_val = toml_double_in(table, "d2");
    PARSE_CONF_CHECK(current_val, "d2");
    conf->d2 = (float)current_val.u.d;
    
    current_val = toml_double_in(table, "alpha_n");
    PARSE_CONF_CHECK(current_val, "alpha_n");
    conf->alpha_n = (float)current_val.u.d;

    current_val = toml_double_in(table, "alpha_m");
    PARSE_CONF_CHECK(current_val, "alpha_m");
    conf->alpha_m = (float)current_val.u.d;

    current_val = toml_double_in(table, "dt");
    PARSE_CONF_CHECK(current_val, "dt");
    conf->dt = (float)current_val.u.d;

    return 0;
}

#undef PARSE_CONF_CHECK

int gen_config(Config* conf, int argc, const char** argv) {
    FILE* file = NULL;
    
    conf->max_fps = 30;                 // TEMP

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