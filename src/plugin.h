#pragma once

#include "config.h"
#include "filter.h"
#include "hook.h"
#include "input.h"

#include <spdlog/spdlog.h>

#include <chrono>
#include <filesystem>
#include <memory>
#include <mutex>

class Plugin {
public:
    Plugin(const Plugin&) = delete;
    Plugin& operator=(const Plugin&) = delete;

    static Plugin& GetInstance();
    void SetWorkDir(const std::filesystem::path& path);
    void Initialize();

private:
    Plugin() = default;
    ~Plugin() = default;

    void InitializeLogger();
    void InitializeConfig();
    void InitializeHook();
    void InitializeInput();
    void InitializeFilter() noexcept;

    static void HkSetFov(void* instance, float value);
    void FilterAndSetFov(void* instance, float value);
    void OnKeyDown(int vKey);

    std::filesystem::path workDir;

    std::shared_ptr<spdlog::logger> logger;
    Config config;
    Hook<void, void*, float> hook;
    InputManager input;
    ExponentialFilter<float> filter;

    std::mutex configMutex;
    int setFovCount = 0;
    uintptr_t firstInstance = 0;
    float firstFov = 0;
    std::chrono::time_point<std::chrono::steady_clock> previousTime;
};
