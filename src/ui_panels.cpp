#include "ui_panels.h"
#include "utils.h"
#include "imgui.h"

namespace peek {

static const ImU32 TOOL_COLORS[] = {
    IM_COL32(255, 0, 0, 255),     // Red
    IM_COL32(0, 200, 0, 255),     // Green
    IM_COL32(0, 100, 255, 255),   // Blue
    IM_COL32(255, 255, 0, 255),   // Yellow
    IM_COL32(255, 0, 255, 255),   // Magenta
    IM_COL32(0, 0, 0, 255),       // Black
    IM_COL32(255, 255, 255, 255), // White
};
static const int NUM_COLORS = 7;

static bool pill_button(const char* label, bool active, ImVec2 size = ImVec2(0, 0)) {
    ImGuiStyle& style = ImGui::GetStyle();
    if (active) {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.784f, 0.902f, 0.196f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.85f, 0.95f, 0.30f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.70f, 0.82f, 0.15f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
    }
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 12.0f);
    bool clicked = ImGui::Button(label, size);
    ImGui::PopStyleVar();
    if (active) ImGui::PopStyleColor(4);
    return clicked;
}

void UIPanels::draw_toolbar(AnnotationTools& tools, AnnotationLayer& layer) {
    home_requested = false;
    crop_execute_requested = false;

    ImGuiIO& io = ImGui::GetIO();
    float panel_width = 200.0f;

    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x - panel_width - 12, 12), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(panel_width, 0), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowBgAlpha(0.9f);

    ImGui::Begin("Tools", &show_toolbar,
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize);

    // Home button
    if (pill_button("Home (Ctrl-F)", false, ImVec2(-1, 0))) {
        home_requested = true;
    }

    ImGui::Spacing();

    // Tool buttons
    ImGui::Text("Annotation Tools");
    ImGui::Separator();

    struct ToolDef { const char* name; ToolType type; };
    ToolDef tool_defs[] = {
        {"Pencil", ToolType::Pencil},
        {"Box", ToolType::Box},
        {"Arrow", ToolType::Arrow},
        {"Text", ToolType::Text},
        {"Eraser", ToolType::Eraser},
    };

    float btn_w = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x) * 0.5f;
    for (int i = 0; i < 5; i++) {
        if (i % 2 != 0) ImGui::SameLine();
        if (pill_button(tool_defs[i].name, tools.active_tool == tool_defs[i].type, ImVec2(btn_w, 0))) {
            if (tools.active_tool == tool_defs[i].type)
                tools.active_tool = ToolType::None;
            else
                tools.active_tool = tool_defs[i].type;
            tools.cancel();
        }
    }

    // Crop button on its own row with special behavior
    ImGui::SameLine();
    {
        bool has_crop_region = (layer.find_crop() >= 0);
        bool crop_active = (tools.active_tool == ToolType::Crop);
        if (pill_button("Crop", crop_active, ImVec2(btn_w, 0))) {
            if (crop_active && has_crop_region) {
                // Execute the crop
                crop_execute_requested = true;
                tools.active_tool = ToolType::None;
                tools.cancel();
            } else if (crop_active) {
                // Deactivate crop tool (no region drawn yet)
                tools.active_tool = ToolType::None;
                tools.cancel();
            } else {
                // Activate crop tool
                tools.active_tool = ToolType::Crop;
                tools.cancel();
            }
        }
    }

    ImGui::Spacing();

    // Color strip
    ImGui::Text("Color");
    ImGui::Separator();
    for (int i = 0; i < NUM_COLORS; i++) {
        if (i > 0) ImGui::SameLine();
        ImVec4 col = ImGui::ColorConvertU32ToFloat4(TOOL_COLORS[i]);
        bool selected = (tools.active_color == TOOL_COLORS[i]);

        if (selected) {
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 2.0f);
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.784f, 0.902f, 0.196f, 1.0f));
        }

        ImGui::PushID(i);
        if (ImGui::ColorButton("##color", col, ImGuiColorEditFlags_NoTooltip, ImVec2(20, 20))) {
            tools.active_color = TOOL_COLORS[i];
        }
        ImGui::PopID();

        if (selected) {
            ImGui::PopStyleColor();
            ImGui::PopStyleVar();
        }
    }

    ImGui::Spacing();

    // Thickness slider
    ImGui::Text("Thickness");
    ImGui::SetNextItemWidth(-1);
    ImGui::SliderFloat("##thickness", &tools.thickness, 1.0f, 10.0f, "%.0f px");

    ImGui::Spacing();

    // Clear all button
    if (pill_button("Clear All", false, ImVec2(-1, 0))) {
        layer.clear();
        tools.cancel();
    }

    ImGui::End();
}

void UIPanels::draw_file_controls(const LoadedImage& img, const std::string& image_path) {
    save_requested = false;

    ImGuiIO& io = ImGui::GetIO();
    float panel_width = 240.0f;

    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x - panel_width - 12, 400), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(panel_width, 0), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowBgAlpha(0.9f);

    ImGui::Begin("Save Options", &show_file_controls,
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize);

    // Format toggle
    ImGui::Text("Format");
    ImGui::Separator();
    float half_w = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x) * 0.5f;
    if (pill_button("JPG", !save_opts.save_as_png, ImVec2(half_w, 0))) save_opts.save_as_png = false;
    ImGui::SameLine();
    if (pill_button("PNG", save_opts.save_as_png, ImVec2(half_w, 0))) save_opts.save_as_png = true;

    ImGui::Spacing();

    // Scale buttons
    ImGui::Text("Scale");
    float third_w = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x * 2) / 3.0f;
    if (pill_button("1x", save_opts.scale == 1.0f, ImVec2(third_w, 0))) save_opts.scale = 1.0f;
    ImGui::SameLine();
    if (pill_button(".5x", save_opts.scale == 0.5f, ImVec2(third_w, 0))) save_opts.scale = 0.5f;
    ImGui::SameLine();
    if (pill_button(".25x", save_opts.scale == 0.25f, ImVec2(third_w, 0))) save_opts.scale = 0.25f;

    ImGui::Spacing();

    // Format-specific options
    if (save_opts.save_as_png) {
        bool transp = save_opts.png_transparency;
        if (ImGui::Checkbox("Transparency", &transp)) {
            save_opts.png_transparency = transp;
        }
    } else {
        ImGui::Text("Quality");
        ImGui::SetNextItemWidth(-1);
        ImGui::SliderInt("##quality", &save_opts.jpeg_quality, 40, 100, "%d%%");
    }

    ImGui::Spacing();

    // Append string
    ImGui::Text("Append to filename");
    static char append_buf[128] = "annotated";
    ImGui::SetNextItemWidth(-1);
    if (ImGui::InputText("##append", append_buf, sizeof(append_buf))) {
        save_opts.append_string = append_buf;
    }

    ImGui::Spacing();
    ImGui::Separator();

    // Save button
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.784f, 0.902f, 0.196f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.85f, 0.95f, 0.30f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.70f, 0.82f, 0.15f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 12.0f);

    if (ImGui::Button("Save", ImVec2(-1, 36))) {
        save_requested = true;
    }

    ImGui::PopStyleVar();
    ImGui::PopStyleColor(4);

    // Preview path and estimated size
    if (!image_path.empty() && img.valid) {
        std::string preview = SaveEngine::preview_path(image_path, save_opts);
        uint64_t est = SaveEngine::estimate_size(img, save_opts);

        // Show just the filename
        std::string filename = std::filesystem::path(preview).filename().string();
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
        ImGui::TextWrapped("%s (~%s)", filename.c_str(), format_file_size(est).c_str());
        ImGui::PopStyleColor();
    }

    ImGui::End();
}

void UIPanels::draw_text_popup(AnnotationTools& tools, AnnotationLayer& layer) {
    if (!tools.text_input_active) return;

    if (ImGui::BeginPopup("##TextInput")) {
        ImGui::Text("Enter text:");
        bool enter = ImGui::InputText("##textfield", tools.text_buffer, sizeof(tools.text_buffer),
                                       ImGuiInputTextFlags_EnterReturnsTrue);
        if (enter || ImGui::Button("OK")) {
            if (tools.text_buffer[0] != '\0') {
                Annotation ann;
                ann.type = AnnotationType::Text;
                ann.color = tools.active_color;
                ann.position = tools.text_position;
                ann.text = tools.text_buffer;
                ann.font_size = tools.font_size;
                layer.annotations.push_back(ann);
            }
            tools.text_input_active = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
            tools.text_input_active = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    } else {
        // Popup was closed externally
        tools.text_input_active = false;
    }
}

} // namespace peek
