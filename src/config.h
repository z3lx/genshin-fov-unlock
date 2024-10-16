#pragma once
#include <vector>
#include <windows.h>
#include <nlohmann/json.hpp>

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

void to_json(nlohmann::ordered_json& j, const Config& c) {
    j = nlohmann::ordered_json{
        { "enabled", c.enabled },
        { "fov", c.fov },
        { "fov_presets", c.fovPresets },
        { "interpolate", c.interpolate },
        { "smoothing", c.smoothing },
        { "threshold", c.threshold },
        { "hook_key", c.hookKey },
        { "enable_key", c.enableKey },
        { "next_key", c.nextKey },
        { "prev_key", c.prevKey }
    };
}

void from_json(const nlohmann::ordered_json& j, Config& c) try {
    j.at("enabled").get_to(c.enabled);

    int fov;
    j.at("fov").get_to(fov);
    if (fov >= 1 && fov <= 179) {
        c.fov = fov;
    }

    std::vector<int> presets;
    j.at("fov_presets").get_to(presets);
    std::ranges::sort(presets);
    c.fovPresets = presets;

    j.at("interpolate").get_to(c.interpolate);
    j.at("smoothing").get_to(c.smoothing);
    j.at("threshold").get_to(c.threshold);

    j.at("hook_key").get_to(c.hookKey);
    j.at("enable_key").get_to(c.enableKey);
    j.at("next_key").get_to(c.nextKey);
    j.at("prev_key").get_to(c.prevKey);
} catch (...) {
    c = Config();
}
