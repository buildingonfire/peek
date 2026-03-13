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

static void inline_label(const char* text) {
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
    ImGui::AlignTextToFramePadding();
    ImGui::TextUnformatted(text);
    ImGui::PopStyleColor();
    ImGui::SameLine();
}

void UIPanels::draw_top_bar(AnnotationTools& tools, AnnotationLayer& layer,
                            const LoadedImage& img, const std::string& image_path) {
    home_requested = false;
    crop_execute_requested = false;
    save_requested = false;

    ImGuiIO& io = ImGui::GetIO();
    float row_h = ImGui::GetFrameHeight();
    float row_spacing = 4.0f;
    float section_gap = 6.0f;  // extra space around divider
    float pad_y = 6.0f;
    // 4 rows: save row1, save row2, divider gap, edit row1, edit row2
    float total_h = pad_y * 2 + row_h * 4 + row_spacing * 3 + section_gap * 2 + 1;

    // Fixed-width inner area so controls never scrunch
    float inner_w = 720.0f;
    float win_w = io.DisplaySize.x;
    // Center the controls if window is wider, pin left if narrower
    float offset_x = (win_w > inner_w + 20) ? (win_w - inner_w) * 0.5f : 10.0f;

    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(win_w, total_h));
    ImGui::SetNextWindowBgAlpha(0.92f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10, pad_y));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(6, row_spacing));

    ImGui::Begin("##topbar", nullptr,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoNav);

    bar_height = total_h;

    // Indent to offset_x
    float base_x = offset_x;

    // ========== SAVE GROUP (top two rows) ==========

    // --- Save Row 1: Format + Scale + format-specific option ---
    ImGui::SetCursorPosX(base_x);
    inline_label("Format");
    if (pill_button("JPG", !save_opts.save_as_png)) save_opts.save_as_png = false;
    ImGui::SameLine();
    if (pill_button("PNG", save_opts.save_as_png)) save_opts.save_as_png = true;

    ImGui::SameLine(0, 14);

    inline_label("Scale");
    if (pill_button("1x", save_opts.scale == 1.0f)) save_opts.scale = 1.0f;
    ImGui::SameLine();
    if (pill_button(".5x", save_opts.scale == 0.5f)) save_opts.scale = 0.5f;
    ImGui::SameLine();
    if (pill_button(".25x", save_opts.scale == 0.25f)) save_opts.scale = 0.25f;

    ImGui::SameLine(0, 14);

    if (save_opts.save_as_png) {
        bool transp = save_opts.png_transparency;
        if (ImGui::Checkbox("Transp.", &transp)) {
            save_opts.png_transparency = transp;
        }
    } else {
        inline_label("Qual.");
        ImGui::SetNextItemWidth(60);
        ImGui::SliderInt("##quality", &save_opts.jpeg_quality, 40, 100, "%d%%");
    }

    // --- Save Row 2: Suffix + Save + preview ---
    ImGui::SetCursorPosX(base_x);
    inline_label("Suffix");
    static char append_buf[128] = "annotated";
    ImGui::SetNextItemWidth(90);
    if (ImGui::InputText("##append", append_buf, sizeof(append_buf))) {
        save_opts.append_string = append_buf;
    }

    ImGui::SameLine(0, 10);

    // Save button
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.784f, 0.902f, 0.196f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.85f, 0.95f, 0.30f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.70f, 0.82f, 0.15f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 12.0f);

    if (ImGui::Button("Save", ImVec2(56, 0))) {
        save_requested = true;
    }

    ImGui::PopStyleVar();
    ImGui::PopStyleColor(4);

    if (!image_path.empty() && img.valid) {
        ImGui::SameLine();
        std::string preview = SaveEngine::preview_path(image_path, save_opts);
        uint64_t est = SaveEngine::estimate_size(img, save_opts);
        std::string filename = std::filesystem::path(preview).filename().string();
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
        ImGui::AlignTextToFramePadding();
        ImGui::Text("%s (~%s)", filename.c_str(), format_file_size(est).c_str());
        ImGui::PopStyleColor();
    }

    // ========== HORIZONTAL DIVIDER ==========
    ImGui::Dummy(ImVec2(0, section_gap * 0.5f));
    {
        ImVec2 p = ImGui::GetCursorScreenPos();
        ImGui::GetWindowDrawList()->AddLine(
            ImVec2(p.x + base_x, p.y),
            ImVec2(p.x + base_x + inner_w - 20, p.y),
            IM_COL32(80, 80, 80, 200), 1.0f);
    }
    ImGui::Dummy(ImVec2(0, section_gap * 0.5f));

    // ========== EDIT GROUP (bottom two rows) ==========

    // --- Edit Row 1: Home + tool buttons ---
    ImGui::SetCursorPosX(base_x);
    if (pill_button("Home", false)) {
        home_requested = true;
    }
    ImGui::SameLine();

    struct ToolDef { const char* name; ToolType type; };
    ToolDef tool_defs[] = {
        {"Pencil", ToolType::Pencil},
        {"Box", ToolType::Box},
        {"Arrow", ToolType::Arrow},
        {"Text", ToolType::Text},
        {"Eraser", ToolType::Eraser},
    };

    for (int i = 0; i < 5; i++) {
        if (pill_button(tool_defs[i].name, tools.active_tool == tool_defs[i].type)) {
            if (tools.active_tool == tool_defs[i].type)
                tools.active_tool = ToolType::None;
            else
                tools.active_tool = tool_defs[i].type;
            tools.cancel();
        }
        ImGui::SameLine();
    }

    {
        bool has_crop_region = (layer.find_crop() >= 0);
        bool crop_active = (tools.active_tool == ToolType::Crop);
        if (pill_button("Crop", crop_active)) {
            if (crop_active && has_crop_region) {
                crop_execute_requested = true;
                tools.active_tool = ToolType::None;
                tools.cancel();
            } else if (crop_active) {
                tools.active_tool = ToolType::None;
                tools.cancel();
            } else {
                tools.active_tool = ToolType::Crop;
                tools.cancel();
            }
        }
    }

    // --- Edit Row 2: Color swatches + thickness + clear ---
    ImGui::SetCursorPosX(base_x);
    inline_label("Color");
    for (int i = 0; i < NUM_COLORS; i++) {
        if (i > 0) ImGui::SameLine(0, 2);
        ImVec4 col = ImGui::ColorConvertU32ToFloat4(TOOL_COLORS[i]);
        bool selected = (tools.active_color == TOOL_COLORS[i]);

        if (selected) {
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 2.0f);
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.784f, 0.902f, 0.196f, 1.0f));
        }

        ImGui::PushID(i);
        if (ImGui::ColorButton("##color", col, ImGuiColorEditFlags_NoTooltip, ImVec2(18, 18))) {
            tools.active_color = TOOL_COLORS[i];
        }
        ImGui::PopID();

        if (selected) {
            ImGui::PopStyleColor();
            ImGui::PopStyleVar();
        }
    }

    ImGui::SameLine();

    inline_label("Size");
    ImGui::SetNextItemWidth(70);
    ImGui::SliderFloat("##thickness", &tools.thickness, 1.0f, 10.0f, "%.0f px");

    ImGui::SameLine();

    if (pill_button("Clear All", false)) {
        layer.clear();
        tools.cancel();
    }

    ImGui::End();
    ImGui::PopStyleVar(2);
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
        tools.text_input_active = false;
    }
}

} // namespace peek
