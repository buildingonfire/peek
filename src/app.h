#pragma once
#include "renderer.h"
#include "image_loader.h"
#include "image_viewport.h"
#include "annotation_layer.h"
#include "annotation_tools.h"
#include "save_engine.h"
#include "ui_panels.h"
#include "folder_browser.h"
#include <string>

namespace peek {

struct App {
    Renderer renderer;
    LoadedImage image;
    ImageViewport viewport;
    AnnotationLayer annotations;
    AnnotationTools tools;
    UIPanels panels;
    FolderBrowser browser;

    std::string current_path;
    bool first_run_checked = false;

    bool init(HWND hwnd, const std::string& initial_path);
    void shutdown();
    void frame();
    void load_image(const std::string& path);
    void on_resize(int w, int h);
    void on_drop(const std::string& path);
    void on_key(int vk);
    void save();
    void reset_view();
    void apply_crop();
    void update_title();
};

} // namespace peek
