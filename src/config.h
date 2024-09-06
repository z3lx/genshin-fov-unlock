#pragma once
#include <vector>
#include <windows.h>
#include <nlohmann/json.hpp>

struct Config {
    bool enabled = true;
    int fov = 75;
    std::vector<int> fovPresets = { 30, 45, 60, 75, 90, 110 };
    int cycleNextKey = VK_UP;
    int cyclePrevKey = VK_DOWN;
};

void to_json(nlohmann::json& j, const Config& c) {
    j = nlohmann::json{
        { "enabled", c.enabled },
        { "fov", c.fov },
        { "fov_presets", c.fovPresets },
        { "cycle_next_key", c.cycleNextKey },
        { "cycle_prev_key", c.cyclePrevKey }
    };
}

void from_json(const nlohmann::json& j, Config& c) {
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

    j.at("cycle_next_key").get_to(c.cycleNextKey);
    j.at("cycle_prev_key").get_to(c.cyclePrevKey);
}
