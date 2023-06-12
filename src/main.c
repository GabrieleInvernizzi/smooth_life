#include "term_renderer.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define WIDTH (100)
#define HEIGHT (40)


typedef struct {
    float ri, ra, b1, d1, b2, d2, alpha_m, alpha_n, dt;
} Config;

static Config conf = {
    .ri = 11.0f/3.0f, .ra = 11.0f, 
    .b1 = .278f, .d1 = .267f, 
    .b2 = .365f, .d2 = .445f,
    .alpha_m = .147f, .alpha_n = .028f,
    .dt = .005f
};
static float* state;

float rand_float(void) {
    return (float)rand() / (float)RAND_MAX;
}

void state_init(void) { 
    srand(time(0));

    state = calloc(WIDTH * HEIGHT, sizeof(float));

    unsigned int w = WIDTH * 0.7;
    unsigned int h = HEIGHT * 0.7;
    for (unsigned int di = 0; di < h; di++) {
        for (unsigned int dj = 0; dj < w; dj++) {
            unsigned int j = dj + WIDTH / 2 - w / 2;
            unsigned int i = di + HEIGHT / 2 - h / 2;
            state[j + WIDTH*i] = rand_float();
        }
    }
}

void state_deinit(void) {
    if (state) free(state);
}

void clamp(float *x, float l, float h)
{
    if (*x < l) *x = l;
    if (*x > h) *x = h;
}

unsigned int mod(int a, int b) {
    return (a % b + b) % b;
}

float sigmoid(float x, float a, float alpha) {
    return 1.0f / (1.0f + expf(-(x - a) * 4 / alpha));
}

float sigmoidn(float x, float a, float b) {
    return sigmoid(x, a, conf.alpha_n) * (1 - sigmoid(x, b, conf.alpha_n));
}

float sigmoidm(float x, float y, float m) {
    float sigm = sigmoid(m, 0.5f, conf.alpha_m);
    return x * (1 - sigm) + y * sigm;
}

float transition_fn(float n, float m) {
    return sigmoidn(n, sigmoidm(conf.b1, conf.d1, m), sigmoidm(conf.b2, conf.d2, m));
}


void calculate_nm(float* n_res, float* m_res, unsigned int ci, unsigned int cj) {
    int n_norm = 0;
    int m_norm = 0;
    float m = 0.0f;
    float n = 0.0f;
    
    for (int i = -(int)(conf.ra - 1); i < (int)(conf.ra - 1); i++) {
        for (int j = -(int)(conf.ra - 1); j < (int)(conf.ra - 1); j++) {
            unsigned int y = mod(i + ci, HEIGHT);
            unsigned int x = mod(j + cj, WIDTH); 
            float dist_squared = i*i + j*j;
            if (dist_squared <= conf.ri*conf.ri) {
                m += state[x + WIDTH*y];
                m_norm++;
            } else if (dist_squared <= conf.ra*conf.ra) {
                n += state[x + WIDTH*y];
                n_norm++;
            }
        }
    }
    if (n_norm != 0) n /= n_norm;
    if (m_norm != 0) m /= m_norm;

    *n_res = n;
    *m_res = m;
}

void state_step(void) {
    float n, m;
    for (unsigned int ci = 0; ci < HEIGHT; ci++) {
        for (unsigned int cj = 0; cj < WIDTH; cj++) {
            calculate_nm(&n, &m, ci, cj);
            float dstate = 2 * transition_fn(n, m) - 1;
            state[cj + WIDTH*ci] += conf.dt * dstate;
            clamp(&state[cj + WIDTH*ci], 0.0f, 1.0f); 
        }
    }
}


int main(const int argc, const char** argv) {
    state_init();
    trenderer_init(WIDTH, HEIGHT);

    while(1) {
        state_step();
        trenderer_render(state);
    }

    trenderer_deinit();
    state_deinit();
}