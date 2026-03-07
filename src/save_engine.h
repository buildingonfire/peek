#pragma once
#include "image_loader.h"
#include "annotation_layer.h"
#include <string>

namespace peek {

struct SaveOptions {
    bool save_as_png = false;      // false = JPG, true = PNG
    float scale = 1.0f;            // 1.0, 0.5, 0.25
    int jpeg_quality = 95;         // 40-100
    bool png_transparency = false;
    std::string append_string = "annotated";
};

struct SaveEngine {
    static bool save(const LoadedImage& img, const AnnotationLayer& layer,
                     const SaveOptions& opts, const std::string& original_path,
                     std::string& out_path);

    static uint64_t estimate_size(const LoadedImage& img, const SaveOptions& opts);

    static std::string preview_path(const std::string& original_path, const SaveOptions& opts);
};

} // namespace peek
