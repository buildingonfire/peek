#include "app.h"
#include "file_assoc.h"
#include "utils.h"
#include "theme.h"
#include "imgui.h"
#include <windows.h>
#include <filesystem>
#include <algorithm>
#include <cstring>

namespace peek {

bool App::init(HWND hwnd, const std::string& initial_path) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.IniFilename = nullptr;

    load_font();
    apply_theme();

    if (!renderer.init(hwnd)) return false;

    RECT rc;
    GetClientRect(hwnd, &rc);
    renderer.width = rc.right - rc.left;
    renderer.height = rc.bottom - rc.top;

    if (!initial_path.empty()) {
        load_image(initial_path);
    }

    return true;
}

void App::shutdown() {
    image.release_gpu();
    renderer.shutdown();
    ImGui::DestroyContext();
}

void App::load_image(const std::string& path) {
    if (path.empty()) return;

    std::string ext = get_file_extension_lower(path);
    if (!is_supported_image(ext)) return;

    if (load_image_from_file(path, image)) {
        upload_to_gpu(renderer.device, image);
        current_path = path;
        viewport.fit_horizontal(image.width, image.height, renderer.width, renderer.height);
        annotations.clear();
        tools.cancel();
        browser.scan(path);
        update_title();
    }
}

void App::on_resize(int w, int h) {
    renderer.resize(w, h);
}

void App::on_drop(const std::string& path) {
    load_image(path);
}

void App::on_key(int vk) {
    bool ctrl = (GetKeyState(VK_CONTROL) & 0x8000) != 0;

    if (vk == VK_LEFT) {
        std::string prev = browser.prev();
        if (!prev.empty()) load_image(prev);
    } else if (vk == VK_RIGHT) {
        std::string next = browser.next();
        if (!next.empty()) load_image(next);
    } else if (ctrl && vk == 'S') {
        save();
    } else if (ctrl && vk == 'F') {
        reset_view();
    }
}

void App::reset_view() {
    if (image.valid) {
        viewport.fit_horizontal(image.width, image.height, renderer.width, renderer.height);
    }
}

void App::apply_crop() {
    if (!image.valid) return;
    int crop_idx = annotations.find_crop();
    if (crop_idx < 0) return;

    auto& crop = annotations.annotations[crop_idx];
    int cx = std::max(0, (int)std::min(crop.start.x, crop.end.x));
    int cy = std::max(0, (int)std::min(crop.start.y, crop.end.y));
    int cr = std::min(image.width, (int)std::max(crop.start.x, crop.end.x));
    int cb = std::min(image.height, (int)std::max(crop.start.y, crop.end.y));
    int cw = cr - cx;
    int ch = cb - cy;
    if (cw <= 0 || ch <= 0) return;

    // Crop the pixel buffer
    std::vector<uint8_t> cropped((size_t)cw * ch * 4);
    for (int y = 0; y < ch; y++) {
        memcpy(&cropped[y * cw * 4],
               &image.pixels[((cy + y) * image.width + cx) * 4],
               cw * 4);
    }
    image.pixels = std::move(cropped);

    // Offset all non-crop annotations by the crop origin
    for (auto& ann : annotations.annotations) {
        if (ann.type == AnnotationType::Crop) continue;
        for (auto& pt : ann.points) {
            pt.x -= cx; pt.y -= cy;
        }
        ann.start.x -= cx; ann.start.y -= cy;
        ann.end.x -= cx; ann.end.y -= cy;
        ann.position.x -= cx; ann.position.y -= cy;
    }

    // Remove the crop annotation
    annotations.remove_at(crop_idx);

    // Update image dimensions and re-upload
    image.width = cw;
    image.height = ch;
    upload_to_gpu(renderer.device, image);
    viewport.fit_horizontal(image.width, image.height, renderer.width, renderer.height);
    update_title();
}

void App::save() {
    if (!image.valid || current_path.empty()) return;

    std::string out_path;
    if (SaveEngine::save(image, annotations, panels.save_opts, current_path, out_path)) {
        // Brief flash or notification could go here
        std::string msg = "Saved: " + std::filesystem::path(out_path).filename().string();
        SetWindowTextA(renderer.hwnd, msg.c_str());
        // Restore title after a moment (next frame will update it anyway)
    }
}

void App::update_title() {
    if (current_path.empty()) {
        SetWindowTextW(renderer.hwnd, L"Peek");
        return;
    }
    std::string title = std::filesystem::path(current_path).filename().string() + " - Peek";
    if (image.valid) {
        title += " (" + std::to_string(image.width) + "x" + std::to_string(image.height) + ")";
    }
    SetWindowTextA(renderer.hwnd, title.c_str());
}

void App::frame() {
    // First-run check
    if (!first_run_checked) {
        first_run_checked = true;
        // Defer to avoid blocking startup
        // show_first_run_dialog() is called from main after first frame
    }

    renderer.begin_frame();

    // Handle viewport input (pan/zoom)
    viewport.handle_input(image.width, image.height);

    // Draw background and image
    ImDrawList* bg_dl = ImGui::GetBackgroundDrawList();
    if (image.valid && image.srv) {
        viewport.draw_checkerboard(renderer.width, renderer.height);
        viewport.draw_image((ImTextureID)image.srv, image.width, image.height,
                            renderer.width, renderer.height);
    }

    // Draw annotations
    ImDrawList* fg_dl = ImGui::GetForegroundDrawList();
    annotations.draw(fg_dl, viewport);

    // Handle annotation tool input
    tools.handle_input(annotations, viewport);
    tools.draw_preview(fg_dl, viewport);

    // Draw UI panels
    panels.draw_top_bar(tools, annotations, image, current_path);
    panels.draw_text_popup(tools, annotations);

    // Handle UI requests
    if (panels.save_requested) {
        save();
    }
    if (panels.home_requested) {
        reset_view();
    }
    if (panels.crop_execute_requested) {
        apply_crop();
    }

    // Status bar at bottom
    {
        ImGuiIO& io = ImGui::GetIO();
        float bar_h = 24.0f;
        ImGui::SetNextWindowPos(ImVec2(0, io.DisplaySize.y - bar_h));
        ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x, bar_h));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 4));
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.11f, 0.11f, 0.11f, 1.0f));
        ImGui::Begin("##statusbar", nullptr,
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoNav);

        if (image.valid) {
            ImGui::Text("%dx%d | %.0f%%", image.width, image.height, viewport.zoom * 100.0f);
            if (!current_path.empty()) {
                ImGui::SameLine(0, 20);
                std::string filename = std::filesystem::path(current_path).filename().string();
                ImGui::TextDisabled("%s", filename.c_str());
            }
            if (browser.files.size() > 1) {
                ImGui::SameLine(0, 20);
                ImGui::TextDisabled("[%d/%d]", browser.current_index + 1, (int)browser.files.size());
            }
        } else {
            ImGui::TextDisabled("Drop an image or open via command line");
        }

        ImGui::End();
        ImGui::PopStyleColor();
        ImGui::PopStyleVar();
    }

    renderer.end_frame();
}

} // namespace peek
