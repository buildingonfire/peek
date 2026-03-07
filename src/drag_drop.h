#pragma once
#include <string>
#include <windows.h>
#include <shellapi.h>

namespace peek {

std::string handle_drop_files(HDROP hdrop);

} // namespace peek
