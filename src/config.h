#pragma once
#include <vector>
#include <windows.h>
#include <nlohmann/json.hpp>

struct Config {
    bool enabled = true;
    int fov = 75;
    std::vector<int> fovPresets = { 30, 45, 60, 75, 90, 110 };
    float smoothing = 0.1;
    float threshold = 4.0;
    int enableKey = VK_LEFT;
    int nextKey = VK_DOWN;
    int prevKey = VK_UP;
};

void to_json(nlohmann::ordered_json& j, const Config& c) {
    j = nlohmann::ordered_json{
        { "enabled", c.enabled },
        { "fov", c.fov },
        { "fov_presets", c.fovPresets },
        { "smoothing", c.smoothing },
        { "threshold", c.threshold },
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

    j.at("smoothing").get_to(c.smoothing);
    j.at("threshold").get_to(c.threshold);
    j.at("enable_key").get_to(c.enableKey);
    j.at("next_key").get_to(c.nextKey);
    j.at("prev_key").get_to(c.prevKey);
} catch (...) {
    c = Config();
}
