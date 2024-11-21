#include "plugin.h"
#include "config.h"
#include "filter.h"
#include "hook.h"
#include "input.h"
#include <cmath>
#include <filesystem>
#include <initializer_list>
#include <mutex>
#include <ranges>

#ifdef ENABLE_LOGGING
#include <chrono>
#include <memory>
#include <spdlog/sinks/basic_file_sink.h>
#include "sink.h"

#define LOG(logger, level, ...) \
    if (logger) logger->log(level, __VA_ARGS__)
#define FLUSH(logger) \
    if (logger) logger->flush()
#else
#define LOG(logger, level, ...) ((void)0)
#define FLUSH(logger) ((void)0)
#endif

namespace fs = std::filesystem;

Plugin& Plugin::GetInstance() {
    static Plugin instance {};
    return instance;
}

void Plugin::SetWorkDir(const fs::path& path) {
    workDir = path;
}

void Plugin::Initialize() try {
    std::lock_guard lock(mutex);
    if (isInitialized) {
        return;
    }

#ifdef ENABLE_LOGGING
    InitializeLogger();
#endif
    InitializeConfig();
    InitializeInput();
    InitializeUnlocker();
    isInitialized = true;

    LOG(logger, spdlog::level::info,
        "Plugin initialized successfully");
    FLUSH(logger);
} catch ([[maybe_unused]] const std::exception& e) {
    LOG(logger, spdlog::level::critical,
        "Failed to initialize Plugin: {}", e.what());
    throw;
}

void Plugin::Uninitialize() try {
    std::lock_guard lock(mutex);
    if (!isInitialized) {
        return;
    }

    hook.Uninitialize();
    input.StopPolling();
    isInitialized = false;

    LOG(logger, spdlog::level::info,
        "Plugin uninitialized successfully");
} catch ([[maybe_unused]] const std::exception& e) {
    LOG(logger, spdlog::level::critical,
        "Failed to uninitialize Plugin: {}", e.what());
}

#ifdef ENABLE_LOGGING
void Plugin::InitializeLogger() try {
    std::string filename = (workDir / "log.txt").string();
    logger = spdlog::basic_logger_st("plugin", filename, true);
    logger->set_level(spdlog::level::trace);
    logger->flush_on(spdlog::level::err);

    filename = (workDir / "values.csv").string();
    csvLogger = TimeBufferedLoggerMt("csvLogger", filename, 10000, true);
    csvLogger->set_pattern("%v");
    csvLogger->set_level(spdlog::level::trace);
    csvLogger->flush_on(spdlog::level::off);

    logger->info("Logger initialized successfully");
    csvLogger->info("time,camera,fov");
    csvLogger->flush();
    start = std::chrono::steady_clock::now();
} catch ([[maybe_unused]] const std::exception& e) {
    LOG(logger, spdlog::level::critical,
        "Failed to initialize Logger: {}", e.what());
    throw;
}
#endif

void Plugin::InitializeConfig() noexcept {
    LOG(logger, spdlog::level::info,
        "Working directory: {}", workDir.string());
    if (const fs::path path = workDir / "fov_config.json";
        fs::exists(path)) {
        try {
            config.FromJson(path);
            LOG(logger, spdlog::level::info,
                "Config loaded successfully");
        } catch ([[maybe_unused]] const std::exception& e) {
            LOG(logger, spdlog::level::err,
                "Failed to load Config: {}", e.what());
        }
    } else {
        try {
            LOG(logger, spdlog::level::info,
                "Config file not found, creating default Config");
            config.ToJson(path);
            LOG(logger, spdlog::level::info,
                "Default Config created successfully");
        } catch ([[maybe_unused]] const std::exception& e) {
            LOG(logger, spdlog::level::err,
                "Failed to create default Config: {}", e.what());
        }
    }
}

void Plugin::InitializeInput() try {
    const DWORD tracked = GetCurrentProcessId();
    input.SetTrackedProcess(tracked);

    const std::initializer_list keys = {
        config.hookKey,
        config.enableKey,
        config.nextKey,
        config.prevKey,
#ifdef ENABLE_LOGGING
        config.dumpKey
#endif
    };
    input.RegisterKeys(keys);
    input.RegisterOnKeyDown([this](const int vKey) {
        this->OnKeyDown(vKey);
    });

    constexpr int pollingRate = 30;
    input.StartPolling(pollingRate);

    LOG(logger, spdlog::level::info,
        "Input initialized successfully");
} catch ([[maybe_unused]] const std::exception& e) {
    LOG(logger, spdlog::level::critical,
        "Failed to initialize Input: {}", e.what());
    throw;
}

void Plugin::InitializeUnlocker() try {
    const auto module = reinterpret_cast<uintptr_t>(GetModuleHandle(nullptr));
    const auto global = GetModuleHandle("GenshinImpact.exe") != nullptr;
    const auto offset = global ? 0x13F87C0 : 0x13F38A0;
    const auto target = reinterpret_cast<void*>(module + offset);

    hook.Initialize();
    hook.Create(target, &HkSetFov);

    filter.SetTimeConstant(static_cast<float>(config.smoothing));

    LOG(logger, spdlog::level::debug,
        "Module: 0x{:X}", module);
    LOG(logger, spdlog::level::debug,
        "Global: {}", global);
    LOG(logger, spdlog::level::debug,
        "Offset: 0x{:X}", offset);
    LOG(logger, spdlog::level::info,
        "Unlocker initialized successfully");
} catch ([[maybe_unused]] const std::exception& e) {
    LOG(logger, spdlog::level::critical,
        "Failed to initialize Unlocker: {}", e.what());
    throw;
}

void Plugin::OnKeyDown(const int vKey) try {
    std::lock_guard lock(mutex);

    if (vKey == config.hookKey) {
        if (hook.IsEnabled()) {
            hook.Disable();
        } else {
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
#ifdef ENABLE_LOGGING
    else if (vKey == config.dumpKey) {
        FLUSH(csvLogger);
    }
#endif
} catch ([[maybe_unused]] const std::exception& e) {
    LOG(logger, spdlog::level::err,
        "Failed to process OnKeyDown: {}", e.what());
}

void Plugin::FilterAndSetFov(void* instance, float value) try {
    if (!config.interpolate) {
        if (value == 45.0f && config.enabled) {
            value = static_cast<float>(config.fov);
        }
        hook.CallOriginal(instance, value);
        return;
    }

    ++setFovCount;
    if (const bool isDefaultFov = value == 45.0f;
        instance == previousInstance &&
        (value == previousValue || isDefaultFov)) {
        if (isDefaultFov) {
            previousInstance = instance;
            previousValue = value;
        }

        if (setFovCount > 8) {
            filter.SetInitialValue(value);
        }
        setFovCount = 0;

        const float target = config.enabled ?
            static_cast<float>(config.fov) : previousValue;
        const float filtered = filter.Update(target);

        if (config.enabled || !isOriginalFov) {
            isOriginalFov = std::abs(previousValue - filtered) < 0.1f;
            value = filtered;
        }
    } else {
        const auto rep = std::bit_cast<std::uint32_t>(value);
        value = std::bit_cast<float>(rep + 1); // marker value
        previousInstance = instance;
        previousValue = value;
    }

#ifdef ENABLE_LOGGING
    const auto now = std::chrono::steady_clock::now();
    const auto elapsed = std::chrono::duration_cast<
        std::chrono::duration<double> // in seconds
    >(now - start).count();
    LOG(csvLogger, spdlog::level::trace,
        "{:.9f},{},{:.9f}", elapsed, instance, value);
#endif

    hook.CallOriginal(instance, value);
} catch ([[maybe_unused]] const std::exception& e) {
    LOG(logger, spdlog::level::err,
        "Failed to set FOV: {}", e.what());
}

void Plugin::HkSetFov(void* instance, const float value) {
    Plugin& plugin = GetInstance();
    std::lock_guard lock(plugin.mutex);
    if (plugin.hook.IsEnabled()) {
        plugin.FilterAndSetFov(instance, value);
    }
}
