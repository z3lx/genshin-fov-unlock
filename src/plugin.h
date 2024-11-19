#pragma once

#include "config.h"
#include "filter.h"
#include "hook.h"
#include "input.h"
#include <filesystem>
#include <mutex>

#ifdef ENABLE_LOGGING
#include <spdlog/spdlog.h>
#include <chrono>
#include <memory>
#endif

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

#ifdef ENABLE_LOGGING
    void InitializeLogger();
#endif
    void InitializeConfig() noexcept;
    void InitializeInput();
    void InitializeUnlocker();

    static void HkSetFov(void* instance, float value);
    void FilterAndSetFov(void* instance, float value);
    void OnKeyDown(int vKey);

    bool isInitialized = false;
    std::filesystem::path workDir;

#ifdef ENABLE_LOGGING
    std::shared_ptr<spdlog::logger> logger;
    std::shared_ptr<spdlog::logger> csvLogger;
    std::chrono::steady_clock::time_point start;
#endif

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
