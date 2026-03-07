#pragma once
#include <string>

namespace peek {

bool is_file_association_set();
bool set_file_associations(const std::string& exe_path);
bool remove_file_associations();
void show_first_run_dialog(const std::string& exe_path);

} // namespace peek
