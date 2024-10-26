#include "hook.h"

#include <MinHook.h>

#include <format>
#include <source_location>
#include <stdexcept>

void CheckStatus(
    const MH_STATUS status,
    const std::source_location& location = std::source_location::current()) {
    if (status == MH_OK) {
        return;
    }
    throw std::runtime_error(std::format(
        "MinHook failed with status {} in {} at {}:{}",
        MH_StatusToString(status),
        location.function_name(),
        location.file_name(),
        location.line()
    ));
}

void Initialize() {
    CheckStatus(MH_Initialize());
}

void Deinitialize() {
    CheckStatus(MH_Uninitialize());
}

void CreateHook(void* target, void* detour, void** original) {
    CheckStatus(MH_CreateHook(target, detour, original));
}

int Hook::_count = 0;

Hook::Hook() noexcept : _isEnabled(false), _target(nullptr) { }

Hook::Hook(void** target, void* detour, const bool enable)
    : _isEnabled(false), _target(*target) {
    if (!_target || !detour) {
        throw std::invalid_argument("Target and detour must not be null");
    }

    if (_count++ == 0) {
        Initialize();
    }
    try {
        CreateHook(_target, detour, target);
    } catch (...) {
        if (--_count == 0) {
            Deinitialize();
        }
        throw;
    }

    if (enable) {
        Enable();
    }
}

Hook::~Hook() noexcept try {
    if (!_target) {
        return;
    }
    if (_isEnabled) {
        Disable();
    }
    if (--_count == 0) {
        Deinitialize();
    }
} catch(...) {
    // TODO: Log error
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

bool Hook::IsEnabled() const noexcept {
    return _isEnabled;
}

void Hook::Enable() {
    CheckStatus(MH_EnableHook(_target));
    _isEnabled = true;
}

void Hook::Disable() {
    CheckStatus(MH_DisableHook(_target));
    _isEnabled = false;
}
