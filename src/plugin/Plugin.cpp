#include "plugin/Plugin.h"
#include "plugin/ConfigManager.h"
#include "plugin/InputManager.h"
#include "plugin/Unlocker.h"
#include "utils/FileHandler.h"
#include "utils/log/Logger.h"
#include "utils/log/sinks/FileSink.h"

#include <exception>
#include <filesystem>
#include <memory>
#include <utility>

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

    auto unlocker = std::make_unique<Unlocker>(plugin);
    plugin->components.push_back(std::move(unlocker));

    auto input = std::make_unique<InputManager>(plugin);
    input->SetTrackedProcess(0);
    input->StartPolling(30);
    plugin->components.push_back(std::move(input));

    auto fileHandler = std::make_unique<FileHandler>();
    fileHandler->SetWorkingDirectory(GetPath());
    auto config = std::make_unique<ConfigManager>(plugin, fileHandler);
    config->Load();
    plugin->components.push_back(std::move(config));

    LOG_I("Plugin initialized");
} catch (const std::exception& e) {
    LOG_F("Failed to initialize plugin: {}", e.what());
    Uninitialize();
}

void Plugin::Uninitialize() try {
    LOG_D("Uninitializing plugin");
    plugin = nullptr;
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

Plugin::Plugin() = default;
Plugin::~Plugin() = default;
