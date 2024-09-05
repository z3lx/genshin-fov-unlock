#include "config.h"
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <windows.h>
#include <MinHook.h>
#include <nlohmann/json.hpp>

Config config;

void InitializeConfig() {
    HMODULE module = nullptr;
    GetModuleHandleExA(
        GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
        GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
        reinterpret_cast<LPCTSTR>(&InitializeConfig),
        &module
    );
    char modulePath[MAX_PATH];
    GetModuleFileNameA(module, modulePath, MAX_PATH);
    auto moduleDir = std::filesystem::path(modulePath).parent_path();
    auto configPath = moduleDir / "fov_config.json";

    if (std::filesystem::exists(configPath)) {
        std::ifstream file(configPath);
        nlohmann::json json;
        file >> json;
        config = json.get<Config>();
    } else {
        nlohmann::json json = config;
        std::ofstream file(configPath);
        file << json.dump(4);
    }
}

void* trampoline = nullptr;

void SetFieldOfView(void* _this, float value) {
    if (value == 45.0f)
        value = config.fieldOfView;
    reinterpret_cast<decltype(&SetFieldOfView)>(trampoline)(_this, value);
}

void InitializeHook() {
    auto module = reinterpret_cast<uintptr_t>(GetModuleHandleA(nullptr));
    auto offset = GetModuleHandleA("GenshinImpact.exe") ? 0x165a1d0 : 0x165f1d0;
    auto target = reinterpret_cast<void*>(module + offset);
    auto detour = reinterpret_cast<void*>(&SetFieldOfView);

    MH_Initialize();
    MH_CreateHook(target, detour, &trampoline);
    MH_EnableHook(MH_ALL_HOOKS);
}

void Initialize() {
    InitializeConfig();
    InitializeHook();
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
