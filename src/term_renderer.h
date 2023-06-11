#pragma once

// Width and height of each frame.
void trenderer_init(unsigned int width, unsigned int height);
// Renders a frame onto the terminal. The frame must have the dimensions specified in trenderer_init.
void trenderer_render(const float** frame);