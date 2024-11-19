#include "config.h"

#include <nlohmann/json.hpp>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>

constexpr std::string ENABLED = "enabled";
constexpr std::string FOV = "fov";
constexpr std::string FOV_PRESETS = "fov_presets";
constexpr std::string INTERPOLATE = "interpolate";
constexpr std::string SMOOTHING = "smoothing";
constexpr std::string HOOK_KEY = "hook_key";
constexpr std::string ENABLE_KEY = "enable_key";
constexpr std::string NEXT_KEY = "next_key";
constexpr std::string PREV_KEY = "prev_key";
#ifdef ENABLE_LOGGING
constexpr std::string DUMP_KEY = "dump_key";
#endif

nlohmann::ordered_json Config::ToJson() const noexcept {
    return {
        { ENABLED, enabled },
        { FOV, fov },
        { FOV_PRESETS, fovPresets },
        { INTERPOLATE, interpolate },
        { SMOOTHING, smoothing },
        { HOOK_KEY, hookKey },
        { ENABLE_KEY, enableKey },
        { PREV_KEY, nextKey },
        { PREV_KEY, prevKey },
#ifdef ENABLE_LOGGING
        { DUMP_KEY, dumpKey }
#endif
    };
}

void Config::ToJson(const std::filesystem::path& path) const {
    const auto json = ToJson();
    std::ofstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file for writing");
    }
    file << json.dump(4);
}

void Config::FromJson(const nlohmann::ordered_json& json) {
    Config config;

    json.at(ENABLED).get_to(config.enabled);
    json.at(FOV).get_to(config.fov);
    json.at(FOV_PRESETS).get_to(config.fovPresets);
    json.at(INTERPOLATE).get_to(config.interpolate);
    json.at(SMOOTHING).get_to(config.smoothing);
    json.at(HOOK_KEY).get_to(config.hookKey);
    json.at(ENABLE_KEY).get_to(config.enableKey);
    json.at(NEXT_KEY).get_to(config.nextKey);
    json.at(PREV_KEY).get_to(config.prevKey);
#ifdef ENABLE_LOGGING
    json.at(DUMP_KEY).get_to(config.dumpKey);
#endif

    if (config.fov >= 1 && config.fov <= 179) {
        fov = config.fov;
    }
    std::ranges::sort(config.fovPresets);

    *this = config;
}

void Config::FromJson(const std::filesystem::path& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file for reading");
    }
    nlohmann::ordered_json json;
    file >> json;
    FromJson(json);
}
