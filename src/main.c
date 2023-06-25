#include <stdlib.h>
#include <time.h>
#include <stdio.h>

#include "smooth_life.h"
#include "tui.h"

#define CLOCK_IN_SECS() ((double)clock() / (double)CLOCKS_PER_SEC)


int main(const int argc, const char** argv) {
    // Set random seed.
    srand(time(0));
    const unsigned int max_fps = 30;

    SMConfig conf = {
        .ex_policy = SM_OMP,
        .n_threads = 0,
        .width = 180, .height = 46,
        .init_percent_x = 0.4, .init_percent_y = 0.4,
        .ri = -1.0f, .ra = 7.0f, 
        .b1 = .278f, .d1 = .267f, 
        .b2 = .365f, .d2 = .445f,
        .alpha_m = .147f, .alpha_n = .028f,
        .dt = .05f
    };

    SMState* s = sm_init(&conf);
    if (!s) return 1;

    if (tui_init(conf.width, conf.height)) {
        sm_deinit(s);
        return 1;
    }

    // Setup timers
    double current_time = CLOCK_IN_SECS();
    double deltatime = 0;
    double fps = 0.0;

    int is_running = 1;
    while(is_running) {
        TUIEvent e = tui_get_event();
        if (e == TUI_QUIT_EVENT) is_running = 0;
        else if (e == TUI_RESTART_EVENT) {
            sm_deinit(s);
            s = sm_init(&conf);
        }

        deltatime = CLOCK_IN_SECS() - current_time;

        if (deltatime > 1.0 / max_fps) {
            current_time = CLOCK_IN_SECS();
            fps = 1.0 / deltatime;
            
            state_step(s);
            tui_render(sm_get_raw_state(s));
        }
    }

    tui_deinit();
    sm_deinit(s);
}