#include <stdlib.h>
#include <time.h>

#include "smooth_life.h"
#include "tui.h"


int main(const int argc, const char** argv) {
    srand(time(0));

    SMConfig conf = {
        .width = 180, .height = 46,
        .init_percent_x = 0.4, .init_percent_y = 0.4,
        .ri = -1.0f, .ra = 8.0f, 
        .b1 = .278f, .d1 = .267f, 
        .b2 = .365f, .d2 = .445f,
        .alpha_m = .147f, .alpha_n = .028f,
        .dt = .008f
    };

    SMState* s = sm_init(&conf);
    if (!s) return 1;

    if (tui_init(conf.width, conf.height)) {
        sm_deinit(s);
        return 1;
    }

    int is_running = 1;
    while(is_running) {
        TUIEvent e = tui_get_event();
        if (e == TUI_QUIT_EVENT) is_running = 0;
        else if (e == TUI_RESTART_EVENT) {
            sm_deinit(s);
            s = sm_init(&conf);
        }

        state_step(s);
        tui_render(sm_get_raw_state(s));
    }

    tui_deinit();
    sm_deinit(s);
}