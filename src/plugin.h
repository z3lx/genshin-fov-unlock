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
    void Uninitialize();

private:
    Plugin() = default;
    ~Plugin() = default;

    void InitializeLogger();
    void InitializeConfig();
    void InitializeInput();
    void InitializeUnlocker();

    static void HkSetFov(void* instance, float value);
    void FilterAndSetFov(void* instance, float value);
    void OnKeyDown(int vKey);

    bool isInitialized = false;
    std::filesystem::path workDir;

    std::shared_ptr<spdlog::logger> logger;
    std::shared_ptr<spdlog::logger> csvLogger;
    std::chrono::steady_clock::time_point start;

    Config config;
    Hook<void, void*, float> hook;
    InputManager input;
    ExponentialFilter<float> filter;

    std::mutex mutex;

    int setFovCount = 0;
    void* previousInstance = nullptr;
    float previousValue = 0;
    bool isOriginalFov = false;
};
