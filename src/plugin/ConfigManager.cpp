#include "plugin/ConfigManager.h"
#include "plugin/Events.h"
#include "plugin/IComponent.h"
#include "plugin/IMediator.h"
#include "utils/FileHandler.h"
#include "utils/log/Logger.h"

#include <nlohmann/json.hpp>

#include <algorithm>
#include <exception>
#include <memory>
#include <ranges>
#include <utility>

constexpr auto CONFIG_FILENAME = "fov_config.json";

constexpr auto ENABLED = "enabled";
constexpr auto FOV = "fov";
constexpr auto FOV_PRESETS = "fov_presets";
constexpr auto SMOOTHING = "smoothing";
constexpr auto CREATE_KEY = "create_key";
constexpr auto ENABLE_KEY = "enable_key";
constexpr auto NEXT_KEY = "next_key";
constexpr auto PREV_KEY = "prev_key";
constexpr auto DUMP_KEY = "dump_key";

ConfigManager::ConfigManager(
    const std::weak_ptr<IMediator<Event>>& mediator,
    std::unique_ptr<FileHandler>& fileHandler) noexcept
    : IComponent(mediator), fileHandler(std::move(fileHandler)) {
    LOG_I("ConfigManager initialized");
}

ConfigManager::~ConfigManager() noexcept {
    LOG_I("ConfigManager uninitialized");
}

void ConfigManager::Load() try {
    LOG_D("Loading config from {}", CONFIG_FILENAME);
    const auto mediator = weakMediator.lock();
    if (!mediator) {
        LOG_E("Mediator is expired");
        return;
    }

    auto& [created, enabled, fov, fovPresets, smoothing,
        createKey, enableKey, nextKey, prevKey, dumpKey] = config;

    try {
        fileHandler->Open(CONFIG_FILENAME, false);
        const auto j = nlohmann::ordered_json::parse(fileHandler->Read());
        fileHandler->Close();

        j.at(ENABLED).get_to(enabled);
        j.at(FOV).get_to(fov);
        j.at(FOV_PRESETS).get_to(fovPresets);
        j.at(SMOOTHING).get_to(smoothing);
        j.at(CREATE_KEY).get_to(createKey);
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

    mediator->Notify(OnCreateToggle { created });
    mediator->Notify(OnEnableToggle { enabled });
    mediator->Notify(OnFovChange { fov });
    mediator->Notify(OnSmoothingChange { smoothing });
    LOG_I("Config loaded");
} catch (const std::exception& e) {
    LOG_F("Failed to load config: {}", e.what());
    throw;
}

void ConfigManager::Save() try {
    LOG_D("Saving config to {}", CONFIG_FILENAME);

    auto& [created, enabled, fov, fovPresets, smoothing,
        createKey, enableKey, nextKey, prevKey, dumpKey] = config;

    const nlohmann::ordered_json j {
        { ENABLED, enabled },
        { FOV, fov },
        { FOV_PRESETS, fovPresets },
        { SMOOTHING, smoothing },
        { CREATE_KEY, createKey },
        { ENABLE_KEY, enableKey },
        { NEXT_KEY, nextKey },
        { PREV_KEY, prevKey },
        { DUMP_KEY, dumpKey }
    };

    fileHandler->Open(CONFIG_FILENAME, true);
    fileHandler->Write(j.dump(4));
    fileHandler->Close();
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
void ConfigManager::Visitor::operator()(const OnKeyDown& event) const try {
    LOG_D("Handling OnKeyDown event with vKey = {}", event.vKey);
    const auto mediator = m.weakMediator.lock();
    if (!mediator) {
        LOG_E("Mediator is expired");
        return;
    }

    auto& [created, enabled, fov, fovPresets, smoothing,
        createKey, enableKey, nextKey, prevKey, dumpKey] = m.config;

    if (event.vKey == createKey) {
        const auto value = !created;
        mediator->Notify(OnCreateToggle { .created = value });
        created = value;
    } else if (event.vKey == enableKey) {
        const auto value = !enabled;
        mediator->Notify(OnEnableToggle { .enabled = value });
        enabled = value;
    } else if (event.vKey == dumpKey) {
        mediator->Notify(OnDumpBuffer {});
    } else if (!created || !enabled) {
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
    }

    LOG_D("OnKeyDown event handled");
} catch (const std::exception& e) {
    LOG_E("Failed to handle OnKeyDown event: {}", e.what());
    throw;
}
