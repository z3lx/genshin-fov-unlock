#include "main.h"
#include "config.h"
#include "filter.h"
#include "hook.h"
#include "input.h"
#include <filesystem>
#include <fstream>
#include <d3d11.h>

// TODO: REFACTOR
// TODO: BETTER ERROR HANDLING

void InitializeConfig();
void InitializeHooks();
void InitializeInput();
void InitializeFilter();

int main() {
    InitializeConfig();
    InitializeHooks();
    InitializeInput();
    InitializeFilter();
}

// Config

Config config;
std::filesystem::path configPath;

void WriteConfig() {
    const nlohmann::json json = config;
    std::ofstream file(configPath);
    file << json.dump(4);
}

void ReadConfig() {
    if (!exists(configPath)) {
        WriteConfig();
        return;
    }
    std::ifstream file(configPath);
    nlohmann::json json;
    file >> json;
    config = json.get<Config>();
}

std::filesystem::path GetConfigPath() {
    HMODULE module = nullptr;
    GetModuleHandleEx(
        GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
        GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
        reinterpret_cast<LPCTSTR>(&GetConfigPath),
        &module
    );
    char modulePath[MAX_PATH];
    GetModuleFileName(module, modulePath, MAX_PATH);
    auto moduleDir = std::filesystem::path(modulePath).parent_path();
    return moduleDir / "fov_config.json";
}

void InitializeConfig() {
    configPath = GetConfigPath();
    ReadConfig();
}

// Hooks

typedef void(*SetFovPtr)(void* instance, float value);
SetFovPtr SetFov;
std::unique_ptr<Hook> setFovHook;
void HkSetFov(void* instance, float value);

void InitializeHooks() {
    // SetFov hook
    auto module = reinterpret_cast<uintptr_t>(GetModuleHandle(nullptr));
    auto offset = GetModuleHandle("GenshinImpact.exe") ? 0x165a1d0 : 0x165f1d0;
    SetFov = reinterpret_cast<SetFovPtr>(module + offset);
    setFovHook = std::make_unique<Hook>(
        reinterpret_cast<void**>(&SetFov),
        reinterpret_cast<void*>(HkSetFov)
    );
}

// Input

std::unique_ptr<InputManager> input;
void OnKeyDown(int vKey);

void InitializeInput() {
    input = std::make_unique<InputManager>(0, GetCurrentProcessId());
    input->RegisterOnKeyDown(OnKeyDown);
}

// Filter

std::unique_ptr<ExponentialFilter> filter;

void InitializeFilter() {
    const auto timeConstant = config.smoothing;
    filter = std::make_unique<ExponentialFilter>(timeConstant);
}

// Logic

int setFovCount = 0;
uintptr_t firstInstance = 0;
float firstSetFov = 0.0f;
std::chrono::time_point<std::chrono::steady_clock> previousTime;

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

void HkSetFov(void* instance, float value) {
    const auto currentTime = std::chrono::steady_clock::now();
    const auto deltaTime = std::chrono::duration<float, std::milli>(
        currentTime - previousTime).count();
    if (deltaTime > config.threshold) {
        if (setFovCount == 1) {
            filter->SetInitialValue(firstSetFov);
        }
        previousTime = currentTime;
        setFovCount = 0;
    }

    input->Poll();

    const auto currentInstance = reinterpret_cast<uintptr_t>(instance);
    if (setFovCount == 0) {
        firstInstance = currentInstance;
        firstSetFov = value;
    } else if (setFovCount == 1 &&currentInstance == firstInstance) {
        value = filter->Update(static_cast<float>(config.fov));
    }
    setFovCount++;

    SetFov(instance, value);
}
