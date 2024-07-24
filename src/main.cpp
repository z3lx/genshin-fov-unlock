#include <windows.h>
#include <cstdint>
#include "MinHook.h"

void* trampoline = nullptr;

void SetFieldOfView(void* _this, float value) {
    if (value == 45.0f)
        value = 90.0f;
    reinterpret_cast<decltype(&SetFieldOfView)>(trampoline)(_this, value);
}

void Initialize() {
    auto module = reinterpret_cast<uintptr_t>(GetModuleHandleA(nullptr));
    auto offset = GetModuleHandleA("GenshinImpact.exe") ? 0x1688e60 : 0x1684560;
    auto target = reinterpret_cast<void*>(module + offset);
    auto detour = reinterpret_cast<void*>(&SetFieldOfView);

    MH_Initialize();
    MH_CreateHook(target, detour, &trampoline);
    MH_EnableHook(MH_ALL_HOOKS);
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved) {
    DisableThreadLibraryCalls(hinstDLL);

    if (fdwReason == DLL_PROCESS_ATTACH) {
        auto threadProc = reinterpret_cast<LPTHREAD_START_ROUTINE>(Initialize);
        auto handle = CreateThread(nullptr, 0, threadProc, nullptr, 0, nullptr);
        if (handle != nullptr)
            CloseHandle(handle);
    }

    return TRUE;
}
