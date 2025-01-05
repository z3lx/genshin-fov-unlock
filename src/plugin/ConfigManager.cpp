#include "plugin/ConfigManager.hpp"
#include "plugin/Events.hpp"
#include "plugin/IComponent.hpp"
#include "plugin/IMediator.hpp"
#include "utils/log/Logger.hpp"

#include <nlohmann/json.hpp>

#include <algorithm>
#include <exception>
#include <filesystem>
#include <fstream>
#include <memory>
#include <ranges>

constexpr auto ENABLED = "enabled";
constexpr auto FOV = "fov";
constexpr auto FOV_PRESETS = "fov_presets";
constexpr auto SMOOTHING = "smoothing";
constexpr auto HOOK_KEY = "hook_key";
constexpr auto ENABLE_KEY = "enable_key";
constexpr auto NEXT_KEY = "next_key";
constexpr auto PREV_KEY = "prev_key";
constexpr auto DUMP_KEY = "dump_key";

ConfigManager::ConfigManager(
    const std::weak_ptr<IMediator<Event>>& mediator,
    const std::filesystem::path& filePath) noexcept
    : IComponent(mediator), filePath(filePath) { }

ConfigManager::~ConfigManager() noexcept = default;

void ConfigManager::Load() try {
    LOG_D("Loading config from {}", filePath.string());
    const auto mediator = weakMediator.lock();
    if (!mediator) {
        LOG_E("Mediator is expired");
        return;
    }

    auto& [hooked, enabled, fov, fovPresets, smoothing,
        hookedKey, enableKey, nextKey, prevKey, dumpKey] = config;

    try {
        std::ifstream file { filePath };
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open file");
        }
        nlohmann::ordered_json j;
        file >> j;

        j.at(ENABLED).get_to(enabled);
        j.at(FOV).get_to(fov);
        j.at(FOV_PRESETS).get_to(fovPresets);
        j.at(SMOOTHING).get_to(smoothing);
        j.at(HOOK_KEY).get_to(hookedKey);
        j.at(ENABLE_KEY).get_to(enableKey);
        j.at(NEXT_KEY).get_to(nextKey);
        j.at(PREV_KEY).get_to(prevKey);
        j.at(DUMP_KEY).get_to(dumpKey);
    } catch (const std::exception& e) {
        LOG_E("Failed to parse config: {}", e.what());
        LOG_I("Using default config values");
        config = {};
    }

    // TODO: VALIDATIONS

    mediator->Notify(OnHookToggle { hooked });
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

    auto& [hooked, enabled, fov, fovPresets, smoothing,
        hookKey, enableKey, nextKey, prevKey, dumpKey] = config;

    const nlohmann::ordered_json j {
        { ENABLED, enabled },
        { FOV, fov },
        { FOV_PRESETS, fovPresets },
        { SMOOTHING, smoothing },
        { HOOK_KEY, hookKey },
        { ENABLE_KEY, enableKey },
        { NEXT_KEY, nextKey },
        { PREV_KEY, prevKey },
        { DUMP_KEY, dumpKey }
    };

    std::ofstream file { filePath };
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file");
    }
    file << j.dump(4);
    LOG_I("Config saved");
} catch (const std::exception& e) {
    LOG_E("Failed to save config: {}", e.what());
    throw;
}

void ConfigManager::Handle(const Event& event) {
   std::visit(Visitor { *this }, event);
}

template <typename T>
void ConfigManager::Visitor::operator()(const T& event) const { }

template <>
void ConfigManager::Visitor::operator()(const OnPluginInitialize& event) const try {
    LOG_D("Handling OnPluginInitialize event");
    m.Load();
    LOG_D("OnPluginInitialize event handled");
} catch (const std::exception& e) {
    LOG_E("Failed to handle OnPluginInitialize event: {}", e.what());
    throw;
}

template <>
void ConfigManager::Visitor::operator()(const OnPluginUninitialize& event) const try {
    LOG_D("Handling OnPluginUninitialize event");
    m.Save();
    LOG_D("OnPluginUninitialize event handled");
} catch (const std::exception& e) {
    LOG_E("Failed to handle OnPluginUninitialize event: {}", e.what());
    throw;
}

template <>
void ConfigManager::Visitor::operator()(const OnKeyDown& event) const try {
    LOG_D("Handling OnKeyDown event with vKey = {}", event.vKey);
    const auto mediator = m.weakMediator.lock();
    if (!mediator) {
        LOG_E("Mediator is expired");
        return;
    }

    auto& [hooked, enabled, fov, fovPresets, smoothing,
        hookKey, enableKey, nextKey, prevKey, dumpKey] = m.config;

    if (event.vKey == hookKey) {
        const auto value = !hooked;
        mediator->Notify(OnHookToggle { .hooked = value });
        hooked = value;
    } else if (!hooked) {
        LOG_D("Event ignored due to unhooked state");
        return;
    } else if (event.vKey == enableKey) {
        const auto value = !enabled;
        mediator->Notify(OnEnableToggle { .enabled = value });
        enabled = value;
    } else if (!enabled) {
        LOG_D("Event ignored due to disabled state");
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
    } else {
        LOG_D("Event ignored due to unknown vKey");
        return;
    }

    LOG_D("OnKeyDown event handled");
} catch (const std::exception& e) {
    LOG_E("Failed to handle OnKeyDown event: {}", e.what());
    throw;
}
