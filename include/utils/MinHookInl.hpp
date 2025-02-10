#pragma once

#include <stdexcept>

template <typename Ret, typename... Args>
MinHook<Ret, Args...>::MinHook()
    : target(nullptr), original(nullptr) {
    if (!MinHookBackend::IsInitialized()) {
        MinHookBackend::Initialize();
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
    if (MinHookBackend::Count() == 0) {
        MinHookBackend::Uninitialize();
    }
}

template <typename Ret, typename... Args>
bool MinHook<Ret, Args...>::IsCreated() const noexcept {
    return MinHookBackend::IsCreated(target);
}

template <typename Ret, typename... Args>
void MinHook<Ret, Args...>::Create(void* target, void* detour, const bool enable) {
    if (!target || !detour) {
        throw std::invalid_argument("Target and detour must not be null");
    }
    Remove();
    MinHookBackend::Create(target, detour, &original);
    this->target = target;
    if (enable) {
        Enable();
    }
}

template <typename Ret, typename... Args>
void MinHook<Ret, Args...>::Remove() const {
    if (IsCreated()) {
        MinHookBackend::Remove(target);
    }
}

template <typename Ret, typename... Args>
bool MinHook<Ret, Args...>::IsEnabled() const noexcept {
    return MinHookBackend::IsEnabled(target);
}

template <typename Ret, typename... Args>
void MinHook<Ret, Args...>::Enable() const {
    if (!IsEnabled()) {
        MinHookBackend::Enable(target);
    }
}

template <typename Ret, typename... Args>
void MinHook<Ret, Args...>::Disable() const {
    if (IsCreated() && IsEnabled()) {
        MinHookBackend::Disable(target);
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
