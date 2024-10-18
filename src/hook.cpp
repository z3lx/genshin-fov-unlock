#include "hook.h"
#include <MinHook.h>

struct Hook::Impl {
    Impl(void** target, void* detour, bool enabled);
    ~Impl();

    [[nodiscard]] bool IsEnabled() const;
    bool Enable();
    bool Disable();

    bool isEnabled;
    void* target;
    static int count;
};

int Hook::Impl::count = 0;

Hook::Impl::Impl(void** target, void* detour, const bool enabled)
    : isEnabled(false)
    , target(*target) {
    if (count++ == 0) {
        MH_Initialize();
    }
    MH_CreateHook(*target, detour, target);
    if (enabled) {
        Enable();
    }
}

Hook::Impl::~Impl() {
    Disable();
    if (--count == 0) {
        MH_Uninitialize();
    }
}

bool Hook::Impl::IsEnabled() const {
    return isEnabled;
}

bool Hook::Impl::Enable() {
    const bool success = MH_EnableHook(this->target) == MH_OK;
    if (success) {
        isEnabled = true;
    }
    return success;
}

bool Hook::Impl::Disable() {
    const bool success = MH_DisableHook(this->target) == MH_OK;
    if (success) {
        isEnabled = false;
    }
    return success;
}

Hook::Hook(void** target, void* detour, const bool enabled)
    : impl(new Impl(target, detour, enabled)) { }

Hook::~Hook() {
    delete impl;
}

bool Hook::IsEnabled() const {
    return impl->IsEnabled();
}

bool Hook::Enable() {
    return impl->Enable();
}

bool Hook::Disable() {
    return impl->Disable();
}
