#include <stdlib.h>
#include <time.h>

#include "smooth_life.h"
#include "term_renderer.h"


int main(const int argc, const char** argv) {
    srand(time(0));

    SMConfig conf = {
        .width = 180, .height = 46,
        .init_percent_x = 0.4, .init_percent_y = 0.4,
        .ri = 11.0f/3.0f, .ra = 11.0f, 
        .b1 = .278f, .d1 = .267f, 
        .b2 = .365f, .d2 = .445f,
        .alpha_m = .147f, .alpha_n = .028f,
        .dt = .005f
    };

    SMState* s = sm_init(&conf);
    if (!s) return 1;

    if (trenderer_init(conf.width, conf.height)) {
        sm_deinit(s);
        return 1;
    }

    while(1) {
        state_step(s);
        trenderer_render(sm_get_raw_state(s));
    }

    trenderer_deinit();
    sm_deinit(s);
}