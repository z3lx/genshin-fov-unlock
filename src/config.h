#pragma once

#include <windows.h>
#include <nlohmann/json.hpp>

#include <filesystem>
#include <vector>

struct Config {
    [[nodiscard]] nlohmann::ordered_json ToJson() const noexcept;
    void ToJson(const std::filesystem::path& path) const;
    void FromJson(const nlohmann::ordered_json& json);
    void FromJson(const std::filesystem::path& path);

    bool enabled = true;
    int fov = 75;
    std::vector<int> fovPresets = { 30, 45, 60, 75, 90, 110 };

    bool interpolate = true;
    double smoothing = 0.2;

    int hookKey = VK_UP;
    int enableKey = VK_DOWN;
    int nextKey = VK_RIGHT;
    int prevKey = VK_LEFT;
#ifdef ENABLE_LOGGING
    int dumpKey = VK_F12;
#endif
};
