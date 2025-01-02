#pragma once

#include <stdexcept>

template <typename Ret, typename... Args>
MinHook<Ret, Args...>::MinHook()
    : target(nullptr), original(nullptr) {
    if (!HookBackend::IsInitialized()) {
        HookBackend::Initialize();
    }
}

template <typename Ret, typename... Args>
MinHook<Ret, Args...>::MinHook(void* target, void* detour, const bool enable)
    : MinHook() {
    Create(target, detour, enable);
}

template <typename Ret, typename... Args>
MinHook<Ret, Args...>::~MinHook() {
    Remove();
    if (HookBackend::Count() == 0) {
        HookBackend::Uninitialize();
    }
}

template <typename Ret, typename... Args>
bool MinHook<Ret, Args...>::IsCreated() const noexcept {
    return HookBackend::IsCreated(target);
}

template <typename Ret, typename... Args>
void MinHook<Ret, Args...>::Create(void* target, void* detour, const bool enable) {
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
void MinHook<Ret, Args...>::Remove() const {
    if (IsCreated()) {
        HookBackend::Remove(target);
    }
}

template <typename Ret, typename... Args>
bool MinHook<Ret, Args...>::IsEnabled() const noexcept {
    return HookBackend::IsEnabled(target);
}

template <typename Ret, typename... Args>
void MinHook<Ret, Args...>::Enable() const {
    if (!IsEnabled()) {
        HookBackend::Enable(target);
    }
}

template <typename Ret, typename... Args>
void MinHook<Ret, Args...>::Disable() const {
    if (IsCreated() && IsEnabled()) {
        HookBackend::Disable(target);
    }
}

template <typename Ret, typename... Args>
Ret MinHook<Ret, Args...>::CallOriginal(Args... args) const {
    if (IsCreated() && IsEnabled()) {
        return reinterpret_cast<FuncPtr>(original)(
            std::forward<Args>(args)...
        );
    }
    throw std::runtime_error("Hook must be created and enabled");
}
