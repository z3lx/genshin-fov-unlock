#pragma once

#include "plugin/Events.h"
#include "plugin/IMediator.h"

#include <filesystem>
#include <memory>
#include <vector>

#include <Windows.h>

class Plugin final :
    public IMediator<Event>,
    std::enable_shared_from_this<Plugin> {
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
