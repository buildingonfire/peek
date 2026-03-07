#include "app.h"
#include "drag_drop.h"
#include "file_assoc.h"
#include "utils.h"
#include "imgui.h"
#include <windows.h>
#include <dwmapi.h>
#include <shellapi.h>
#include <string>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

static peek::App g_app;
static bool g_initialized = false;

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (g_initialized) {
        if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam))
            return true;
    }

    switch (msg) {
        case WM_SIZE:
            if (g_initialized && wParam != SIZE_MINIMIZED) {
                int w = LOWORD(lParam);
                int h = HIWORD(lParam);
                g_app.on_resize(w, h);
            }
            return 0;

        case WM_DROPFILES: {
            std::string path = peek::handle_drop_files((HDROP)wParam);
            if (!path.empty()) {
                g_app.on_drop(path);
            }
            return 0;
        }

        case WM_KEYDOWN: {
            if (!g_initialized) break;
            ImGuiIO& io = ImGui::GetIO();
            bool ctrl = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
            // Always handle Ctrl+S and Ctrl+F, even when ImGui has focus
            if (ctrl && (wParam == 'S' || wParam == 'F')) {
                g_app.on_key((int)wParam);
            } else if (!io.WantCaptureKeyboard) {
                g_app.on_key((int)wParam);
            }
            break;
        }

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }

    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

static std::string parse_command_line() {
    int argc;
    LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    std::string result;
    if (argv && argc > 1) {
        result = peek::wide_to_utf8(argv[1]);
    }
    if (argv) LocalFree(argv);
    return result;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
    // Initialize COM for WIC
    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);

    // Parse command line
    std::string initial_path = parse_command_line();

    // Register window class
    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(wc);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIconW(hInstance, L"IDI_ICON1");
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.lpszClassName = L"PeekImageViewer";
    RegisterClassExW(&wc);

    // Create window
    HWND hwnd = CreateWindowExW(
        WS_EX_ACCEPTFILES,
        L"PeekImageViewer",
        L"Peek",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        1280, 800,
        nullptr, nullptr, hInstance, nullptr);

    if (!hwnd) return 1;

    // Dark title bar
    BOOL dark = TRUE;
    DwmSetWindowAttribute(hwnd, 20 /* DWMWA_USE_IMMERSIVE_DARK_MODE */, &dark, sizeof(dark));

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    // Initialize app
    if (!g_app.init(hwnd, initial_path)) {
        g_app.shutdown();
        CoUninitialize();
        return 1;
    }
    g_initialized = true;

    // First-run file association dialog (deferred)
    {
        wchar_t exe_path[MAX_PATH];
        GetModuleFileNameW(nullptr, exe_path, MAX_PATH);
        peek::show_first_run_dialog(peek::wide_to_utf8(exe_path));
    }

    // Main loop
    MSG msg = {};
    while (true) {
        while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
        if (msg.message == WM_QUIT) break;

        g_app.frame();
    }

    g_app.shutdown();
    CoUninitialize();
    return 0;
}
