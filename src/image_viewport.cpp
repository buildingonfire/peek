#include "image_viewport.h"
#include <algorithm>
#include <cmath>

namespace peek {

void ImageViewport::fit_to_window(int img_w, int img_h, int win_w, int win_h) {
    if (img_w <= 0 || img_h <= 0 || win_w <= 0 || win_h <= 0) return;

    float scale_x = (float)win_w / (float)img_w;
    float scale_y = (float)win_h / (float)img_h;
    zoom = std::min(scale_x, scale_y);
    if (zoom > 1.0f) zoom = 1.0f; // don't upscale beyond 1:1

    float disp_w = img_w * zoom;
    float disp_h = img_h * zoom;
    offset.x = (win_w - disp_w) * 0.5f;
    offset.y = (win_h - disp_h) * 0.5f;
}

void ImageViewport::fit_horizontal(int img_w, int img_h, int win_w, int win_h) {
    if (img_w <= 0 || img_h <= 0 || win_w <= 0 || win_h <= 0) return;

    zoom = (float)win_w / (float)img_w;
    if (zoom > 1.0f) zoom = 1.0f;

    float disp_w = img_w * zoom;
    float disp_h = img_h * zoom;
    offset.x = (win_w - disp_w) * 0.5f;
    offset.y = (win_h - disp_h) * 0.5f;
}

void ImageViewport::handle_input(int img_w, int img_h) {
    ImGuiIO& io = ImGui::GetIO();

    // Don't handle input if ImGui wants the mouse (e.g., over a panel)
    if (io.WantCaptureMouse) return;

    // Middle-wheel zoom toward cursor
    if (io.MouseWheel != 0.0f) {
        float old_zoom = zoom;
        float factor = io.MouseWheel > 0 ? 1.15f : 1.0f / 1.15f;
        zoom *= factor;
        zoom = std::clamp(zoom, 0.01f, 100.0f);

        // Zoom toward cursor position
        ImVec2 mouse = io.MousePos;
        offset.x = mouse.x - (mouse.x - offset.x) * (zoom / old_zoom);
        offset.y = mouse.y - (mouse.y - offset.y) * (zoom / old_zoom);
    }

    // Middle-button drag to pan
    if (ImGui::IsMouseDown(ImGuiMouseButton_Middle)) {
        if (!dragging) {
            dragging = true;
            drag_start = io.MousePos;
        }
        offset.x += io.MouseDelta.x;
        offset.y += io.MouseDelta.y;
    } else {
        dragging = false;
    }
}

ImVec2 ImageViewport::image_to_screen(ImVec2 img_pos) const {
    return ImVec2(img_pos.x * zoom + offset.x, img_pos.y * zoom + offset.y);
}

ImVec2 ImageViewport::screen_to_image(ImVec2 scr_pos) const {
    return ImVec2((scr_pos.x - offset.x) / zoom, (scr_pos.y - offset.y) / zoom);
}

void ImageViewport::draw_checkerboard(int win_w, int win_h) {
    ImDrawList* dl = ImGui::GetBackgroundDrawList();
    const float checker_size = 16.0f;
    ImU32 c1 = IM_COL32(40, 40, 40, 255);
    ImU32 c2 = IM_COL32(50, 50, 50, 255);

    for (float y = 0; y < win_h; y += checker_size) {
        for (float x = 0; x < win_w; x += checker_size) {
            int ix = (int)(x / checker_size);
            int iy = (int)(y / checker_size);
            ImU32 col = ((ix + iy) % 2 == 0) ? c1 : c2;
            dl->AddRectFilled(ImVec2(x, y), ImVec2(x + checker_size, y + checker_size), col);
        }
    }
}

void ImageViewport::draw_image(ImTextureID tex, int img_w, int img_h, int win_w, int win_h) {
    if (!tex) return;

    ImDrawList* dl = ImGui::GetBackgroundDrawList();
    ImVec2 p0 = image_to_screen(ImVec2(0, 0));
    ImVec2 p1 = image_to_screen(ImVec2((float)img_w, (float)img_h));
    dl->AddImage(tex, p0, p1);
}

} // namespace peek
