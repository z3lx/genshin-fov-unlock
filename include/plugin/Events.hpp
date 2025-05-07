#pragma once

#include <cstdint>
#include <variant>

#include <Windows.h>

struct OnPluginStart {};

struct OnPluginEnd {};

struct OnKeyDown {
    const uint8_t vKey;
};

struct OnKeyHold {
    const uint8_t vKey;
};

struct OnKeyUp {
    const uint8_t vKey;
};

struct OnCursorVisibilityChange {
    const bool isCursorVisible;
};

struct OnForegroundWindowChange {
    const HWND foregroundWindow;
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
