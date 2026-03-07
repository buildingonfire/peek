#include "annotation_tools.h"

namespace peek {

void AnnotationTools::handle_input(AnnotationLayer& layer, const ImageViewport& vp) {
    ImGuiIO& io = ImGui::GetIO();

    if (active_tool == ToolType::None) return;
    if (io.WantCaptureMouse && !text_input_active) return;

    ImVec2 mouse = io.MousePos;
    ImVec2 img_pos = vp.screen_to_image(mouse);

    switch (active_tool) {
        case ToolType::Pencil: {
            if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                drawing = true;
                current = {};
                current.type = AnnotationType::Freehand;
                current.color = active_color;
                current.thickness = thickness;
                current.points.push_back(img_pos);
            }
            if (drawing && ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
                current.points.push_back(img_pos);
            }
            if (drawing && ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
                if (current.points.size() >= 2) {
                    layer.annotations.push_back(current);
                }
                drawing = false;
            }
            break;
        }
        case ToolType::Box: {
            if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                drawing = true;
                current = {};
                current.type = AnnotationType::Box;
                current.color = active_color;
                current.thickness = thickness;
                current.start = img_pos;
                current.end = img_pos;
            }
            if (drawing && ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
                current.end = img_pos;
            }
            if (drawing && ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
                layer.annotations.push_back(current);
                drawing = false;
            }
            break;
        }
        case ToolType::Arrow: {
            if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                drawing = true;
                current = {};
                current.type = AnnotationType::Arrow;
                current.color = active_color;
                current.thickness = thickness;
                current.start = img_pos;
                current.end = img_pos;
            }
            if (drawing && ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
                current.end = img_pos;
            }
            if (drawing && ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
                layer.annotations.push_back(current);
                drawing = false;
            }
            break;
        }
        case ToolType::Text: {
            if (!text_input_active && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                text_input_active = true;
                text_position = img_pos;
                text_buffer[0] = '\0';
                ImGui::OpenPopup("##TextInput");
            }
            break;
        }
        case ToolType::Eraser: {
            if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                float threshold = 10.0f / vp.zoom; // 10 screen pixels
                int idx = layer.hit_test(img_pos, threshold);
                if (idx >= 0 && layer.annotations[idx].type != AnnotationType::Crop) {
                    layer.remove_at(idx);
                }
            }
            break;
        }
        case ToolType::Crop: {
            if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                // Remove existing crop
                int crop_idx = layer.find_crop();
                if (crop_idx >= 0) layer.remove_at(crop_idx);

                drawing = true;
                current = {};
                current.type = AnnotationType::Crop;
                current.start = img_pos;
                current.end = img_pos;
            }
            if (drawing && ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
                current.end = img_pos;
            }
            if (drawing && ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
                layer.annotations.push_back(current);
                drawing = false;
            }
            break;
        }
        default: break;
    }
}

void AnnotationTools::draw_preview(ImDrawList* dl, const ImageViewport& vp) const {
    if (!drawing) return;

    switch (current.type) {
        case AnnotationType::Freehand: {
            for (size_t i = 1; i < current.points.size(); i++) {
                ImVec2 a = vp.image_to_screen(current.points[i - 1]);
                ImVec2 b = vp.image_to_screen(current.points[i]);
                dl->AddLine(a, b, current.color, current.thickness * vp.zoom);
            }
            break;
        }
        case AnnotationType::Box: {
            ImVec2 a = vp.image_to_screen(current.start);
            ImVec2 b = vp.image_to_screen(current.end);
            dl->AddRect(a, b, current.color, 0.0f, 0, current.thickness * vp.zoom);
            break;
        }
        case AnnotationType::Arrow: {
            ImVec2 a = vp.image_to_screen(current.start);
            ImVec2 b = vp.image_to_screen(current.end);
            dl->AddLine(a, b, current.color, current.thickness * vp.zoom);
            float head_size = std::max(10.0f * vp.zoom, 6.0f);
            // Arrowhead
            float dx = b.x - a.x, dy = b.y - a.y;
            float len = sqrtf(dx * dx + dy * dy);
            if (len > 1.0f) {
                dx /= len; dy /= len;
                float px = -dy, py = dx;
                ImVec2 left(b.x - dx * head_size + px * head_size * 0.5f,
                            b.y - dy * head_size + py * head_size * 0.5f);
                ImVec2 right(b.x - dx * head_size - px * head_size * 0.5f,
                             b.y - dy * head_size - py * head_size * 0.5f);
                dl->AddTriangleFilled(b, left, right, current.color);
            }
            break;
        }
        case AnnotationType::Crop: {
            ImVec2 a = vp.image_to_screen(current.start);
            ImVec2 b = vp.image_to_screen(current.end);
            dl->AddRect(a, b, IM_COL32(200, 230, 50, 255), 0.0f, 0, 2.0f);
            break;
        }
        default: break;
    }
}

void AnnotationTools::cancel() {
    drawing = false;
    text_input_active = false;
}

} // namespace peek
