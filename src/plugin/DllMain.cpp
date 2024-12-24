#include "plugin/Plugin.h"

#include <thread>

#include <Windows.h>

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved) {
    DisableThreadLibraryCalls(hinstDLL);

    switch (fdwReason) {
        case DLL_PROCESS_ATTACH: {
            std::thread([] { Plugin::Initialize(); }).detach();
            break;
        }
        case DLL_PROCESS_DETACH: {
            Plugin::Uninitialize();
            break;
        }
        default: {
            break;
        }
    }

    return TRUE;
}
