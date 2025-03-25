#include "plugin/Plugin.hpp"
#include "plugin/Events.hpp"
#include "plugin/components/ConfigManager.hpp"
#include "plugin/components/KeyboardObserver.hpp"
#include "plugin/components/MouseObserver.hpp"
#include "plugin/components/Unlocker.hpp"
#include "plugin/components/WinEventNotifier.hpp"
#include "utils/log/Logger.hpp"
#include "utils/log/sinks/FileSink.hpp"

#include <chrono>
#include <exception>
#include <filesystem>
#include <iostream>
#include <memory>
#include <mutex>
#include <ranges>
#include <thread>
#include <tuple>
#include <utility>
#include <variant>

#include <Windows.h>

namespace fs = std::filesystem;

namespace {
void AllocateConsole() noexcept {
    if (!AllocConsole()) {
        return;
    }
    FILE* file {};
    freopen_s(&file, "CONOUT$", "w", stdout);
    freopen_s(&file, "CONOUT$", "w", stderr);
    freopen_s(&file, "CONIN$", "r", stdin);
    std::cout.clear();
    std::cerr.clear();
    std::cin.clear();
    std::wcout.clear();
    std::wcerr.clear();
    std::wcin.clear();
}

fs::path GetPath() {
    static fs::path path {};
    if (!path.empty()) {
        return path;
    }

    HMODULE module {};
    GetModuleHandleEx(
        GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
        GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
        static_cast<LPCTSTR>(reinterpret_cast<void*>(GetPath)),
        &module
    );

    char buffer[MAX_PATH];
    GetModuleFileName(module, buffer, MAX_PATH);
    path = fs::path { buffer }.parent_path();
    return path;
}

std::vector<HWND> GetWindows() {
    DWORD targetProcessId = GetCurrentProcessId();
    std::vector<HWND> targetWindows {};
    std::tuple params = std::tie(targetProcessId, targetWindows);

    const auto callback = [](HWND hwnd, LPARAM lParam) -> BOOL {
        auto& [targetProcessId, targetWindows] = *reinterpret_cast<
            std::tuple<DWORD&, std::vector<HWND>&>*>(lParam);

        DWORD processId;
        GetWindowThreadProcessId(hwnd, &processId);
        if (processId == targetProcessId &&
            !std::ranges::contains(targetWindows, hwnd)) {
            targetWindows.push_back(hwnd);
        }
        return TRUE;
    };

    while (targetWindows.empty()) {
        EnumWindows(callback, reinterpret_cast<LPARAM>(&params));
        std::this_thread::sleep_for(std::chrono::seconds { 1 });
    }
    return targetWindows;
}

std::shared_ptr<Plugin> plugin = nullptr;
} // namespace

void Plugin::Initialize() try {
    if (plugin) {
        return;
    }

    LOG_SET_LEVEL(Level::Trace);
    LOG_SET_SINKS(std::make_unique<FileSink>(GetPath() / "logs.txt", true));
    LOG_D("Working directory: {}", GetPath().string());

    plugin = std::shared_ptr<Plugin>(new Plugin {});
    auto path = GetPath() / "fov_config.json";
    plugin->keyboardObserver = std::make_unique<KeyboardObserver>(plugin);
    plugin->mouseObserver = std::make_unique<MouseObserver>(plugin);
    plugin->winEventNotifier = std::make_unique<WinEventNotifier>(plugin);
    plugin->configManager = std::make_unique<ConfigManager>(plugin, path);
    plugin->unlocker = std::make_unique<Unlocker>(plugin);

    plugin->targetWindows = GetWindows();
    plugin->Notify(OnPluginStart {});
} catch (const std::exception& e) {
    LOG_F("Failed to initialize plugin: {}", e.what());
    Uninitialize();
}

void Plugin::Uninitialize() try {
    if (plugin) {
        plugin->Notify(OnPluginEnd {});
        plugin = nullptr;
    }
} catch (const std::exception& e) {
    LOG_F("Failed to uninitialize plugin: {}", e.what());
}

Plugin::Plugin()
    : isUnlockerHooked { false }
    , isWindowFocused { true }
    , isCursorVisible { true } {};

Plugin::~Plugin() = default;

template <>
void Plugin::Handle(const OnPluginStart& event) noexcept {
    keyboardObserver->SetEnabled(true);
    mouseObserver->SetEnabled(true);
    winEventNotifier->SetEnabled(true);

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
    winEventNotifier->SetEnabled(false);

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
    ConsumeState();
}

template <>
void Plugin::Handle(const OnForegroundWindowChange& event) noexcept {
    isWindowFocused = std::ranges::contains(targetWindows, event.hwnd);
    ConsumeState();
}

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
