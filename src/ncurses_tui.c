#include "tui.h"

#include <ncurses.h>

// Define brightness levels.
static const char LEVELS[] = " .,:-=+*#%@";
static const unsigned char N_LEVELS = sizeof(LEVELS) / sizeof(char) - 1;

typedef struct {
    unsigned int width, height;
    unsigned int x, y;
    TUIEvent last_event;
} TUI;

static TUI tui = { 0 };

#define TUI_MIN_ROWS (12)
#define TUI_MIN_COLS (42)

int tui_init(unsigned int* width_out, unsigned int* height_out, TUITitleInfo* title_info) {
    unsigned int mrows, mcols;
    unsigned int width, height;

    if (initscr() == NULL) return -1;
    cbreak();
    noecho();
    curs_set(0);
    nodelay(stdscr, TRUE);
    getmaxyx(stdscr, mrows, mcols);
    if (mrows < TUI_MIN_ROWS || mcols < TUI_MIN_COLS) {
        tui_deinit();
        fprintf(stderr, "Error. The width or height of the terminal are too small.\n");
        return -1;
    }

    (*width_out) = mcols - 2;
    (*height_out) = mrows - 2;
    width = (*width_out);
    height = (*height_out);

    tui.last_event = TUI_NO_EVENT;
    tui.width = width;
    tui.height = height;

    tui.x = (mcols - width) / 2;
    tui.y = (mrows - height) / 2;

    unsigned int x = tui.x - 1;
    unsigned int y = tui.y - 1;

    clear();
    // Make the frame
    for (size_t i = x + 1; i < width + x; i++) {
        mvaddch(y, i, '-');
        mvaddch(y + height, i, '-');
    }
    for (size_t i = y + 1; i < height + y; i++) {
        mvaddch(i, x, '|');
        mvaddch(i, x + width, '|');
    }
    mvaddch(y, x, '+');
    mvaddch(y + height, x, '+');
    mvaddch(y, x + width, '+');
    mvaddch(y + height, x + width, '+');

    // Make
    const unsigned int title_x = 5;
    mvprintw(tui.y - 1, title_x, "[ Smooth Life - %s ]",
        title_info->ex_policy);

    return 0;
}


void tui_deinit(void) {
    curs_set(1);
    endwin();
}


void tui_render(float* frame) {
    // Check keyboard
    int input = getch();
    switch (input) {
        case 'r': tui.last_event = TUI_RESTART_EVENT; break;
        case 'q': tui.last_event = TUI_QUIT_EVENT; break;
        default: break;
    }
    // Render frame
    for (size_t i = tui.y; i < tui.height + tui.y - 1; i++) {
        for (size_t j = tui.x; j < tui.width + tui.x - 1; j++) {
            unsigned int level = (unsigned int)((frame[j + tui.width*i] * N_LEVELS));
            char c = LEVELS[level < N_LEVELS ? level : (N_LEVELS - (unsigned int)1)];
            mvaddch(i, j, c);
        }
    }
    refresh();
}

TUIEvent tui_get_event(void) {
    TUIEvent e = tui.last_event;
    tui.last_event = TUI_NO_EVENT;
    return e;
}