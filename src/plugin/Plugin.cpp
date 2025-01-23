#include "plugin/Plugin.hpp"
#include "plugin/components/ConfigManager.hpp"
#include "plugin/components/InputManager.hpp"
#include "plugin/components/Unlocker.hpp"
#include "utils/Windows.hpp"
#include "utils/log/Logger.hpp"
#include "utils/log/sinks/FileSink.hpp"

#include <chrono>
#include <exception>
#include <filesystem>
#include <memory>
#include <thread>
#include <tuple>
#include <utility>
#include <vector>

#include <Windows.h>

namespace sc = std::chrono;

std::shared_ptr<Plugin> Plugin::plugin = nullptr;

void Plugin::Initialize() try {
    if (plugin) {
        return;
    }

    LOG_SET_LEVEL(Level::Trace);
    LOG_SET_SINKS(std::make_unique<FileSink>(GetPath() / "logs.txt", true));
    LOG_D("Initializing plugin");
    LOG_D("Working directory: {}", GetPath().string());

    plugin = std::shared_ptr<Plugin>(new Plugin());
    plugin->AddComponents(
        std::make_unique<InputManager>(plugin, GetWindows(sc::seconds(30))),
        std::make_unique<ConfigManager>(plugin, GetPath() / "fov_config.json"),
        std::make_unique<Unlocker>(plugin)
    );
    plugin->Notify(OnPluginStart {});

    LOG_I("Plugin initialized");
} catch (const std::exception& e) {
    LOG_F("Failed to initialize plugin: {}", e.what());
    Uninitialize();
}

void Plugin::Uninitialize() try {
    LOG_D("Uninitializing plugin");
    if (plugin) {
        plugin->Notify(OnPluginEnd {});
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
    ThrowOnSystemError(GetModuleHandleExA(
        GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
        GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
        static_cast<LPCTSTR>(reinterpret_cast<void*>(GetPath)),
        &module
    ));

    constexpr auto bufferSize = 1024;
    wchar_t buffer[bufferSize];
    ThrowOnSystemError(GetModuleFileNameW(
        module, buffer, bufferSize
    ));
    path = fs::path(buffer).parent_path();
    return path;
}

std::vector<HWND> Plugin::GetWindows(const sc::milliseconds timeout) {
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

    const auto start = sc::steady_clock::now();
    while (targetWindows.empty()) {
        EnumWindows(callback, reinterpret_cast<LPARAM>(&params));
        if (sc::steady_clock::now() - start > timeout) {
            throw std::runtime_error("Failed to find target windows");
        }
        std::this_thread::sleep_for(sc::seconds(1));
    }
    return targetWindows;
}

Plugin::Plugin() = default;
Plugin::~Plugin() = default;
