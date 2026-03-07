#pragma once
#include "annotation_tools.h"
#include "annotation_layer.h"
#include "save_engine.h"
#include "image_loader.h"
#include <string>

namespace peek {

struct UIPanels {
    SaveOptions save_opts;
    bool show_toolbar = true;
    bool show_file_controls = true;

    void draw_toolbar(AnnotationTools& tools, AnnotationLayer& layer);
    void draw_file_controls(const LoadedImage& img, const std::string& image_path);
    void draw_text_popup(AnnotationTools& tools, AnnotationLayer& layer);

    bool save_requested = false;
    bool home_requested = false;
    bool crop_execute_requested = false;
};

} // namespace peek
