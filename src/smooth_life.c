#include "smooth_life.h"

#include <stdlib.h>
#include <math.h>

struct SMState {
    unsigned int width, height;
    float ra, ri;
    float b1, d1, b2, d2, alpha_m, alpha_n;
    float dt;
    float* state;
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


SMState* sm_init(SMConfig* conf) {
    SMState* s = malloc(sizeof(SMState));
    if (!s) return s;
    s->width   = conf->width;
    s->height  = conf->height;
    s->ra      = conf->ra;
    s->ri      = conf->ri < 0.0f ? conf->ra / 3.0f : conf->ri;
    s->b1      = conf->b1;
    s->d1      = conf->d1;
    s->b2      = conf->b2;
    s->d2      = conf->d2;
    s->alpha_m = conf->alpha_m;
    s->alpha_n = conf->alpha_n;
    s->dt      = conf->dt;

    s->state = calloc(conf->width * conf->height, sizeof(float));
    if (!s->state) {
        free(s);
        return NULL;
    }

    unsigned int w = conf->width * conf->init_percent_x;
    unsigned int h = conf->height * conf->init_percent_y;
    for (size_t di = 0; di < h; di++) {
        for (size_t dj = 0; dj < w; dj++) {
            unsigned int j = dj + conf->width / 2 - w / 2;
            unsigned int i = di + conf->height / 2 - h / 2;
            s->state[j + conf->width*i] = rand_float();
        }
    }

    return s;
}


void sm_deinit(SMState* s) {
    if (s) {
        if (s->state) free(s->state);
        free(s);
    }
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


void state_step(SMState* s) {
    float n, m;
    for (size_t ci = 0; ci < s->height; ci++) {
        for (size_t cj = 0; cj < s->width; cj++) {
            calculate_nm(s, &n, &m, ci, cj);
            float dstate = 2 * transition_fn(s, n, m) - 1;
            s->state[cj + s->width*ci] += s->dt * dstate;
            clamp(&s->state[cj + s->width*ci], 0.0f, 1.0f); 
        }
    }
}



float* sm_get_raw_state(SMState* s) {
    return s->state;
}