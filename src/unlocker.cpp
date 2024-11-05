#include "unlocker.h"
#include "config.h"
#include "filter.h"
#include "hook.h"
#include "input.h"

#include <chrono>
#include <cmath>
#include <filesystem>
#include <mutex>

namespace fs = std::filesystem;

Unlocker& Unlocker::GetInstance() {
    static Unlocker instance {};
    return instance;
}

void Unlocker::SetWorkDir(const fs::path& path) {
    workDir = path;
}

void Unlocker::Initialize() {
    static bool isInitialized = false;
    if (isInitialized) {
        return;
    }

    InitializeConfig();
    InitializeHook();
    InitializeInput();
    InitializeFilter();

    isInitialized = true;
}

void Unlocker::InitializeConfig() {
    const fs::path path = workDir / "fov_config.json";
    if (!fs::exists(path)) {
        config.ToJson(path);
    } else {
        config.FromJson(path);
    }
}

void Unlocker::InitializeHook() {
    const auto module = reinterpret_cast<uintptr_t>(GetModuleHandle(nullptr));
    const auto isGlobal = GetModuleHandle("GenshinImpact.exe") != nullptr;
    const uintptr_t offset = isGlobal ? 0x1136f30 : 0x1136d30;
    const auto target = reinterpret_cast<void*>(module + offset);

    hook.Initialize();
    hook.Create(target, &HkSetFov);
}

void Unlocker::InitializeInput() {
    const DWORD tracked = GetCurrentProcessId();
    input.SetTrackedProcess(tracked);
    input.RegisterKeys({
        config.hookKey,
        config.enableKey,
        config.nextKey,
        config.prevKey
    });
    input.RegisterOnKeyDown([this](const int vKey) {
        this->OnKeyDown(vKey);
    });
    input.StartPolling(30);
}

void Unlocker::InitializeFilter() {
    const float timeConstant = config.smoothing;
    filter.SetTimeConstant(timeConstant);
}

void Unlocker::OnKeyDown(const int vKey) {
    std::lock_guard lock(configMutex);

    if (vKey == config.hookKey) {
        if (!hook.IsEnabled()) {
            hook.Enable();
        }
    } else if (vKey == config.nextKey && config.enabled) {
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

void Unlocker::FilterAndSetFov(void* instance, float value) {
    std::lock_guard lock(configMutex);

    if (!config.interpolate) {
        if (value == 45.0 && config.enabled) {
            value = static_cast<float>(config.fov);
        }
        hook.CallOriginal(instance, value);
        return;
    }

    const auto currentTime = std::chrono::steady_clock::now();
    const auto deltaTime = std::chrono::duration<float, std::milli>(
        currentTime - previousTime).count();
    if (deltaTime > config.threshold) {
        if (setFovCount == 1) {
            filter.SetInitialValue(firstFov);
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
            const auto filtered = filter.Update(target);
            const auto isOnTarget = std::abs(filtered - target) < 0.1;

            if (config.enabled || !isOnTarget) {
                value = filtered;
            } else {
                filter.SetInitialValue(firstFov);
            }
            break;
        }
        default: {
            break;
        }
    }

    hook.CallOriginal(instance, value);
}

void Unlocker::HkSetFov(void* instance, float value) {
    GetInstance().FilterAndSetFov(instance, value);
}
