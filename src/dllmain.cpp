#include "plugin.h"

std::filesystem::path GetPath(const HMODULE module) {
    char path[MAX_PATH];
    GetModuleFileName(module, path, MAX_PATH);
    return std::filesystem::path(path).parent_path();
}

void Initialize(HINSTANCE hinstDLL) try {
    Plugin& plugin = Plugin::GetInstance();
    plugin.SetWorkDir(GetPath(hinstDLL));
    plugin.Initialize();
} catch (...) {
    // TODO: CLEANUP
    FreeLibraryAndExitThread(hinstDLL, 1);
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved) {
    DisableThreadLibraryCalls(hinstDLL);

    if (fdwReason == DLL_PROCESS_ATTACH) {
        const auto fn = reinterpret_cast<LPTHREAD_START_ROUTINE>(Initialize);
        const auto args = reinterpret_cast<LPVOID>(hinstDLL);
        if (const auto handle = CreateThread(nullptr, 0, fn, args, 0, nullptr);
            handle != nullptr) {
            CloseHandle(handle);
        }
    }

    return TRUE;
}
