#pragma once

// Width and height of each frame.
void trenderer_init(unsigned int width, unsigned int height);
// Deinit trenderer.
void trenderer_deinit(void);
// Renders a frame onto the terminal.
// The frame must have the dimensions specified in trenderer_init and the valuse must be between 0.0 and 1.0.
void trenderer_render(float* frame);