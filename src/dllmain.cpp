#include "main.h"
#include <windows.h>

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved) {
    DisableThreadLibraryCalls(hinstDLL);

    if (fdwReason == DLL_PROCESS_ATTACH) {
        auto threadProc = reinterpret_cast<LPTHREAD_START_ROUTINE>(main);
        auto handle = CreateThread(nullptr, 0, threadProc, nullptr, 0, nullptr);
        if (handle != nullptr)
            CloseHandle(handle);
    }

    return TRUE;
}
