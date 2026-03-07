#pragma once
#include <string>
#include <vector>
#include <algorithm>
#include <filesystem>

namespace peek {

std::wstring utf8_to_wide(const std::string& str);
std::string wide_to_utf8(const std::wstring& wstr);
std::string get_file_extension_lower(const std::string& path);
std::string get_file_stem(const std::string& path);
std::string get_parent_dir(const std::string& path);
bool is_supported_image(const std::string& ext);
std::string format_file_size(uint64_t bytes);
std::string generate_save_path(const std::string& original_path, const std::string& append_str, const std::string& ext);

} // namespace peek
