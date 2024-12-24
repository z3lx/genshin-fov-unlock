#include "Plugin.h"
#include "ConfigManager.h"
#include "FileHandler.h"
#include "InputManager.h"
#include "Unlocker.h"

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
    plugin = std::shared_ptr<Plugin>(new Plugin());

    auto unlocker = std::make_unique<Unlocker>(plugin);
    plugin->components.push_back(std::move(unlocker));

    // TODO: REPLACE WITH EVENTS
    auto input = std::make_unique<InputManager>(plugin);
    input->SetTrackedProcess(0);
    input->StartPolling(30);
    plugin->components.push_back(std::move(input));

    auto fileHandler = std::make_unique<FileHandler>();
    fileHandler->SetWorkingDirectory(GetPath());
    auto config = std::make_unique<ConfigManager>(plugin, fileHandler);
    config->Load();
    plugin->components.push_back(std::move(config));
} catch (const std::exception& e) {
    // TODO: LOG ERROR
    Uninitialize();
}

void Plugin::Uninitialize() {
    plugin = nullptr;
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
