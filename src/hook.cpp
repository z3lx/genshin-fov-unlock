#include "hook.h"
#include <MinHook.h>

int Hook::_count = 0;

Hook::Hook(): _isEnabled(false), _target(nullptr) { }

Hook::Hook(void** target, void* detour, const bool enabled)
    : _isEnabled(false), _target(*target) {
    if (!_target) {
        return;
    }
    if (_count++ == 0) {
        MH_Initialize();
    }
    MH_CreateHook(*target, detour, target);
    if (enabled) {
        Enable();
    }
}

Hook::~Hook() {
    Disable();
    if (_target && --_count == 0) {
        MH_Uninitialize();
    }
}

Hook& Hook::operator=(Hook&& other) noexcept {
    if (this == &other) {
        return *this;
    }
    _isEnabled = other._isEnabled;
    _target = other._target;
    other._isEnabled = false;
    other._target = nullptr;
    return *this;
}

Hook::Hook(Hook&& other) noexcept
    : _isEnabled(other._isEnabled), _target(other._target) {
    other._isEnabled = false;
    other._target = nullptr;
}

bool Hook::IsEnabled() const {
    return _isEnabled;
}

bool Hook::Enable() {
    const bool success = MH_EnableHook(_target) == MH_OK;
    if (success) {
        _isEnabled = true;
    }
    return success;
}

bool Hook::Disable() {
    const bool success = MH_DisableHook(_target) == MH_OK;
    if (success) {
        _isEnabled = false;
    }
    return success;
}
