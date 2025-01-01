#pragma once

#include <variant>

struct OnPluginInitialize {
};

struct OnPluginUninitialize {
};

struct OnHookToggle {
    const bool hooked;
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
    OnPluginInitialize,
    OnPluginUninitialize,
    OnHookToggle,
    OnEnableToggle,
    OnFovChange,
    OnSmoothingChange,
    OnDumpBuffer,
    OnKeyDown,
    OnKeyUp
>;
