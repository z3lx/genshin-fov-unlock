#pragma once

#include <variant>

#include <Windows.h>

struct OnPluginStart {
};

struct OnPluginEnd {
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

struct OnKeyHold {
    const int vKey;
};

struct OnKeyUp {
    const int vKey;
};

struct OnCursorVisibilityChange {
    const bool isCursorVisible;
};

struct OnForegroundWindowChange {
    const HWND hwnd;
};

using Event = std::variant<
    OnPluginStart,
    OnPluginEnd,
    OnHookToggle,
    OnEnableToggle,
    OnFovChange,
    OnSmoothingChange,
    OnDumpBuffer,
    OnKeyDown,
    OnKeyHold,
    OnKeyUp,
    OnCursorVisibilityChange,
    OnForegroundWindowChange
>;
