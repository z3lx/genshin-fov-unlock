#pragma once

#include <stdexcept>

template <typename Ret, typename... Args>
Hook<Ret, Args...>::Hook()
    : target(nullptr), original(nullptr) {
    if (!HookBackend::IsInitialized()) {
        HookBackend::Initialize();
    }
}

template <typename Ret, typename... Args>
Hook<Ret, Args...>::Hook(void* target, void* detour, const bool enable)
    : Hook() {
    Create(target, detour, enable);
}

template <typename Ret, typename... Args>
Hook<Ret, Args...>::~Hook() {
    Remove();
    if (HookBackend::Count() == 0) {
        HookBackend::Uninitialize();
    }
}

template <typename Ret, typename... Args>
bool Hook<Ret, Args...>::IsCreated() const noexcept {
    return HookBackend::IsCreated(target);
}

template <typename Ret, typename... Args>
void Hook<Ret, Args...>::Create(void* target, void* detour, const bool enable) {
    if (!target || !detour) {
        throw std::invalid_argument("Target and detour must not be null");
    }
    Remove();
    HookBackend::Create(target, detour, &original);
    this->target = target;
    if (enable) {
        Enable();
    }
}

template <typename Ret, typename... Args>
void Hook<Ret, Args...>::Remove() const {
    if (IsCreated()) {
        HookBackend::Remove(target);
    }
}

template <typename Ret, typename... Args>
bool Hook<Ret, Args...>::IsEnabled() const noexcept {
    return HookBackend::IsEnabled(target);
}

template <typename Ret, typename... Args>
void Hook<Ret, Args...>::Enable() const {
    if (!IsEnabled()) {
        HookBackend::Enable(target);
    }
}

template <typename Ret, typename... Args>
void Hook<Ret, Args...>::Disable() const {
    if (IsCreated() && IsEnabled()) {
        HookBackend::Disable(target);
    }
}

template <typename Ret, typename... Args>
Ret Hook<Ret, Args...>::CallOriginal(Args... args) const {
    if (IsCreated() && IsEnabled()) {
        return reinterpret_cast<FuncPtr>(original)(
            std::forward<Args>(args)...
        );
    }
    throw std::runtime_error("Hook must be created and enabled");
}
