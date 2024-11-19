#include "plugin.h"
#include "config.h"
#include "filter.h"
#include "hook.h"
#include "input.h"
#include "sink.h"

#include "spdlog/sinks/basic_file_sink.h"

#include <chrono>
#include <cmath>
#include <filesystem>
#include <memory>
#include <mutex>
#include <ranges>

namespace fs = std::filesystem;

#ifdef ENABLE_LOGGING
#define LOG(logger, level, ...) logger->log(level, __VA_ARGS__)
#define FLUSH(logger) logger->flush()
#else
#define LOG(logger, level, ...) ((void)0)
#define FLUSH(logger) ((void)0)
#endif

Plugin& Plugin::GetInstance() {
    static Plugin instance {};
    return instance;
}

void Plugin::SetWorkDir(const fs::path& path) {
    workDir = path;
}

void Plugin::Initialize() {
    std::lock_guard lock(mutex);
    if (isInitialized) {
        return;
    }

    InitializeLogger();
    InitializeConfig();
    InitializeInput();
    InitializeUnlocker();
    isInitialized = true;

    LOG(logger, spdlog::level::info,
        "Plugin initialized successfully"
    );
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
        "Plugin uninitialized successfully"
    );
} catch (const std::exception& e) {
    LOG(logger, spdlog::level::critical,
        "Failed to uninitialize Plugin: {}", e.what()
    );
}

void Plugin::InitializeLogger() {
#ifdef ENABLE_LOGGING
    std::string filename = (workDir / "log.txt").string();
    logger = spdlog::basic_logger_st("plugin", filename, true);
    logger->set_level(spdlog::level::trace);
    logger->flush_on(spdlog::level::trace);

    filename = (workDir / "values.csv").string();
    auto sink = std::make_shared<TimeBufferedFileSinkMT>(filename, 10000);
    csvLogger = std::make_shared<spdlog::logger>("csvLogger", sink);
    csvLogger->set_pattern("%v");
    csvLogger->set_level(spdlog::level::trace);
    csvLogger->flush_on(spdlog::level::off);

    csvLogger->info("time,camera,fov");
    csvLogger->flush();
    start = std::chrono::steady_clock::now();
#endif
}

void Plugin::InitializeConfig() {
    if (const fs::path path = workDir / "fov_config.json";
        fs::exists(path)) {
        try {
            config.FromJson(path);
            LOG(logger, spdlog::level::info,
                "Config loaded successfully"
            );
        } catch (const std::exception& e) {
            LOG(logger, spdlog::level::err,
                "Failed to load Config: {}", e.what()
            );
            LOG(logger, spdlog::level::info,
                "Using plugin defaults"
            );
        }
    } else {
        try {
            LOG(logger, spdlog::level::info,
                "Config file not found, creating default Config"
            );
            config.ToJson(path);
            LOG(logger, spdlog::level::info,
                "Default Config created successfully"
            );
        } catch (const std::exception& e) {
            LOG(logger, spdlog::level::err,
                "Failed to create default Config: {}", e.what()
            );
            LOG(logger, spdlog::level::info,
                "Using plugin defaults"
            );
        }
    }
}

void Plugin::InitializeInput() try {
    const DWORD tracked = GetCurrentProcessId();
    input.SetTrackedProcess(tracked);
    input.RegisterKeys({
        config.hookKey,
        config.enableKey,
        config.nextKey,
        config.prevKey,
        config.dumpKey
    });
    input.RegisterOnKeyDown([this](const int vKey) {
        this->OnKeyDown(vKey);
    });
    input.StartPolling(30);
    LOG(logger, spdlog::level::info,
        "Input initialized successfully"
    );
} catch (const std::exception& e) {
    LOG(logger, spdlog::level::critical,
        "Failed to initialize Input: {}", e.what()
    );
    throw;
}

void Plugin::InitializeUnlocker() try {
    const auto module = reinterpret_cast<uintptr_t>(GetModuleHandle(nullptr));
    const auto isGlobal = GetModuleHandle("GenshinImpact.exe") != nullptr;
    const uintptr_t offset = isGlobal ? 0x1136f30 : 0x1136d30;
    const auto target = reinterpret_cast<void*>(module + offset);

    hook.Initialize();
    hook.Create(target, &HkSetFov);
    filter.SetTimeConstant(config.smoothing);
    LOG(logger, spdlog::level::info,
        "Unlocker initialized successfully"
    );
} catch (const std::exception& e) {
    LOG(logger, spdlog::level::critical,
        "Failed to initialize Unlocker: {}", e.what()
    );
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
    } else if (vKey == config.dumpKey) {
        FLUSH(csvLogger);
    }
} catch (const std::exception& e) {
    LOG(logger, spdlog::level::err,
        "Failed to process OnKeyDown: {}", e.what()
    );
}

void Plugin::FilterAndSetFov(void* instance, float value) try {
    if (!config.interpolate) {
        if (value == 45.0 && config.enabled) {
            value = static_cast<float>(config.fov);
        }
        hook.CallOriginal(instance, value);
        return;
    }

    ++setFovCount;

    if (instance == previousInstance &&
        value == previousValue) {
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
        value = std::bit_cast<float>(rep + 1);
        previousInstance = instance;
        previousValue = value;
    }

#ifdef ENABLE_LOGGING
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<
        std::chrono::duration<double> // in seconds
    >(now - start).count();
    LOG(csvLogger, spdlog::level::trace,
        "{:.9f},{},{:.9f}", elapsed, instance, value
    );
#endif

    hook.CallOriginal(instance, value);
} catch (const std::exception& e) {
    LOG(logger, spdlog::level::err,
        "Failed to set FOV: {}", e.what()
    );
}

void Plugin::HkSetFov(void* instance, float value) {
    Plugin& plugin = GetInstance();
    std::lock_guard lock(plugin.mutex);
    if (plugin.hook.IsEnabled()) {
        plugin.FilterAndSetFov(instance, value);
    }
}
