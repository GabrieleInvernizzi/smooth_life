#include "term_renderer.h"

#include <stdio.h>

// Define brightness levels.
static const char LEVELS[] = " .,:-=+*#%@";
static const unsigned char N_LEVELS = sizeof(LEVELS) / sizeof(char) - 1;

// Renderer state.
typedef struct {
    unsigned int width;
    unsigned int height;
} TRenderer;

static TRenderer trenderer = { 0 };

void trenderer_init(unsigned int width, unsigned int height) {
    trenderer.width = width;
    trenderer.height = height;
}


void trenderer_render(const float** frame) {
    printf("Helloo.\n");
}
