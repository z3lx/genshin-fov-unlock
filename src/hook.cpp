#include "hook.h"
#include <MinHook.h>

struct Hook::Impl {
    Impl(void** target, void* detour);
    ~Impl();

    void* target;
    static int count;
};

int Hook::Impl::count = 0;

Hook::Impl::Impl(
    void** target, void* detour
) : target(*target) {
    if (count++ == 0) {
        MH_Initialize();
    }
    MH_CreateHook(*target, detour, target);
    MH_EnableHook(this->target);
}

Hook::Impl::~Impl() {
    MH_DisableHook(this->target);
    if (--count == 0) {
        MH_Uninitialize();
    }
}

Hook::Hook(
    void** target, void* detour
) : impl(new Impl(target, detour)) { }

Hook::~Hook() {
    delete impl;
}
