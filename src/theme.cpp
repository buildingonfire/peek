#include "theme.h"
#include "inter_font.h"

namespace peek {

ImFont* load_font() {
    ImGuiIO& io = ImGui::GetIO();
    ImFontConfig config;
    config.FontDataOwnedByAtlas = false;
    ImFont* font = io.Fonts->AddFontFromMemoryTTF(
        (void*)inter_regular_ttf_data,
        (int)inter_regular_ttf_size,
        16.0f, &config);
    return font;
}

void apply_theme() {
    ImGuiStyle& style = ImGui::GetStyle();

    // Rounding
    style.WindowRounding = 6.0f;
    style.FrameRounding = 6.0f;
    style.GrabRounding = 6.0f;
    style.TabRounding = 6.0f;
    style.PopupRounding = 6.0f;
    style.ChildRounding = 6.0f;
    style.ScrollbarRounding = 6.0f;

    // Spacing
    style.WindowPadding = ImVec2(12, 12);
    style.FramePadding = ImVec2(10, 6);
    style.ItemSpacing = ImVec2(8, 6);
    style.ScrollbarSize = 12.0f;

    // Colors
    ImVec4* c = style.Colors;
    ImVec4 bg       = ImVec4(0.102f, 0.102f, 0.102f, 1.0f);  // #1a1a1a
    ImVec4 panel    = ImVec4(0.133f, 0.133f, 0.133f, 1.0f);  // #222222
    ImVec4 text     = ImVec4(0.800f, 0.800f, 0.800f, 1.0f);  // #cccccc
    ImVec4 accent   = ImVec4(0.784f, 0.902f, 0.196f, 1.0f);  // #c8e632
    ImVec4 dim_text = ImVec4(0.500f, 0.500f, 0.500f, 1.0f);
    ImVec4 hover    = ImVec4(0.200f, 0.200f, 0.200f, 1.0f);
    ImVec4 active   = ImVec4(0.250f, 0.250f, 0.250f, 1.0f);

    c[ImGuiCol_WindowBg]             = bg;
    c[ImGuiCol_ChildBg]              = bg;
    c[ImGuiCol_PopupBg]              = panel;
    c[ImGuiCol_Border]               = ImVec4(0.18f, 0.18f, 0.18f, 1.0f);
    c[ImGuiCol_FrameBg]              = panel;
    c[ImGuiCol_FrameBgHovered]       = hover;
    c[ImGuiCol_FrameBgActive]        = active;
    c[ImGuiCol_TitleBg]              = panel;
    c[ImGuiCol_TitleBgActive]        = panel;
    c[ImGuiCol_MenuBarBg]            = panel;
    c[ImGuiCol_ScrollbarBg]          = bg;
    c[ImGuiCol_ScrollbarGrab]        = hover;
    c[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.30f, 0.30f, 0.30f, 1.0f);
    c[ImGuiCol_ScrollbarGrabActive]  = ImVec4(0.35f, 0.35f, 0.35f, 1.0f);
    c[ImGuiCol_CheckMark]            = accent;
    c[ImGuiCol_SliderGrab]           = accent;
    c[ImGuiCol_SliderGrabActive]     = accent;
    c[ImGuiCol_Button]               = panel;
    c[ImGuiCol_ButtonHovered]        = hover;
    c[ImGuiCol_ButtonActive]         = active;
    c[ImGuiCol_Header]               = panel;
    c[ImGuiCol_HeaderHovered]        = hover;
    c[ImGuiCol_HeaderActive]         = active;
    c[ImGuiCol_Separator]            = ImVec4(0.18f, 0.18f, 0.18f, 1.0f);
    c[ImGuiCol_Tab]                  = panel;
    c[ImGuiCol_TabHovered]           = hover;
    c[ImGuiCol_Text]                 = text;
    c[ImGuiCol_TextDisabled]         = dim_text;
    c[ImGuiCol_PlotHistogram]        = accent;
}

} // namespace peek
