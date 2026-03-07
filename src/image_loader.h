#pragma once
#include <string>
#include <vector>
#include <d3d11.h>

namespace peek {

struct LoadedImage {
    std::vector<uint8_t> pixels; // RGBA, full resolution
    int width = 0;
    int height = 0;
    int channels = 4;
    ID3D11ShaderResourceView* srv = nullptr;
    int tex_width = 0;  // may be downsampled for display
    int tex_height = 0;
    bool valid = false;

    void release_gpu();
};

bool load_image_from_file(const std::string& path, LoadedImage& img);
bool upload_to_gpu(ID3D11Device* device, LoadedImage& img);

} // namespace peek
