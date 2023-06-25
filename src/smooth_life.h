#pragma once

#include <stddef.h>

// Configuration struct
typedef struct {
    size_t n_threads;                 
    unsigned int width, height;
    float init_percent_x, init_percent_y;
    float ra, ri;                           // If ri is set < 0 it is assumed to be ri = ra / 3.0f
    float b1, d1, b2, d2, alpha_m, alpha_n;
    float dt;
} SMConfig;

// State
typedef struct SMState SMState;

SMState* sm_init(SMConfig* conf);
void sm_deinit(SMState* state);
void state_step(SMState* s);
void state_step_nothreads(SMState* s);
float* sm_get_raw_state(SMState* s);