#include "main.h"
#include "config.h"
#include "filter.h"
#include "hook.h"
#include "input.h"
#include <chrono>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <memory>
#include <xutility>
#include <nlohmann/json.hpp>
#include <windows.h>

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

void WriteConfig(
    const Config& config,
    const std::filesystem::path& configPath
) {
    const nlohmann::ordered_json json = config;
    std::ofstream file(configPath);
    file << json.dump(4);
}

Config ReadConfig(
    const std::filesystem::path& configPath
) {
    if (!exists(configPath)) {
        Config config {};
        WriteConfig(config, configPath);
        return config;
    }
    std::ifstream file(configPath);
    nlohmann::json json;
    file >> json;
    return json.get<Config>();
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
    const auto moduleDir = std::filesystem::path(modulePath).parent_path();
    return moduleDir / "fov_config.json";
}

void InitializeConfig() {
    configPath = GetConfigPath();
    config = ReadConfig(configPath);
}

// Hooks

typedef void(*SetFovPtr)(void* instance, float value);
SetFovPtr SetFov;
std::unique_ptr<Hook> setFovHook;
void HkSetFov(void* instance, float value);

void InitializeHooks() {
    const auto module = reinterpret_cast<uintptr_t>(GetModuleHandle(nullptr));
    const auto isGlobal = GetModuleHandle("GenshinImpact.exe") != nullptr;
    const auto offset = isGlobal ? 0x1136f30 : 0x1136d30;
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
    const auto trackedProcess = GetCurrentProcessId();
    input = std::make_unique<InputManager>(trackedProcess);
    input->RegisterKeys({
        config.enableKey,
        config.nextKey,
        config.prevKey
    });
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
float firstFov = 0;
std::chrono::time_point<std::chrono::steady_clock> previousTime;

void OnKeyDown(const int vKey) {
    if (vKey == config.nextKey && config.enabled) {
        const auto it = std::ranges::find_if(
            config.fovPresets,
            [&](const int presetFov) { return config.fov < presetFov; }
        );
        config.fov = it != config.fovPresets.end()
            ? *it : config.fovPresets.front();
    } else if (vKey == config.prevKey && config.enabled) {
        const auto it = std::ranges::find_if(
            config.fovPresets | std::views::reverse,
            [&](const int presetFov) { return config.fov > presetFov; }
        );
        config.fov = it != config.fovPresets.rend()
            ? *it : config.fovPresets.back();
    } else if (vKey == config.enableKey) {
        config.enabled = !config.enabled;
    }
}

void HkSetFov(void* instance, float value) {
    if (!config.interpolate) {
        input->Poll();
        if (value == 45.0 && config.enabled) {
            value = static_cast<float>(config.fov);
        }
        SetFov(instance, value);
        return;
    }

    const auto currentTime = std::chrono::steady_clock::now();
    const auto deltaTime = std::chrono::duration<float, std::milli>(
        currentTime - previousTime).count();
    if (deltaTime > config.threshold) {
        input->Poll();

        if (setFovCount == 1) {
            filter->SetInitialValue(firstFov);
        }
        previousTime = currentTime;
        setFovCount = 0;
    }

    setFovCount++;
    const auto currentInstance = reinterpret_cast<uintptr_t>(instance);
    switch (setFovCount) {
        case 1: {
            firstInstance = currentInstance;
            firstFov = value;
            break;
        }
        case 2: {
            if (currentInstance != firstInstance) {
                break;
            }

            const auto target = config.enabled
                ? static_cast<float>(config.fov) : value;
            const auto filtered = filter->Update(target);
            const auto isOnTarget = std::abs(filtered - target) < 0.1;

            if (config.enabled || !isOnTarget) {
                value = filtered;
            } else {
                filter->SetInitialValue(firstFov);
            }
            break;
        }
        default: {
            break;
        }
    }

    SetFov(instance, value);
}
