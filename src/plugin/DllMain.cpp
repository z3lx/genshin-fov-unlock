#include "plugin/Plugin.hpp"
#include "utils/Windows.hpp"
#include "utils/log/Logger.hpp"
#include "utils/log/sinks/FileSink.hpp"

#include <exception>
#include <memory>
#include <thread>

#include <Windows.h>

namespace {
std::unique_ptr<Plugin> plugin {};

void Initialize() noexcept {
    const auto init = []() {
        try {
            const auto workingDirectory = GetModulePath().parent_path();
            LOG_SET_LEVEL(Level::Trace);
            LOG_SET_SINKS(std::make_unique<FileSink>(
                workingDirectory / "logs.txt", true));
            // LOG_D("Working directory: {}", workingDirectory.string());

            plugin = std::make_unique<Plugin>();
        } catch (const std::exception& e) {
            LOG_F("Failed to initialize plugin: {}", e.what());
        }
    };

    try {
        std::thread { init }.detach();
    } catch (const std::exception& e) {
        LOG_F("Failed to create thread: {}", e.what());
    }
}

void Uninitialize() noexcept {
    plugin = nullptr;
}
} // namespace

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved) {
    DisableThreadLibraryCalls(hinstDLL);

    switch (fdwReason) {
        case DLL_PROCESS_ATTACH: Initialize(); break;
        case DLL_PROCESS_DETACH: Uninitialize(); break;
        default: break;
    }

    return TRUE;
}
