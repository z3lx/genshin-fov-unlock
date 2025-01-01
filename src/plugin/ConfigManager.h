#pragma once

#include "plugin/Events.h"
#include "plugin/IComponent.h"
#include "plugin/IMediator.h"

#include <filesystem>
#include <memory>
#include <vector>

#include <Windows.h>

class ConfigManager final : public IComponent<Event> {
public:
    explicit ConfigManager(
        const std::weak_ptr<IMediator<Event>>& mediator,
        const std::filesystem::path& filePath = "fov_config.json"
    ) noexcept;
    ~ConfigManager() noexcept override;

    void Load();
    void Save();

    void Handle(const Event& event) override;

private:
    struct Visitor {
        ConfigManager& m;

        template <typename T>
        void operator()(const T& event) const;
    };

    struct Config {
        bool hooked = false;
        bool enabled = true;
        int fov = 75;
        std::vector<int> fovPresets = { 30, 45, 60, 75, 90, 110 };
        double smoothing = 0.2;

        int hookKey = VK_UP;
        int enableKey = VK_DOWN;
        int nextKey = VK_RIGHT;
        int prevKey = VK_LEFT;
        int dumpKey = VK_F12;
    };

    Config config;
    std::filesystem::path filePath;
};
