#pragma once
#include <d3d11.h>
#include <dxgi.h>
#include <windows.h>

namespace peek {

struct Renderer {
    ID3D11Device* device = nullptr;
    ID3D11DeviceContext* context = nullptr;
    IDXGISwapChain* swap_chain = nullptr;
    ID3D11RenderTargetView* rtv = nullptr;
    HWND hwnd = nullptr;
    int width = 0;
    int height = 0;

    bool init(HWND hwnd);
    void shutdown();
    void resize(int w, int h);
    void begin_frame();
    void end_frame();

private:
    void create_rtv();
    void release_rtv();
};

} // namespace peek
