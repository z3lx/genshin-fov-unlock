#pragma once

#include "plugin/Events.hpp"
#include "plugin/interfaces/IComponent.hpp"

#include <cstdint>
#include <filesystem>
#include <vector>

#include <Windows.h>

struct Config {
    bool enabled = true;
    int fov = 75;
    std::vector<int> fovPresets {
        30, 45, 60, 75, 90, 110
    };
    float smoothing = 0.125;

    uint8_t enableKey = VK_DOWN;
    uint8_t nextKey = VK_RIGHT;
    uint8_t prevKey = VK_LEFT;
    uint8_t dumpKey = VK_F12;
};

class ConfigManager final : public IComponent<Event> {
public:
    explicit ConfigManager(
        std::filesystem::path filePath = "fov_config.json") noexcept;
    ~ConfigManager() noexcept override;

    // TODO: Add file watcher
    [[nodiscard]] Config Read() const;
    void Write(const Config& config) const;

private:
    std::filesystem::path filePath;
};
