#include "config.h"
#include "input.h"
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <memory>
#include <windows.h>
#include <MinHook.h>
#include <nlohmann/json.hpp>

std::unique_ptr<InputManager> inputManager;
void OnKeyDown(int vKey);

void InitializeInput() {
    inputManager = std::make_unique<InputManager>(
        0, GetCurrentProcessId()
    );
    inputManager->RegisterOnKeyDown(OnKeyDown);
}

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
    inputManager->Poll();
    if (value == 45.0f && config.enabled)
        value = static_cast<float>(config.fov);
    reinterpret_cast<decltype(&SetFieldOfView)>(trampoline)(_this, value);
}

void OnKeyDown(int vKey) {
    if (vKey == config.nextKey && config.enabled) {
        for (auto i = 0; i < config.fovPresets.size(); ++i) {
            if (const auto presetFov = config.fovPresets[i];
                config.fov < presetFov) {
                config.fov = presetFov;
                break;
            }
        }
    } else if (vKey == config.prevKey && config.enabled) {
        for (auto i = config.fovPresets.size(); i > 0; --i) {
            if (const auto presetFov = config.fovPresets[i - 1];
                config.fov > presetFov) {
                config.fov = presetFov;
                break;
            }
        }
    } else if (vKey == config.enableKey) {
        config.enabled = !config.enabled;
    }
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
    InitializeInput();
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
