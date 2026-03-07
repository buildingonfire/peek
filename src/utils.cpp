#include "utils.h"
#include <windows.h>

namespace peek {

std::wstring utf8_to_wide(const std::string& str) {
    if (str.empty()) return {};
    int len = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), nullptr, 0);
    std::wstring wstr(len, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), wstr.data(), len);
    return wstr;
}

std::string wide_to_utf8(const std::wstring& wstr) {
    if (wstr.empty()) return {};
    int len = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), nullptr, 0, nullptr, nullptr);
    std::string str(len, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), str.data(), len, nullptr, nullptr);
    return str;
}

std::string get_file_extension_lower(const std::string& path) {
    auto ext = std::filesystem::path(path).extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    return ext;
}

std::string get_file_stem(const std::string& path) {
    return std::filesystem::path(path).stem().string();
}

std::string get_parent_dir(const std::string& path) {
    return std::filesystem::path(path).parent_path().string();
}

bool is_supported_image(const std::string& ext) {
    static const std::vector<std::string> exts = {
        ".png", ".jpg", ".jpeg", ".bmp", ".gif", ".tiff", ".tif",
        ".webp", ".ico", ".jfif", ".psd", ".tga", ".hdr",
        ".heic", ".heif", ".exr"
    };
    std::string lower = ext;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    for (auto& e : exts) {
        if (lower == e) return true;
    }
    return false;
}

std::string format_file_size(uint64_t bytes) {
    if (bytes < 1024) return std::to_string(bytes) + " B";
    if (bytes < 1024 * 1024) return std::to_string(bytes / 1024) + " KB";
    return std::to_string(bytes / (1024 * 1024)) + "." + std::to_string((bytes / (1024 * 100)) % 10) + " MB";
}

std::string generate_save_path(const std::string& original_path, const std::string& append_str, const std::string& ext) {
    namespace fs = std::filesystem;
    fs::path orig(original_path);
    std::string stem = orig.stem().string();
    fs::path dir = orig.parent_path();

    std::string base_name = stem + "_" + append_str;
    fs::path candidate = dir / (base_name + ext);

    if (!fs::exists(candidate)) {
        return candidate.string();
    }

    // Try _a, _b, _c, ... _z, _aa, _ab, ...
    for (int i = 0; i < 26 * 27; i++) {
        std::string suffix;
        if (i < 26) {
            suffix = std::string(1, 'a' + i);
        } else {
            suffix = std::string(1, 'a' + (i / 26 - 1)) + std::string(1, 'a' + (i % 26));
        }
        candidate = dir / (base_name + "_" + suffix + ext);
        if (!fs::exists(candidate)) {
            return candidate.string();
        }
    }

    return (dir / (base_name + "_zzz" + ext)).string();
}

} // namespace peek
