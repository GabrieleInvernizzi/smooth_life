#include "smooth_life.h"

#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>

#include <stdio.h>

#include "thread_pool.h"

typedef struct {
    size_t i_min, i_max;
    SMState* state;
} SMTask;

struct SMState {
    size_t n_threads;
    unsigned int width, height;
    float ra, ri;
    float b1, d1, b2, d2, alpha_m, alpha_n;
    float dt;
    float* state;
    float* state_out;
    bool is_state_swapped;
    SMTask* tasks;
    ThreadPool* tp;
};


static float rand_float(void) {
    return (float)rand() / (float)RAND_MAX;
}

static void clamp(float *x, float l, float h) {
    if (*x < l) *x = l;
    if (*x > h) *x = h;
}

static unsigned int mod(int a, int b) {
    return (a % b + b) % b;
}

static float sigmoid(float x, float a, float alpha) {
    return 1.0f / (1.0f + expf(-(x - a) * 4 / alpha));
}

static float sigmoidn(float x, float a, float b, float alpha) {
    return sigmoid(x, a, alpha) * (1 - sigmoid(x, b, alpha));
}

static float sigmoidm(float x, float y, float m, float alpha) {
    float sigm = sigmoid(m, 0.5f, alpha);
    return x * (1 - sigm) + y * sigm;
}

static float transition_fn(SMState* s, float n, float m) {
    return sigmoidn(n, sigmoidm(s->b1, s->d1, m, s->alpha_m), sigmoidm(s->b2, s->d2, m, s->alpha_m), s->alpha_n);
}

static void calculate_nm(SMState* s, float* n_res, float* m_res, unsigned int ci, unsigned int cj) {
    int n_norm = 0;
    int m_norm = 0;
    float m = 0.0f;
    float n = 0.0f;
    
    for (int i = -(int)(s->ra - 1); i < (int)(s->ra - 1); i++) {
        for (int j = -(int)(s->ra - 1); j < (int)(s->ra - 1); j++) {
            unsigned int y = mod(i + ci, s->height);
            unsigned int x = mod(j + cj, s->width); 
            float dist_squared = i*i + j*j;
            if (dist_squared <= s->ri*s->ri) {
                m += s->state[x + s->width*y];
                m_norm++;
            } else if (dist_squared <= s->ra*s->ra) {
                n += s->state[x + s->width*y];
                n_norm++;
            }
        }
    }
    if (n_norm != 0) n /= n_norm;
    if (m_norm != 0) m /= m_norm;

    *n_res = n;
    *m_res = m;
}


SMState* sm_init(SMConfig* conf) {
    SMState* s = malloc(sizeof(SMState));
    if (!s) return NULL;
    s->n_threads = conf->n_threads > 0 ? conf->n_threads : (size_t)(sysconf(_SC_NPROCESSORS_ONLN) - 1);
    s->width     = conf->width;
    s->height    = conf->height;
    s->ra        = conf->ra;
    s->ri        = conf->ri < 0.0f ? conf->ra / 3.0f : conf->ri;
    s->b1        = conf->b1;
    s->d1        = conf->d1;
    s->b2        = conf->b2;
    s->d2        = conf->d2;
    s->alpha_m   = conf->alpha_m;
    s->alpha_n   = conf->alpha_n;
    s->dt        = conf->dt;

    s->state = calloc(conf->width * conf->height, sizeof(float));
    if (!s->state) {
        free(s);
        return NULL;
    }
    s->state_out = calloc(conf->width * conf->height, sizeof(float));
    if (!s->state_out) {
        free(s->state);
        free(s);
        return NULL;
    }
    s->is_state_swapped = false;

    unsigned int w = conf->width * conf->init_percent_x;
    unsigned int h = conf->height * conf->init_percent_y;
    for (size_t di = 0; di < h; di++) {
        for (size_t dj = 0; dj < w; dj++) {
            unsigned int j = dj + conf->width / 2 - w / 2;
            unsigned int i = di + conf->height / 2 - h / 2;
            s->state[j + conf->width*i] = rand_float();
        }
    }

    if (!(s->tp = tp_init(
        s->n_threads,
        s->n_threads + 5
    ))) 
    {
        sm_deinit(s);
        return NULL;
    }
    
    s->tasks = malloc(s->n_threads * sizeof(SMTask));
    if (!s->tasks) {
        sm_deinit(s);
        return NULL;
    }

    return s;
}

void sm_deinit(SMState* s) {
    if (s) {
        if (s->state) free(s->state);
        if (s->state_out) free(s->state_out);
        tp_deinit(s->tp);
        if (s->tasks) free(s->tasks);
        free(s);
    }
}


static inline void state_swap(SMState* s) {
    float* tmp_state = s->state;
    s->state = s->state_out;
    s->state_out = tmp_state;
    s->is_state_swapped ^= 1;
}

static void state_step_task(void* args) {
    SMTask* task = args;
    float n, m;

    const unsigned int width = task->state->width;

    for (size_t i = task->i_min; i < task->i_max; i++) {
        size_t ci = i / width;
        size_t cj = i % width;
        calculate_nm(task->state, &n, &m, ci, cj);
        float dstate = 2 * transition_fn(task->state, n, m) - 1;
        task->state->state_out[i] = task->state->state[i] + 
                                        task->state->dt * dstate;

        clamp(&task->state->state_out[i], 0.0f, 1.0f);
    }
}

void state_step_nothreads(SMState* s) {
    float n, m;
    for (size_t ci = 0; ci < s->height; ci++) {
        for (size_t cj = 0; cj < s->width; cj++) {
            calculate_nm(s, &n, &m, ci, cj);
            float dstate = 2 * transition_fn(s, n, m) - 1;
            s->state[cj + s->width*ci] += s->dt * dstate;
            clamp(&s->state[cj + s->width*ci], 0.0f, 1.0f);
        }
    }
    exit(0);
}

void state_step(SMState* s) {
    bool add_one = false;
    size_t elems_per_thread = (s->width * s->height) / s->n_threads;
    if ((s->width * s->height) % s->n_threads)
        add_one = true;

    for (size_t i = 0; i < s->n_threads; i++) {
        size_t index_min = i * elems_per_thread;
        s->tasks[i].state = s;
        s->tasks[i].i_min = index_min;
        s->tasks[i].i_max = index_min + elems_per_thread - 1;
        
        tp_add_task(s->tp, state_step_task, (void*)&s->tasks[i]);
    }

    tp_wait(s->tp);

    state_swap(s);
}

float* sm_get_raw_state(SMState* s) {
    if (!s->is_state_swapped)
        return s->state;
    else 
        return s->state_out;
}