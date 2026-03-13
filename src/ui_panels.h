#pragma once
#include "annotation_tools.h"
#include "annotation_layer.h"
#include "save_engine.h"
#include "image_loader.h"
#include <string>

namespace peek {

struct UIPanels {
    SaveOptions save_opts;
    float bar_height = 0.0f;

    void draw_top_bar(AnnotationTools& tools, AnnotationLayer& layer,
                      const LoadedImage& img, const std::string& image_path);
    void draw_text_popup(AnnotationTools& tools, AnnotationLayer& layer);

    bool save_requested = false;
    bool home_requested = false;
    bool crop_execute_requested = false;
};

} // namespace peek
