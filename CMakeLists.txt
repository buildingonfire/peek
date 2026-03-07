cmake_minimum_required(VERSION 3.20)
project(Peek VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Static CRT linkage for self-contained exe
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

# ImGui sources
set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/third_party/imgui)
set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_win32.cpp
    ${IMGUI_DIR}/backends/imgui_impl_dx11.cpp
)

# Application sources
set(APP_SOURCES
    src/main.cpp
    src/app.cpp
    src/renderer.cpp
    src/image_loader.cpp
    src/image_viewport.cpp
    src/annotation_layer.cpp
    src/annotation_tools.cpp
    src/save_engine.cpp
    src/ui_panels.cpp
    src/theme.cpp
    src/file_assoc.cpp
    src/folder_browser.cpp
    src/drag_drop.cpp
    src/utils.cpp
)

# stb implementation
set(STB_SOURCES src/stb_impl.cpp)

# tinyfiledialogs
set(TFD_SOURCES third_party/tinyfiledialogs/tinyfiledialogs.c)

add_executable(Peek WIN32
    ${APP_SOURCES}
    ${IMGUI_SOURCES}
    ${STB_SOURCES}
    ${TFD_SOURCES}
    resources/peek.rc
)

target_include_directories(Peek PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
    ${CMAKE_SOURCE_DIR}/third_party/stb
    ${CMAKE_SOURCE_DIR}/third_party/tinyfiledialogs
    ${CMAKE_SOURCE_DIR}/third_party/tinyexr
)

target_link_libraries(Peek PRIVATE
    d3d11
    dxgi
    d3dcompiler
    dwmapi
    windowscodecs
    ole32
    shlwapi
    comdlg32
    shell32
    advapi32
)

target_compile_definitions(Peek PRIVATE
    UNICODE
    _UNICODE
    WIN32_LEAN_AND_MEAN
    NOMINMAX
)

# Enable LTO for Release
set_property(TARGET Peek PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)

# Strip debug info in Release
if(MSVC)
    target_compile_options(Peek PRIVATE
        $<$<CONFIG:Release>:/O2 /GL>
        /W3
    )
    target_link_options(Peek PRIVATE
        $<$<CONFIG:Release>:/LTCG /OPT:REF /OPT:ICF>
        /MANIFEST:NO
    )
endif()
