#pragma once
#include "imgui.h"
#include <vector>
#include <string>

namespace peek {

enum class AnnotationType {
    Freehand,
    Box,
    Arrow,
    Text,
    Crop
};

struct Annotation {
    AnnotationType type;
    ImU32 color = IM_COL32(255, 0, 0, 255);
    float thickness = 3.0f;

    // Freehand
    std::vector<ImVec2> points; // image-space

    // Box, Arrow, Crop
    ImVec2 start = {0, 0}; // image-space
    ImVec2 end = {0, 0};   // image-space

    // Text
    std::string text;
    ImVec2 position = {0, 0}; // image-space
    float font_size = 24.0f;
};

struct AnnotationLayer {
    std::vector<Annotation> annotations;
    Annotation* active_crop = nullptr; // pointer to the current crop annotation, if any

    void clear();
    void remove_at(int index);
    void draw(ImDrawList* dl, const struct ImageViewport& vp) const;
    int hit_test(ImVec2 image_pos, float threshold) const;

    // Find crop annotation (returns -1 if none)
    int find_crop() const;
};

} // namespace peek
