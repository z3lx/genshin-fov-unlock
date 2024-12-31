#pragma once

#include <variant>
#include <vector>

struct OnCreateToggle {
    const bool created;
};

struct OnEnableToggle {
    const bool enabled;
};

struct OnFovChange {
    const int fov;
};

struct OnSmoothingChange {
    const double smoothing;
};

struct OnDumpBuffer {
};

struct OnKeyDown {
    const int vKey;
};

struct OnKeyUp {
    const int vKey;
};

using Event = std::variant<
    OnCreateToggle, OnEnableToggle, OnFovChange, OnSmoothingChange, OnDumpBuffer,
    OnKeyDown, OnKeyUp
>;
