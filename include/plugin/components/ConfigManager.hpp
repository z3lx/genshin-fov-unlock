#pragma once

#include "plugin/Events.hpp"
#include "plugin/interfaces/IComponent.hpp"
#include "plugin/interfaces/IMediator.hpp"

#include <filesystem>
#include <memory>
#include <vector>

#include <Windows.h>

struct Config {
    bool enabled = true;
    int fov = 75;
    std::vector<int> fovPresets {
        30, 45, 60, 75, 90, 110
    };
    double smoothing = 0.125;

    int enableKey = VK_DOWN;
    int nextKey = VK_RIGHT;
    int prevKey = VK_LEFT;
    int dumpKey = VK_F12;
};

class ConfigManager final : public IComponent<Event> {
public:
    explicit ConfigManager(
        std::weak_ptr<IMediator<Event>> mediator,
        std::filesystem::path filePath = "fov_config.json") noexcept;
    ~ConfigManager() noexcept override;

    // TODO: Add file watcher
    [[nodiscard]] Config Read() const;
    void Write(const Config& config) const;

private:
    std::filesystem::path filePath;
};
