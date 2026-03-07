#include "image_loader.h"
#include "utils.h"
#include <windows.h>
#include <wincodec.h>
#include <comdef.h>
#include <algorithm>
#include <cmath>
#include "stb_image.h"
#include "stb_image_resize2.h"
#define TINYEXR_USE_MINIZ (0)
#define TINYEXR_USE_STB_ZLIB (1)
#include "tinyexr.h"

#pragma comment(lib, "windowscodecs.lib")
#pragma comment(lib, "ole32.lib")

namespace peek {

static const int MAX_TEXTURE_DIM = 16384;

void LoadedImage::release_gpu() {
    if (srv) { srv->Release(); srv = nullptr; }
}

static bool load_with_wic(const std::string& path, LoadedImage& img) {
    HRESULT hr;
    IWICImagingFactory* factory = nullptr;
    hr = CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER,
                          IID_PPV_ARGS(&factory));
    if (FAILED(hr)) return false;

    std::wstring wpath = utf8_to_wide(path);
    IWICBitmapDecoder* decoder = nullptr;
    hr = factory->CreateDecoderFromFilename(wpath.c_str(), nullptr, GENERIC_READ,
                                             WICDecodeMetadataCacheOnLoad, &decoder);
    if (FAILED(hr)) { factory->Release(); return false; }

    IWICBitmapFrameDecode* frame = nullptr;
    hr = decoder->GetFrame(0, &frame);
    if (FAILED(hr)) { decoder->Release(); factory->Release(); return false; }

    UINT w, h;
    frame->GetSize(&w, &h);

    IWICFormatConverter* converter = nullptr;
    hr = factory->CreateFormatConverter(&converter);
    if (FAILED(hr)) { frame->Release(); decoder->Release(); factory->Release(); return false; }

    hr = converter->Initialize(frame, GUID_WICPixelFormat32bppRGBA,
                                WICBitmapDitherTypeNone, nullptr, 0.0,
                                WICBitmapPaletteTypeCustom);
    if (FAILED(hr)) {
        converter->Release(); frame->Release(); decoder->Release(); factory->Release();
        return false;
    }

    img.width = (int)w;
    img.height = (int)h;
    img.channels = 4;
    img.pixels.resize((size_t)w * h * 4);

    hr = converter->CopyPixels(nullptr, w * 4, (UINT)img.pixels.size(), img.pixels.data());

    converter->Release();
    frame->Release();
    decoder->Release();
    factory->Release();

    return SUCCEEDED(hr);
}

static bool load_with_stb(const std::string& path, LoadedImage& img) {
    int w, h, ch;
    unsigned char* data = stbi_load(path.c_str(), &w, &h, &ch, 4);
    if (!data) return false;

    img.width = w;
    img.height = h;
    img.channels = 4;
    img.pixels.assign(data, data + (size_t)w * h * 4);
    stbi_image_free(data);
    return true;
}

static bool load_with_exr(const std::string& path, LoadedImage& img) {
    float* rgba = nullptr;
    int w, h;
    const char* err = nullptr;
    int ret = LoadEXR(&rgba, &w, &h, path.c_str(), &err);
    if (ret != TINYEXR_SUCCESS || !rgba) {
        if (err) FreeEXRErrorMessage(err);
        return false;
    }

    // Convert float RGBA [0..inf) to uint8 RGBA with simple tonemap
    img.width = w;
    img.height = h;
    img.channels = 4;
    img.pixels.resize((size_t)w * h * 4);
    for (int i = 0; i < w * h; i++) {
        for (int c = 0; c < 3; c++) {
            float v = rgba[i * 4 + c];
            // Simple Reinhard tonemap
            v = v / (1.0f + v);
            // Gamma correction
            v = powf(v, 1.0f / 2.2f);
            img.pixels[i * 4 + c] = (uint8_t)(std::clamp(v, 0.0f, 1.0f) * 255.0f + 0.5f);
        }
        float a = rgba[i * 4 + 3];
        img.pixels[i * 4 + 3] = (uint8_t)(std::clamp(a, 0.0f, 1.0f) * 255.0f + 0.5f);
    }
    free(rgba);
    return true;
}

bool load_image_from_file(const std::string& path, LoadedImage& img) {
    img.release_gpu();
    img.pixels.clear();
    img.valid = false;

    // EXR needs its own loader
    std::string ext = get_file_extension_lower(path);
    if (ext == ".exr") {
        if (!load_with_exr(path, img)) return false;
        img.valid = true;
        return true;
    }

    // Try WIC first (handles WebP, TIFF, HEIC, etc.)
    if (!load_with_wic(path, img)) {
        // Fallback to stb_image
        if (!load_with_stb(path, img)) {
            return false;
        }
    }

    img.valid = true;
    return true;
}

bool upload_to_gpu(ID3D11Device* device, LoadedImage& img) {
    if (!img.valid || img.pixels.empty()) return false;

    img.release_gpu();

    const uint8_t* src_pixels = img.pixels.data();
    int tex_w = img.width;
    int tex_h = img.height;
    std::vector<uint8_t> downsampled;

    // Downsample if exceeds max texture dimension
    if (tex_w > MAX_TEXTURE_DIM || tex_h > MAX_TEXTURE_DIM) {
        float scale = (float)MAX_TEXTURE_DIM / std::max(tex_w, tex_h);
        tex_w = std::max(1, (int)(img.width * scale));
        tex_h = std::max(1, (int)(img.height * scale));
        downsampled.resize((size_t)tex_w * tex_h * 4);
        stbir_resize_uint8_linear(
            img.pixels.data(), img.width, img.height, img.width * 4,
            downsampled.data(), tex_w, tex_h, tex_w * 4,
            STBIR_RGBA);
        src_pixels = downsampled.data();
    }

    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width = tex_w;
    desc.Height = tex_h;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    D3D11_SUBRESOURCE_DATA init_data = {};
    init_data.pSysMem = src_pixels;
    init_data.SysMemPitch = tex_w * 4;

    ID3D11Texture2D* texture = nullptr;
    HRESULT hr = device->CreateTexture2D(&desc, &init_data, &texture);
    if (FAILED(hr)) return false;

    D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
    srv_desc.Format = desc.Format;
    srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srv_desc.Texture2D.MipLevels = 1;

    hr = device->CreateShaderResourceView(texture, &srv_desc, &img.srv);
    texture->Release();

    if (FAILED(hr)) return false;

    img.tex_width = tex_w;
    img.tex_height = tex_h;
    return true;
}

} // namespace peek
