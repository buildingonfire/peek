#include "folder_browser.h"
#include "utils.h"
#include <filesystem>
#include <algorithm>

namespace peek {

void FolderBrowser::scan(const std::string& image_path) {
    namespace fs = std::filesystem;
    files.clear();
    current_index = -1;

    fs::path p(image_path);
    directory = p.parent_path().string();

    for (auto& entry : fs::directory_iterator(directory)) {
        if (!entry.is_regular_file()) continue;
        std::string ext = entry.path().extension().string();
        if (is_supported_image(ext)) {
            files.push_back(entry.path().string());
        }
    }

    std::sort(files.begin(), files.end());

    std::string abs_path = fs::absolute(p).string();
    for (int i = 0; i < (int)files.size(); i++) {
        if (fs::absolute(files[i]).string() == abs_path) {
            current_index = i;
            break;
        }
    }
}

std::string FolderBrowser::prev() {
    if (files.empty()) return {};
    if (current_index <= 0) current_index = (int)files.size();
    current_index--;
    return files[current_index];
}

std::string FolderBrowser::next() {
    if (files.empty()) return {};
    current_index++;
    if (current_index >= (int)files.size()) current_index = 0;
    return files[current_index];
}

std::string FolderBrowser::current() const {
    if (current_index >= 0 && current_index < (int)files.size())
        return files[current_index];
    return {};
}

} // namespace peek
