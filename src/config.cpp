#include "config.h"

#include <nlohmann/json.hpp>

#include <algorithm>
#include <string>
#include <vector>

constexpr std::string ENABLED = "enabled";
constexpr std::string FOV = "fov";
constexpr std::string FOV_PRESETS = "fov_presets";
constexpr std::string INTERPOLATE = "interpolate";
constexpr std::string SMOOTHING = "smoothing";
constexpr std::string THRESHOLD = "threshold";
constexpr std::string HOOK_KEY = "hook_key";
constexpr std::string ENABLE_KEY = "enable_key";
constexpr std::string NEXT_KEY = "next_key";
constexpr std::string PREV_KEY = "prev_key";

void to_json(nlohmann::ordered_json& j, const Config& c) {
    j = nlohmann::ordered_json{
        { ENABLED, c.enabled },
        { FOV, c.fov },
        { FOV_PRESETS, c.fovPresets },
        { INTERPOLATE, c.interpolate },
        { SMOOTHING, c.smoothing },
        { THRESHOLD, c.threshold },
        { HOOK_KEY, c.hookKey },
        { ENABLE_KEY, c.enableKey },
        { PREV_KEY, c.nextKey },
        { PREV_KEY, c.prevKey }
    };
}

void from_json(const nlohmann::ordered_json& j, Config& c) try {
    j.at(ENABLED).get_to(c.enabled);

    int fov;
    j.at(FOV).get_to(fov);
    if (fov >= 1 && fov <= 179) {
        c.fov = fov;
    }

    std::vector<int> presets;
    j.at(FOV_PRESETS).get_to(presets);
    std::ranges::sort(presets);
    c.fovPresets = presets;

    j.at(INTERPOLATE).get_to(c.interpolate);
    j.at(SMOOTHING).get_to(c.smoothing);
    j.at(THRESHOLD).get_to(c.threshold);

    j.at(HOOK_KEY).get_to(c.hookKey);
    j.at(ENABLE_KEY).get_to(c.enableKey);
    j.at(NEXT_KEY).get_to(c.nextKey);
    j.at(PREV_KEY).get_to(c.prevKey);
} catch (...) {
    c = Config();
}
