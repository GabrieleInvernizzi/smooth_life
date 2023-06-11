#include "term_renderer.h"

#include <stdlib.h>
#include <time.h>

#define WIDTH (30)
#define HEIGHT (20)

static float* state = NULL;

float rand_float(void) {
    return (float)rand() / (float)RAND_MAX;
}

void state_init(void) { 
    srand(time(0));

    float *buffer = malloc(WIDTH * HEIGHT * sizeof(float));
    for (unsigned int i = 0; i < HEIGHT; i++) {
        for (unsigned int j = 0; j < WIDTH; j++)
            buffer[j + WIDTH*i] = rand_float();
    }
}

void state_deinit(void) {
    if (state) free(state);
}



int main(const int argc, const char** argv) {
    state_init();

    trenderer_init(WIDTH, HEIGHT);
    trenderer_render(state);    
    trenderer_deinit();
    state_deinit();
}