#pragma once
#include <string>
#include <vector>

namespace peek {

struct FolderBrowser {
    std::vector<std::string> files;
    int current_index = -1;
    std::string directory;

    void scan(const std::string& image_path);
    std::string prev();
    std::string next();
    std::string current() const;
};

} // namespace peek
