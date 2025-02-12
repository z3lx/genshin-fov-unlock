#pragma once

#include "plugin/Events.hpp"
#include "plugin/interfaces/IMediator.hpp"

#include <filesystem>
#include <memory>
#include <unordered_set>

#include <Windows.h>

class Plugin final : public IMediator<Event> {
public:
    static void Initialize();
    static void Uninitialize();
    ~Plugin() override;

private:
    Plugin();
    static std::filesystem::path GetPath();
    static std::unordered_set<HWND> GetWindows();
    static std::shared_ptr<Plugin> plugin;
};
