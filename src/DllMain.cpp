#include "Plugin.h"

std::filesystem::path GetPath(const HMODULE module) {
    char path[MAX_PATH];
    GetModuleFileName(module, path, MAX_PATH);
    return std::filesystem::path(path).parent_path();
}

void Uninitialize(HINSTANCE hinstDLL) {
    Plugin& plugin = Plugin::GetInstance();
    plugin.Uninitialize();
    FreeLibrary(hinstDLL);
}

void Initialize(HINSTANCE hinstDLL) try {
    Plugin& plugin = Plugin::GetInstance();
    plugin.SetWorkDir(GetPath(hinstDLL));
    plugin.Initialize();
} catch (...) {
    Uninitialize(hinstDLL);
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved) {
    DisableThreadLibraryCalls(hinstDLL);

    switch (fdwReason) {
        case DLL_PROCESS_ATTACH: {
            std::thread([hinstDLL]() {
                Initialize(hinstDLL);
            }).detach();
            break;
        }
        case DLL_PROCESS_DETACH: {
            Uninitialize(hinstDLL);
            break;
        }
        default: {
            break;
        }
    }

    return TRUE;
}
