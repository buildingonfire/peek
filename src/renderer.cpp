#include "renderer.h"
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"

namespace peek {

bool Renderer::init(HWND hwnd_) {
    hwnd = hwnd_;

    DXGI_SWAP_CHAIN_DESC sd = {};
    sd.BufferCount = 2;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hwnd;
    sd.SampleDesc.Count = 1;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    D3D_FEATURE_LEVEL feature_level;
    UINT flags = 0;
#ifdef _DEBUG
    flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, flags,
        nullptr, 0, D3D11_SDK_VERSION,
        &sd, &swap_chain, &device, &feature_level, &context);

    if (FAILED(hr)) return false;

    create_rtv();

    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(device, context);

    return true;
}

void Renderer::shutdown() {
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();

    release_rtv();
    if (swap_chain) { swap_chain->Release(); swap_chain = nullptr; }
    if (context) { context->Release(); context = nullptr; }
    if (device) { device->Release(); device = nullptr; }
}

void Renderer::resize(int w, int h) {
    if (w <= 0 || h <= 0) return;
    width = w;
    height = h;

    release_rtv();
    swap_chain->ResizeBuffers(0, w, h, DXGI_FORMAT_UNKNOWN, 0);
    create_rtv();
}

void Renderer::begin_frame() {
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
}

void Renderer::end_frame() {
    ImGui::Render();

    const float clear_color[4] = { 0.102f, 0.102f, 0.102f, 1.0f }; // #1a1a1a
    context->OMSetRenderTargets(1, &rtv, nullptr);
    context->ClearRenderTargetView(rtv, clear_color);

    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    swap_chain->Present(1, 0);
}

void Renderer::create_rtv() {
    ID3D11Texture2D* back_buffer = nullptr;
    swap_chain->GetBuffer(0, IID_PPV_ARGS(&back_buffer));
    if (back_buffer) {
        device->CreateRenderTargetView(back_buffer, nullptr, &rtv);
        back_buffer->Release();
    }
}

void Renderer::release_rtv() {
    if (rtv) { rtv->Release(); rtv = nullptr; }
}

} // namespace peek
