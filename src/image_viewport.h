#pragma once
#include "imgui.h"

namespace peek {

struct ImageViewport {
    float zoom = 1.0f;
    ImVec2 offset = {0, 0}; // screen-space offset of image origin
    bool dragging = false;
    ImVec2 drag_start = {0, 0};

    void fit_to_window(int img_w, int img_h, int win_w, int win_h);
    void fit_horizontal(int img_w, int img_h, int win_w, int win_h);
    void handle_input(int img_w, int img_h);
    ImVec2 image_to_screen(ImVec2 img_pos) const;
    ImVec2 screen_to_image(ImVec2 scr_pos) const;
    void draw_image(ImTextureID tex, int img_w, int img_h, int win_w, int win_h);
    void draw_checkerboard(int win_w, int win_h);
};

} // namespace peek
