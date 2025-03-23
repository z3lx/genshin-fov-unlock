#include "plugin/components/ConfigManager.hpp"
#include "plugin/Events.hpp"
#include "plugin/interfaces/IComponent.hpp"
#include "plugin/interfaces/IMediator.hpp"
#include "utils/log/Logger.hpp"

#include <nlohmann/json.hpp>

#include <algorithm>
#include <exception>
#include <filesystem>
#include <fstream>
#include <memory>
#include <ranges>
#include <utility>

namespace {
constexpr auto ENABLED = "enabled";
constexpr auto FOV = "fov";
constexpr auto FOV_PRESETS = "fov_presets";
constexpr auto SMOOTHING = "smoothing";
constexpr auto ENABLE_KEY = "enable_key";
constexpr auto NEXT_KEY = "next_key";
constexpr auto PREV_KEY = "prev_key";
constexpr auto DUMP_KEY = "dump_key";
} // namespace

ConfigManager::ConfigManager(
    std::weak_ptr<IMediator<Event>> mediator,
    std::filesystem::path filePath) noexcept
    : IComponent { std::move(mediator) }
    , filePath { std::move(filePath) } {}

ConfigManager::~ConfigManager() noexcept = default;

#define TRY_GET_TO(json, key, value)                                            \
    try {                                                                       \
        (json).at(key).get_to(value);                                           \
    } catch (const std::exception& e) {                                         \
        LOG_W("Failed to parse key '{}': {}", key, e.what());                   \
    }

#define TRY_GET_TO_IF(json, key, value, condition)                              \
    try {                                                                       \
        auto previousValue = value;                                             \
        (json).at(key).get_to(value);                                           \
        if (!(condition)) {                                                     \
            (value) = previousValue;                                            \
            LOG_W(                                                              \
                "Invalid value for key '{}': {} is not satisfied",              \
                key, #condition                                                 \
            );                                                                  \
        }                                                                       \
    } catch (const std::exception& e) {                                         \
        LOG_W("Failed to parse key '{}': {}", key, e.what());                   \
    }

void ConfigManager::Load() try {
    LOG_D("Loading config from {}", filePath.string());
    const auto mediator = GetMediator().lock();
    if (!mediator) {
        LOG_E("Mediator is expired");
        return;
    }

    auto& [enabled, fov, fovPresets, smoothing,
        enableKey, nextKey, prevKey, dumpKey] = config;

    try {
        std::ifstream file { filePath };
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open file");
        }
        nlohmann::ordered_json j;
        file >> j;

        TRY_GET_TO(j, ENABLED, enabled);
        TRY_GET_TO_IF(j, FOV, fov,
            fov > 0 && fov < 180);
        TRY_GET_TO_IF(j, FOV_PRESETS, fovPresets,
            std::ranges::all_of(fovPresets, [](const int fovPreset) {
                return fovPreset > 0 && fovPreset < 180;
            }));
        TRY_GET_TO_IF(j, SMOOTHING, smoothing,
            smoothing >= 0.0 && smoothing <= 1.0);
        TRY_GET_TO_IF(j, ENABLE_KEY, enableKey,
            enableKey > 0 && enableKey < 255);
        TRY_GET_TO_IF(j, NEXT_KEY, nextKey,
            nextKey > 0 && nextKey < 255);
        TRY_GET_TO_IF(j, PREV_KEY, prevKey,
            prevKey > 0 && prevKey < 255);
        TRY_GET_TO_IF(j, DUMP_KEY, dumpKey,
            dumpKey > 0 && dumpKey < 255);
    } catch (const std::exception& e) {
        LOG_E("Failed to parse config: {}", e.what());
        LOG_I("Using default config values");
        config = {};
    }

    std::ranges::sort(fovPresets);
    const auto last = std::ranges::unique(fovPresets).begin();
    fovPresets.erase(last, fovPresets.end());

    mediator->Notify(OnEnableToggle { enabled });
    mediator->Notify(OnFovChange { fov });
    mediator->Notify(OnSmoothingChange { smoothing });
    LOG_I("Config loaded");
} catch (const std::exception& e) {
    LOG_F("Failed to load config: {}", e.what());
    throw;
}

void ConfigManager::Save() try {
    LOG_D("Saving config to {}", filePath.string());

    auto& [enabled, fov, fovPresets, smoothing,
        enableKey, nextKey, prevKey, dumpKey] = config;

    const nlohmann::ordered_json j {
        { ENABLED, enabled },
        { FOV, fov },
        { FOV_PRESETS, fovPresets },
        { SMOOTHING, smoothing },
        { ENABLE_KEY, enableKey },
        { NEXT_KEY, nextKey },
        { PREV_KEY, prevKey },
        { DUMP_KEY, dumpKey }
    };

    std::ofstream file { filePath };
    if (!file.is_open()) {
        throw std::runtime_error { "Failed to open file" };
    }
    file << j.dump(4);
    LOG_I("Config saved");
} catch (const std::exception& e) {
    LOG_E("Failed to save config: {}", e.what());
    throw;
}

namespace {
struct Visitor {
    ConfigManager& instance;

    void operator()(const OnPluginStart& event) const;
    void operator()(const OnPluginEnd& event) const;
    void operator()(const OnKeyDown& event) const;
    void operator()(const OnCursorVisibilityChange& event) const;
    template <typename T> void operator()(const T& event) const;
};
} // namespace

void ConfigManager::Handle(const Event& event) noexcept {
   std::visit(Visitor { *this }, event);
}

void Visitor::operator()(const OnPluginStart& event) const try {
    instance.Load();
} catch (const std::exception& e) {
    LOG_E("Failed to handle OnPluginStart event: {}", e.what());
}

void Visitor::operator()(const OnPluginEnd& event) const try {
    instance.Save();
} catch (const std::exception& e) {
    LOG_E("Failed to handle OnPluginEnd event: {}", e.what());
}

void Visitor::operator()(const OnKeyDown& event) const try {
    const auto mediator = instance.GetMediator().lock();
    if (!mediator) {
        LOG_E("Mediator is expired");
        return;
    }

    auto& [enabled, fov, fovPresets, smoothing,
        enableKey, nextKey, prevKey, dumpKey] = instance.config;

    if (!instance.hooked) { // TODO: Refactor
        return;
    }

    if (event.vKey == enableKey) {
        const auto value = !enabled;
        mediator->Notify(OnEnableToggle { .enabled = value });
        enabled = value;
    } else if (!enabled) {
        return;
    } else if (event.vKey == nextKey) {
        const auto it = std::ranges::find_if(
            fovPresets,
            [&](const int fovPreset) { return fov < fovPreset; }
        );
        const auto value = it != fovPresets.end() ? *it : fovPresets.front();
        mediator->Notify(OnFovChange { .fov = value });
        fov = value;
    } else if (event.vKey == prevKey) {
        const auto it = std::ranges::find_if(
            fovPresets | std::views::reverse,
            [&](const int fovPreset) { return fov > fovPreset; }
        );
        const auto value = it != fovPresets.rend() ? *it : fovPresets.back();
        mediator->Notify(OnFovChange { .fov = value });
        fov = value;
    } else if (event.vKey == dumpKey) {
        mediator->Notify(OnDumpBuffer {});
    }
} catch (const std::exception& e) {
    LOG_E("Failed to handle OnKeyDown event: {}", e.what());
}

void Visitor::operator()(const OnCursorVisibilityChange& event) const try {
    // TODO: Synchronize
    instance.hooked = !event.isCursorVisible;
    if (const auto mediator = instance.GetMediator().lock()) {
        mediator->Notify(OnHookToggle { instance.hooked });
    }
} catch (const std::exception& e) {
    LOG_E("Failed to handle OnCursorVisibilityChange event: {}", e.what());
}

template <typename T>
void Visitor::operator()(const T& event) const {}
