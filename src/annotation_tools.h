#pragma once
#include "annotation_layer.h"
#include "image_viewport.h"
#include <string>

namespace peek {

enum class ToolType {
    None,
    Pencil,
    Box,
    Arrow,
    Text,
    Eraser,
    Crop
};

struct AnnotationTools {
    ToolType active_tool = ToolType::None;
    ImU32 active_color = IM_COL32(255, 0, 0, 255);
    float thickness = 3.0f;
    float font_size = 24.0f;

    // State for in-progress drawing
    bool drawing = false;
    Annotation current; // in-progress annotation

    // Text input state
    bool text_input_active = false;
    char text_buffer[256] = {};
    ImVec2 text_position = {0, 0};

    void handle_input(AnnotationLayer& layer, const ImageViewport& vp);
    void draw_preview(ImDrawList* dl, const ImageViewport& vp) const;
    void cancel();
};

} // namespace peek
