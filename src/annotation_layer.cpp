#include "annotation_layer.h"
#include "image_viewport.h"
#include <algorithm>
#include <cmath>

namespace peek {

void AnnotationLayer::clear() {
    annotations.clear();
    active_crop = nullptr;
}

void AnnotationLayer::remove_at(int index) {
    if (index >= 0 && index < (int)annotations.size()) {
        annotations.erase(annotations.begin() + index);
        active_crop = nullptr;
    }
}

static void draw_arrowhead(ImDrawList* dl, ImVec2 from, ImVec2 to, ImU32 color, float size) {
    float dx = to.x - from.x;
    float dy = to.y - from.y;
    float len = sqrtf(dx * dx + dy * dy);
    if (len < 1.0f) return;

    dx /= len; dy /= len;
    float px = -dy, py = dx; // perpendicular

    ImVec2 tip = to;
    ImVec2 left = ImVec2(to.x - dx * size + px * size * 0.5f,
                          to.y - dy * size + py * size * 0.5f);
    ImVec2 right = ImVec2(to.x - dx * size - px * size * 0.5f,
                           to.y - dy * size - py * size * 0.5f);
    dl->AddTriangleFilled(tip, left, right, color);
}

void AnnotationLayer::draw(ImDrawList* dl, const ImageViewport& vp) const {
    for (auto& ann : annotations) {
        switch (ann.type) {
            case AnnotationType::Freehand: {
                if (ann.points.size() < 2) break;
                for (size_t i = 1; i < ann.points.size(); i++) {
                    ImVec2 a = vp.image_to_screen(ann.points[i - 1]);
                    ImVec2 b = vp.image_to_screen(ann.points[i]);
                    dl->AddLine(a, b, ann.color, ann.thickness * vp.zoom);
                }
                break;
            }
            case AnnotationType::Box: {
                ImVec2 a = vp.image_to_screen(ann.start);
                ImVec2 b = vp.image_to_screen(ann.end);
                dl->AddRect(a, b, ann.color, 0.0f, 0, ann.thickness * vp.zoom);
                break;
            }
            case AnnotationType::Arrow: {
                ImVec2 a = vp.image_to_screen(ann.start);
                ImVec2 b = vp.image_to_screen(ann.end);
                dl->AddLine(a, b, ann.color, ann.thickness * vp.zoom);
                float head_size = std::max(10.0f * vp.zoom, 6.0f);
                draw_arrowhead(dl, a, b, ann.color, head_size);
                break;
            }
            case AnnotationType::Text: {
                ImVec2 pos = vp.image_to_screen(ann.position);
                float scaled_size = ann.font_size * vp.zoom;
                if (scaled_size < 4.0f) break;
                dl->AddText(nullptr, scaled_size, pos, ann.color, ann.text.c_str());
                break;
            }
            case AnnotationType::Crop: {
                ImVec2 a = vp.image_to_screen(ann.start);
                ImVec2 b = vp.image_to_screen(ann.end);
                // Darken area outside crop
                ImVec2 screen_min(0, 0);
                ImVec2 screen_max(dl->GetClipRectMax());
                ImU32 dark = IM_COL32(0, 0, 0, 128);
                float left = std::min(a.x, b.x);
                float right = std::max(a.x, b.x);
                float top = std::min(a.y, b.y);
                float bottom = std::max(a.y, b.y);
                // Top strip
                dl->AddRectFilled(screen_min, ImVec2(screen_max.x, top), dark);
                // Bottom strip
                dl->AddRectFilled(ImVec2(screen_min.x, bottom), screen_max, dark);
                // Left strip
                dl->AddRectFilled(ImVec2(screen_min.x, top), ImVec2(left, bottom), dark);
                // Right strip
                dl->AddRectFilled(ImVec2(right, top), ImVec2(screen_max.x, bottom), dark);
                // Crop border
                dl->AddRect(ImVec2(left, top), ImVec2(right, bottom),
                            IM_COL32(200, 230, 50, 255), 0.0f, 0, 2.0f);
                break;
            }
        }
    }
}

int AnnotationLayer::hit_test(ImVec2 image_pos, float threshold) const {
    // Iterate in reverse so top-most annotations are hit first
    for (int i = (int)annotations.size() - 1; i >= 0; i--) {
        auto& ann = annotations[i];
        switch (ann.type) {
            case AnnotationType::Freehand: {
                for (auto& pt : ann.points) {
                    float dx = pt.x - image_pos.x;
                    float dy = pt.y - image_pos.y;
                    if (dx * dx + dy * dy < threshold * threshold) return i;
                }
                break;
            }
            case AnnotationType::Box: {
                float l = std::min(ann.start.x, ann.end.x);
                float r = std::max(ann.start.x, ann.end.x);
                float t = std::min(ann.start.y, ann.end.y);
                float b = std::max(ann.start.y, ann.end.y);
                // Check if near edges
                bool near_h = (image_pos.x >= l - threshold && image_pos.x <= r + threshold);
                bool near_v = (image_pos.y >= t - threshold && image_pos.y <= b + threshold);
                bool on_left = fabsf(image_pos.x - l) < threshold;
                bool on_right = fabsf(image_pos.x - r) < threshold;
                bool on_top = fabsf(image_pos.y - t) < threshold;
                bool on_bottom = fabsf(image_pos.y - b) < threshold;
                if ((on_left || on_right) && near_v) return i;
                if ((on_top || on_bottom) && near_h) return i;
                break;
            }
            case AnnotationType::Arrow: {
                // Distance from point to line segment
                float dx = ann.end.x - ann.start.x;
                float dy = ann.end.y - ann.start.y;
                float len2 = dx * dx + dy * dy;
                float t_val = 0;
                if (len2 > 0) {
                    t_val = ((image_pos.x - ann.start.x) * dx + (image_pos.y - ann.start.y) * dy) / len2;
                    t_val = std::clamp(t_val, 0.0f, 1.0f);
                }
                float px = ann.start.x + t_val * dx;
                float py = ann.start.y + t_val * dy;
                float dist = sqrtf((image_pos.x - px) * (image_pos.x - px) + (image_pos.y - py) * (image_pos.y - py));
                if (dist < threshold) return i;
                break;
            }
            case AnnotationType::Text: {
                float tw = ann.font_size * ann.text.size() * 0.5f; // rough estimate
                float th = ann.font_size;
                if (image_pos.x >= ann.position.x && image_pos.x <= ann.position.x + tw &&
                    image_pos.y >= ann.position.y && image_pos.y <= ann.position.y + th)
                    return i;
                break;
            }
            case AnnotationType::Crop:
                break; // Crop not erasable
        }
    }
    return -1;
}

int AnnotationLayer::find_crop() const {
    for (int i = 0; i < (int)annotations.size(); i++) {
        if (annotations[i].type == AnnotationType::Crop) return i;
    }
    return -1;
}

} // namespace peek
