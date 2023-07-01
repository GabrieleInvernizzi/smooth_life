#include <stdlib.h>
#include <time.h>
#include <stdio.h>

#include "config.h"
#include "smooth_life.h"
#include "tui.h"

// TEMP
#define WIDTH (173)
#define HEIGHT (41)

#define CLOCK_IN_SECS() ((double)clock() / (double)CLOCKS_PER_SEC)


const char* SM_EX_POLICY_STR[] = {
    "Singlethreaded",
    "Thread Pool",
    "OpenMP"
};


int main(const int argc, const char** argv) {
    // Set random seed.
    srand(time(0));

    Config conf;
    if (gen_config(&conf, argc, argv) == -1)
        return -1;

    // Setup tui info struct
    TUITitleInfo tui_title_info;
    tui_title_info.ex_policy = SM_EX_POLICY_STR[conf.sm_conf.ex_policy];

    if (tui_init(&conf.sm_conf.width, &conf.sm_conf.height, &tui_title_info))
        return 1;

    SMState* s = sm_init(&conf.sm_conf);
    if (!s) {
        tui_deinit();
        return 1;
    }


    // Setup timers
    double current_time = CLOCK_IN_SECS();
    double deltatime = 0;

    int is_running = 1;
    while(is_running) {
        TUIEvent e = tui_get_event();
        if (e == TUI_QUIT_EVENT) is_running = 0;
        else if (e == TUI_RESTART_EVENT) {
            sm_deinit(s);
            s = sm_init(&conf.sm_conf);
        }

        deltatime = CLOCK_IN_SECS() - current_time;

        if (deltatime > 1.0 / conf.max_fps) {
            current_time = CLOCK_IN_SECS();

            state_step(s);
            tui_render(sm_get_raw_state(s));
        }
    }

    tui_deinit();
    sm_deinit(s);
}