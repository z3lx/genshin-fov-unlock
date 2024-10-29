#pragma once

#include <windows.h>
#include <nlohmann/json.hpp>

#include <vector>

struct Config {
    bool enabled = true;
    int fov = 75;
    std::vector<int> fovPresets = { 30, 45, 60, 75, 90, 110 };

    bool interpolate = true;
    float smoothing = 0.2;
    float threshold = 4.0;

    int hookKey = VK_UP;
    int enableKey = VK_DOWN;
    int nextKey = VK_RIGHT;
    int prevKey = VK_LEFT;
};

void to_json(nlohmann::ordered_json& j, const Config& c);
void from_json(const nlohmann::ordered_json& j, Config& c);
