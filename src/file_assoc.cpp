#include "file_assoc.h"
#include "utils.h"
#include <windows.h>
#include <shlobj.h>
#include <string>
#include <vector>

namespace peek {

static const wchar_t* PROG_ID = L"Peek.ImageViewer";
static const std::vector<std::wstring> IMAGE_EXTS = {
    L".png", L".jpg", L".jpeg", L".bmp", L".gif", L".tiff", L".tif",
    L".webp", L".ico", L".jfif", L".heic", L".heif", L".exr"
};

static bool set_registry_value(HKEY root, const std::wstring& subkey,
                                const std::wstring& name, const std::wstring& value) {
    HKEY hkey;
    LONG result = RegCreateKeyExW(root, subkey.c_str(), 0, nullptr,
                                   REG_OPTION_NON_VOLATILE, KEY_SET_VALUE, nullptr, &hkey, nullptr);
    if (result != ERROR_SUCCESS) return false;
    result = RegSetValueExW(hkey, name.empty() ? nullptr : name.c_str(), 0, REG_SZ,
                             (const BYTE*)value.c_str(), (DWORD)((value.size() + 1) * sizeof(wchar_t)));
    RegCloseKey(hkey);
    return result == ERROR_SUCCESS;
}

bool is_file_association_set() {
    HKEY hkey;
    std::wstring key = std::wstring(L"Software\\Classes\\") + PROG_ID;
    LONG result = RegOpenKeyExW(HKEY_CURRENT_USER, key.c_str(), 0, KEY_READ, &hkey);
    if (result == ERROR_SUCCESS) {
        RegCloseKey(hkey);
        return true;
    }
    return false;
}

bool set_file_associations(const std::string& exe_path) {
    std::wstring wexe = utf8_to_wide(exe_path);
    std::wstring base = L"Software\\Classes\\";

    // Register ProgID
    std::wstring prog_key = base + PROG_ID;
    set_registry_value(HKEY_CURRENT_USER, prog_key, L"", L"Peek Image Viewer");
    set_registry_value(HKEY_CURRENT_USER, prog_key + L"\\DefaultIcon", L"", wexe + L",0");
    set_registry_value(HKEY_CURRENT_USER, prog_key + L"\\shell\\open\\command", L"",
                        L"\"" + wexe + L"\" \"%1\"");

    // Associate extensions
    for (auto& ext : IMAGE_EXTS) {
        std::wstring ext_key = base + ext;
        set_registry_value(HKEY_CURRENT_USER, ext_key, L"", std::wstring(PROG_ID));
    }

    // Notify shell
    SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, nullptr, nullptr);
    return true;
}

bool remove_file_associations() {
    std::wstring base = L"Software\\Classes\\";

    for (auto& ext : IMAGE_EXTS) {
        std::wstring ext_key = base + ext;
        RegDeleteTreeW(HKEY_CURRENT_USER, ext_key.c_str());
    }

    std::wstring prog_key = base + PROG_ID;
    RegDeleteTreeW(HKEY_CURRENT_USER, prog_key.c_str());

    SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, nullptr, nullptr);
    return true;
}

void show_first_run_dialog(const std::string& exe_path) {
    if (is_file_association_set()) return;

    int result = MessageBoxW(nullptr,
        L"Would you like to set Peek as your default image viewer?\n\n"
        L"This will associate common image formats (.png, .jpg, .bmp, etc.) with Peek.",
        L"Peek - First Run",
        MB_YESNO | MB_ICONQUESTION);

    if (result == IDYES) {
        set_file_associations(exe_path);
    }
}

} // namespace peek
