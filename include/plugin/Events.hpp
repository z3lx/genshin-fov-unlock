#pragma once

#include <variant>

#include <Windows.h>

struct OnPluginStart {
};

struct OnPluginEnd {
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
    OnKeyDown,
    OnKeyHold,
    OnKeyUp,
    OnCursorVisibilityChange,
    OnForegroundWindowChange
>;
