#include "term_renderer.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define WIDTH (30)
#define HEIGHT (20)

typedef struct {
    float ri, ra;
} Config;

static Config conf = {
    .ri = 7, .ra = 7*3
};
static float* state;

float rand_float(void) {
    return (float)rand() / (float)RAND_MAX;
}

void state_init(void) { 
    srand(time(0));

    state = malloc(WIDTH * HEIGHT * sizeof(float));
    for (unsigned int i = 0; i < (HEIGHT * WIDTH); i++)
            state[i] = rand_float();
}

void state_deinit(void) {
    if (state) free(state);
}


unsigned int mod(int a, int b) {
    return (a % b + b) % b;
}

void state_step(void) {
    // Calculate n and m
    int n_norm = 0;
    int m_norm = 0;
    float n = 0.0f;
    float m = 0.0f;
    unsigned int ci = 0;
    unsigned int cj = 0;
    for (int i = -(int)conf.ra; i < (int)conf.ra; i++) {
        for (int j = -(int)conf.ra; j < (int)conf.ra; j++) {
            unsigned int y = mod(i + ci, HEIGHT);
            unsigned int x = mod(j + cj, WIDTH); 
            float dist_squared = x*x + y*y;
            if (dist_squared < conf.ri*conf.ri) {
                m += state[x + WIDTH*y];
                m_norm++;
            } else if (dist_squared < conf.ra*conf.ra) {
                n += state[x + WIDTH*y];
                n_norm++;
            }
        }
    }
    n /= n_norm;
    m /= m_norm;
}


int main(const int argc, const char** argv) {
    state_init();

    trenderer_init(WIDTH, HEIGHT);
    trenderer_render(state);    
    trenderer_deinit();
    state_step();
    state_deinit();
}