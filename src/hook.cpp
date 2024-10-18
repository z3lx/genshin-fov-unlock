#include "hook.h"
#include <MinHook.h>

struct Hook::Impl {
    Impl(void** target, void* detour, bool enabled);
    ~Impl();

    bool Enable() const;
    bool Disable() const;

    void* target;
    static int count;
};

int Hook::Impl::count = 0;

Hook::Impl::Impl(void** target, void* detour, const bool enabled)
    : target(*target) {
    if (count++ == 0) {
        MH_Initialize();
    }
    MH_CreateHook(*target, detour, target);
    if (enabled) {
        MH_EnableHook(*target);
    }
}

Hook::Impl::~Impl() {
    Disable();
    if (--count == 0) {
        MH_Uninitialize();
    }
}

bool Hook::Impl::Enable() const {
    return MH_EnableHook(this->target) == MH_OK;
}

bool Hook::Impl::Disable() const {
    return MH_DisableHook(this->target) == MH_OK;
}

Hook::Hook(void** target, void* detour, const bool enabled)
    : impl(new Impl(target, detour, enabled)) { }

Hook::~Hook() {
    delete impl;
}

bool Hook::Enable() const {
    return impl->Enable();
}

bool Hook::Disable() const {
    return impl->Disable();
}
