#include "hook.h"

#include <MinHook.h>

#include <format>
#include <source_location>
#include <stdexcept>

namespace mh {
    void Initialize();
    void Deinitialize();

    void CreateHook(void* target, void* detour, void** original);
    void Remove(void* target);

    void Enable(void* target);
    void Disable(void* target);
}

int Hook::_count = 0;

Hook::Hook()
    : _isCreated(false)
    , _isEnabled(false)
    , _target(nullptr) {
    if (_count++ == 0) {
        mh::Initialize();
    }
}

Hook::~Hook() {
    Remove();
    if (--_count == 0) {
        mh::Deinitialize();
    }
}

Hook::Hook(Hook&& other) noexcept
    : _isCreated(other._isCreated)
    , _isEnabled(other._isEnabled)
    , _target(other._target) {
    ++_count;
    other._isCreated = false;
    other._isEnabled = false;
    other._target = nullptr;
}

Hook& Hook::operator=(Hook&& other) noexcept {
    if (this == &other) {
        return *this;
    }
    _isCreated = other._isCreated;
    _isEnabled = other._isEnabled;
    _target = other._target;
    other._isCreated = false;
    other._isEnabled = false;
    other._target = nullptr;
    return *this;
}

bool Hook::IsCreated() const noexcept {
    return _isCreated;
}

void Hook::Create(void** target, void* detour, const bool enable) {
    if (!target || !detour) {
        throw std::invalid_argument(
            "Target and detour must not be null"
        );
    }

    Remove();

    _target = *target;
    mh::CreateHook(_target, detour, target);
    _isCreated = true;

    if (enable) {
        Enable();
    }
}

void Hook::Remove() {
    if (!_isCreated) {
        return;
    }
    Disable();
    mh::Remove(_target);
    _isCreated = false;
}

bool Hook::IsEnabled() const noexcept {
    return _isEnabled;
}

void Hook::Enable() {
    if (!_isCreated || _isEnabled) {
        return;
    }
    mh::Enable(_target);
    _isEnabled = true;
}

void Hook::Disable() {
    if (!_isCreated || !_isEnabled) {
        return;
    }
    mh::Disable(_target);
    _isEnabled = false;
}

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

void mh::Initialize() {
    CheckStatus(MH_Initialize());
}

void mh::Deinitialize() {
    CheckStatus(MH_Uninitialize());
}

void mh::CreateHook(void* target, void* detour, void** original) {
    CheckStatus(MH_CreateHook(target, detour, original));
}

void mh::Remove(void* target) {
    CheckStatus(MH_RemoveHook(target));
}

void mh::Enable(void* target) {
    CheckStatus(MH_EnableHook(target));
}

void mh::Disable(void* target) {
    CheckStatus(MH_DisableHook(target));
}
