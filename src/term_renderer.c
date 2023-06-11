#include "term_renderer.h"

#include <stdlib.h>
#include <stdio.h>

// Define brightness levels.
static const char LEVELS[] = " .,:-=+*#%@";
static const unsigned char N_LEVELS = sizeof(LEVELS) / sizeof(char) - 1;

// Renderer state.
typedef struct {
    unsigned int width;
    unsigned int height;
    unsigned int buf_len;
    char *buffer;
} TRenderer;

static TRenderer trenderer = { 0 };

static inline void trenderer_set_buf_at(unsigned int i, unsigned int j, char val) {
    trenderer.buffer[j + i * (trenderer.width + 1)] = val;
}

static inline char trenderer_get_buf_at(unsigned int i, unsigned int j) {
    return trenderer.buffer[j + i * (trenderer.width + 1)];
}

void trenderer_init(unsigned int width, unsigned int height) {
    trenderer.width = width;
    trenderer.height = height;
    trenderer.buf_len = (width + 1) * height;
    trenderer.buffer = malloc(trenderer.buf_len * sizeof(char));
    for (unsigned int i = 0; i < height; i++)
        trenderer_set_buf_at(i, width, '\n');
}

void trenderer_deinit(void) {
    if (trenderer.buffer) free(trenderer.buffer);

    trenderer.buf_len = 0;
    trenderer.width = 0;
    trenderer.height = 0;
}

void trenderer_render(float* frame) {
    for (unsigned int i = 0; i < trenderer.height; i++) {
        for (unsigned int j = 0; j < trenderer.width; j++) {
            unsigned int level = (unsigned int)((frame[j + trenderer.width*i] * N_LEVELS));
            trenderer_set_buf_at(i, j, LEVELS[level < N_LEVELS ? level : (N_LEVELS - (unsigned int)1)]);
        }
    }
    fwrite(trenderer.buffer, sizeof(char), trenderer.buf_len, stdout);
}
