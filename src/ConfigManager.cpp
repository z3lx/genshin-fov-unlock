#include "ConfigManager.h"
#include "Events.h"
#include "FileHandler.h"
#include "IComponent.h"
#include "IMediator.h"

#include <nlohmann/json.hpp>

#include <algorithm>
#include <exception>
#include <memory>
#include <ranges>
#include <utility>

constexpr auto CONFIG_FILENAME = "fps_config.json";

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
    : IComponent(mediator), fileHandler(std::move(fileHandler)) { }

ConfigManager::~ConfigManager() noexcept = default;

void ConfigManager::Load() try {
    const auto mediator = weakMediator.lock();
    if (!mediator) {
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
        // TODO: LOG ERROR
        config = {};
        Save();
    }

    // TODO: VALIDATIONS

    mediator->Notify(OnCreateToggle { created });
    mediator->Notify(OnEnableToggle { enabled });
    mediator->Notify(OnFovChange { fov });
    mediator->Notify(OnSmoothingChange { smoothing });
    mediator->Notify(OnKeyBindChange {
        { createKey, enableKey, nextKey, prevKey, dumpKey }
    });
} catch (const std::exception& e) {
    // TODO: LOG ERROR
}

void ConfigManager::Save() try {
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
} catch (const std::exception& e) {
    // TODO: LOG ERROR
}

void ConfigManager::Handle(const Event& event) {
   std::visit(Visitor { *this }, event);
}

template <typename T>
void ConfigManager::Visitor::operator()(const T& event) const { }

template <>
void ConfigManager::Visitor::operator()(const OnKeyDown& event) const {
    const auto mediator = m.weakMediator.lock();
    if (!mediator) {
        return;
    }

    auto& [created, enabled, fov, fovPresets, smoothing,
        createKey, enableKey, nextKey, prevKey, dumpKey] = m.config;

    if (event.vKey == createKey) {
        // TODO: FIX TOGGLE
        created = true;
        mediator->Notify(OnCreateToggle { true });
    } else if (event.vKey == enableKey) {
        enabled = !enabled;
        mediator->Notify(OnEnableToggle { enabled });
    }

    if (!created || !enabled) {
        return;
    }

    if (event.vKey == nextKey) {
        const auto it = std::ranges::find_if(
            fovPresets,
            [&](const int fovPreset) { return fov < fovPreset; }
        );
        fov = it != fovPresets.end() ? *it : fovPresets.front();
        mediator->Notify(OnFovChange { fov });
    } else if (event.vKey == prevKey) {
        const auto it = std::ranges::find_if(
            fovPresets | std::views::reverse,
            [&](const int fovPreset) { return fov > fovPreset; }
        );
        fov = it != fovPresets.rend() ? *it : fovPresets.back();
        mediator->Notify(OnFovChange { fov });
    }
}
