#include "term_renderer.h"

#include <ncurses.h>


// Define brightness levels.
static const char LEVELS[] = " .,:-=+*#%@";
static const unsigned char N_LEVELS = sizeof(LEVELS) / sizeof(char) - 1;

typedef struct {
    unsigned int width, height;
    unsigned int x, y;
} TRenderer;

static TRenderer trenderer = { 0 };

int trenderer_init(unsigned int width, unsigned int height) {
    unsigned int mrows, mcols;
    initscr();
    cbreak();
    noecho();
    curs_set(0);
    getmaxyx(stdscr, mrows, mcols);
    if (mrows < (height + 1) || mcols < (width + 1)) {
        fprintf(stderr, "Error. The width or height specified: \"%u, %u\" are greater than the terminal dimensions: \"%u, %u\".\n",
            width, height, mcols, mrows);
            trenderer_deinit();
        return 1;
    }
    trenderer.width = width;
    trenderer.height = height;

    trenderer.x = (mcols - width) / 2 + 1;
    trenderer.y = (mrows - height) / 2 + 1;
    unsigned int x = trenderer.x - 1;
    unsigned int y = trenderer.y - 1;
    clear();
    // Make the frame
    for (size_t i = x; i < width + x; i++) {
        mvaddch(y, i, '-');
        mvaddch(y + height, i, '-');
    }
    for (size_t i = y; i < height + y; i++) {
        mvaddch(i, x, '|');
        mvaddch(i, x + width, '|');
    }
    mvaddch(y, x, '+');
    mvaddch(y + height, x, '+');
    mvaddch(y, x + width, '+');
    mvaddch(y + height, x + width, '+');

    return 0;
}


void trenderer_deinit(void) {
    endwin();
}


void trenderer_render(float* frame) {
    for (size_t i = trenderer.y; i < trenderer.height + trenderer.y - 1; i++) {
        for (size_t j = trenderer.x; j < trenderer.width + trenderer.x - 1; j++) {
            unsigned int level = (unsigned int)((frame[j + trenderer.width*i] * N_LEVELS));
            char c = LEVELS[level < N_LEVELS ? level : (N_LEVELS - (unsigned int)1)];
            mvaddch(i, j, c);
        }
    }
    refresh();
}