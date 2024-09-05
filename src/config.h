#pragma once
#include <nlohmann/json.hpp>

struct Config {
    int fieldOfView = 90;
};

void to_json(nlohmann::json& j, const Config& c) {
    j = nlohmann::json{{"field_of_view", c.fieldOfView}};
}

void from_json(const nlohmann::json& j, Config& c) {
    int fov;
    j.at("field_of_view").get_to(fov);
    if (fov >= 1 && fov <= 179) {
        c.fieldOfView = fov;
    }
}
