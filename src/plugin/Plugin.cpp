#include "plugin/Plugin.hpp"
#include "plugin/ConfigManager.hpp"
#include "plugin/InputManager.hpp"
#include "plugin/Unlocker.hpp"
#include "utils/log/Logger.hpp"
#include "utils/log/sinks/FileSink.hpp"

#include <exception>
#include <filesystem>
#include <memory>
#include <thread>
#include <tuple>
#include <utility>
#include <vector>

#include <Windows.h>

std::shared_ptr<Plugin> Plugin::plugin = nullptr;

void Plugin::Initialize() try {
    if (plugin) {
        return;
    }

    LOG_D("Initializing plugin");
    LOG_D("Working directory: {}", GetPath().string());
    plugin = std::shared_ptr<Plugin>(new Plugin());

    LOG_SET_LEVEL(Level::Trace);
    LOG_SET_SINKS(
        std::make_unique<FileSink>(GetPath() / "logs.txt", true)
    );

    plugin->components.push_back(
        std::make_unique<Unlocker>(plugin)
    );

    plugin->components.push_back(
        std::make_unique<InputManager>(plugin, GetWindows())
    );

    plugin->components.push_back(
        std::make_unique<ConfigManager>(plugin, GetPath() / "fov_config.json")
    );

    plugin->Notify(OnPluginInitialize {});
    LOG_I("Plugin initialized");
} catch (const std::exception& e) {
    LOG_F("Failed to initialize plugin: {}", e.what());
    Uninitialize();
}

void Plugin::Uninitialize() try {
    LOG_D("Uninitializing plugin");
    if (plugin) {
        plugin->Notify(OnPluginUninitialize {});
        plugin = nullptr;
    }
    LOG_I("Plugin uninitialized");
} catch (const std::exception& e) {
    LOG_F("Failed to uninitialize plugin: {}", e.what());
}

namespace fs = std::filesystem;

fs::path Plugin::GetPath() {
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
    path = fs::path(buffer).parent_path();
    return path;
}

std::vector<HWND> Plugin::GetWindows() {
    DWORD targetProcessId = GetCurrentProcessId();
    std::vector<HWND> targetWindows {};
    std::tuple params = std::tie(targetProcessId, targetWindows);

    auto callback = [](HWND hwnd, LPARAM lParam) -> BOOL {
        auto& [targetProcessId, targetWindows] = *reinterpret_cast<
            std::tuple<DWORD&, std::vector<HWND>&>*>(lParam);

        DWORD processId;
        GetWindowThreadProcessId(hwnd, &processId);
        if (processId == targetProcessId) {
            targetWindows.push_back(hwnd);
        }
        return TRUE;
    };

    while (targetWindows.empty()) {
        EnumWindows(callback, reinterpret_cast<LPARAM>(&params));
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    return targetWindows;
}

Plugin::Plugin() = default;
Plugin::~Plugin() = default;
