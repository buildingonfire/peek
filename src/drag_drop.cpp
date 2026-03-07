#include "drag_drop.h"
#include "utils.h"

namespace peek {

std::string handle_drop_files(HDROP hdrop) {
    wchar_t path[MAX_PATH];
    UINT count = DragQueryFileW(hdrop, 0xFFFFFFFF, nullptr, 0);
    if (count > 0) {
        DragQueryFileW(hdrop, 0, path, MAX_PATH);
        DragFinish(hdrop);
        return wide_to_utf8(path);
    }
    DragFinish(hdrop);
    return {};
}

} // namespace peek
