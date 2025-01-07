#pragma once

#include "plugin/Events.hpp"
#include "plugin/interfaces/IMediator.hpp"

#include <filesystem>
#include <memory>
#include <vector>

#include <Windows.h>

class Plugin final : public IMediator<Event> {
public:
    static void Initialize();
    static void Uninitialize();
    ~Plugin() override;

private:
    Plugin();
    static std::filesystem::path GetPath();
    static std::vector<HWND> GetWindows();
    static std::shared_ptr<Plugin> plugin;
};
