#include "plugin/Plugin.hpp"
#include "plugin/Events.hpp"
#include "plugin/components/ConfigManager.hpp"
#include "plugin/components/KeyboardObserver.hpp"
#include "plugin/components/MouseObserver.hpp"
#include "plugin/components/Unlocker.hpp"
#include "plugin/components/WinEventNotifier.hpp"
#include "utils/Windows.hpp"
#include "utils/log/Logger.hpp"

#include <chrono>
#include <exception>
#include <memory>
#include <mutex>
#include <ranges>
#include <thread>
#include <tuple>
#include <utility>
#include <variant>

#include <Windows.h>

std::shared_ptr<Plugin> Plugin::MakePlugin() {
    auto plugin = std::shared_ptr<Plugin>(new Plugin {});
    plugin->keyboardObserver = std::make_unique<KeyboardObserver>(plugin);
    plugin->mouseObserver = std::make_unique<MouseObserver>(plugin);
    // plugin->winEventNotifier = std::make_unique<WinEventNotifier>(plugin);
    plugin->configManager = std::make_unique<ConfigManager>(
        plugin, GetModulePath().parent_path() / "fov_config.json");
    plugin->unlocker = std::make_unique<Unlocker>(plugin);

    plugin->targetWindows = GetProcessWindows();
    plugin->Notify(OnPluginStart {});
    return plugin;
}

Plugin::Plugin()
    : isUnlockerHooked { false }
    , isWindowFocused { true }
    , isCursorVisible { true } {
    // Construction delegated to MakePlugin
};

Plugin::~Plugin() {
    Notify(OnPluginEnd {});
}

template <>
void Plugin::Handle(const OnPluginStart& event) noexcept {
    keyboardObserver->SetEnabled(true);
    mouseObserver->SetEnabled(true);
    // winEventNotifier->SetEnabled(true);

    try {
        config = configManager->Read();
    } catch (const std::exception& e) {
        LOG_W("Failed to read config: {}", e.what());
    }

    unlocker->SetEnable(config.enabled);
    unlocker->SetFieldOfView(config.fov);
    unlocker->SetSmoothing(config.smoothing);
}

template <>
void Plugin::Handle(const OnPluginEnd& event) noexcept {
    keyboardObserver->SetEnabled(false);
    mouseObserver->SetEnabled(false);
    // winEventNotifier->SetEnabled(false);

    try {
        configManager->Write(config);
    } catch (const std::exception& e) {
        LOG_W("Failed to write config: {}", e.what());
    }
}

template <>
void Plugin::Handle(const OnKeyDown& event) noexcept {
    const auto key = event.vKey;
    auto& [enabled, fov, fovPresets, smoothing,
        enableKey, nextKey, prevKey, dumpKey] = config;

    if (!isUnlockerHooked) {
        return;
    }

    if (key == enableKey) {
        enabled = !enabled;
        unlocker->SetEnable(enabled);
    } else if (!enabled) {
        return;
    } else if (key == nextKey) {
        const auto it = std::ranges::find_if(
            fovPresets,
            [fov](const int fovPreset) { return fov < fovPreset; });
        fov = it != fovPresets.end() ? *it : fovPresets.front();
        unlocker->SetFieldOfView(fov);
    } else if (key == prevKey) {
        const auto it = std::ranges::find_if(
            fovPresets | std::views::reverse,
            [fov](const int fovPreset) { return fov > fovPreset; });
        fov = it != fovPresets.rend() ? *it : fovPresets.back();
        unlocker->SetFieldOfView(fov);
    } else if (key == dumpKey) {
        // TODO: plugin.unlocker.DumpBuffer();
    }
}

template <>
void Plugin::Handle(const OnCursorVisibilityChange& event) noexcept {
    isCursorVisible = event.isCursorVisible;
    const HWND foregroundWindow = GetForegroundWindow();
    isWindowFocused = std::ranges::contains(targetWindows, foregroundWindow);
    ConsumeState();
}

// template <>
// void Plugin::Handle(const OnForegroundWindowChange& event) noexcept {
//     isWindowFocused = std::ranges::contains(targetWindows, event.hwnd);
//     ConsumeState();
// }

template <typename Event>
void Plugin::Handle(const Event& event) noexcept {};

void Plugin::ConsumeState() noexcept try {
    if (const bool value = isWindowFocused && !isCursorVisible;
        isUnlockerHooked != value) {
        unlocker->SetHook(value);
        isUnlockerHooked = value;
    }
} catch (const std::exception& e) {
    LOG_E("Failed to set hook: {}", e.what());
}

struct Plugin::Visitor {
    Plugin& plugin;

    template <typename Event>
    void operator()(const Event& event) const noexcept;
};

template <typename Event>
void Plugin::Visitor::operator()(const Event& event) const noexcept {
    plugin.Handle(event);
}

void Plugin::Notify(const Event& event) noexcept {
    std::lock_guard lock { mutex };
    std::visit(Visitor { *this }, event);
}
