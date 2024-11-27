#pragma once

#include <stdexcept>

template <typename Ret, typename... Args>
Hook<Ret, Args...>::Hook() noexcept
    : target(nullptr)
    , original(nullptr) { }

template <typename Ret, typename... Args>
Hook<Ret, Args...>::~Hook() noexcept {
    try { Remove(); }
    catch (...) { }
}

// ReSharper disable once CppMemberFunctionMayBeStatic
template <typename Ret, typename... Args>
bool Hook<Ret, Args...>::IsInitialized() const noexcept {
    return HookShared::IsInitialized();
}

template <typename Ret, typename... Args>
void Hook<Ret, Args...>::Initialize() const {
    if (!IsInitialized()) {
        HookShared::Initialize();
    }
}

template <typename Ret, typename... Args>
void Hook<Ret, Args...>::Uninitialize() const {
    if (IsInitialized()) {
        HookShared::Uninitialize();
    }
}

template <typename Ret, typename... Args>
bool Hook<Ret, Args...>::IsCreated() const noexcept {
    return HookShared::IsCreated(target);
}

template <typename Ret, typename... Args>
void Hook<Ret, Args...>::Create(void* target, void* detour) {
    if (!target || !detour) {
        throw std::invalid_argument(
            "Target and detour must not be null"
        );
    }
    Remove();
    HookShared::Create(target, detour, &original);
    this->target = target;
}

template <typename Ret, typename... Args>
void Hook<Ret, Args...>::Remove() const {
    if (IsInitialized() && IsCreated()) {
        HookShared::Remove(target);
    }
}

template <typename Ret, typename... Args>
bool Hook<Ret, Args...>::IsEnabled() const noexcept {
    return HookShared::IsEnabled(target);
}

template <typename Ret, typename... Args>
void Hook<Ret, Args...>::Enable() const {
    if (!IsEnabled()) {
        HookShared::Enable(target);
    }
}

template <typename Ret, typename... Args>
void Hook<Ret, Args...>::Disable() const {
    if (IsInitialized() && IsCreated() && IsEnabled()) {
        HookShared::Disable(target);
    }
}

template <typename Ret, typename... Args>
Ret Hook<Ret, Args...>::CallOriginal(Args... args) const {
    if (IsInitialized() && IsCreated() && IsEnabled()) {
        return reinterpret_cast<FuncPtr>(original)(
            std::forward<Args>(args)...
        );
    }
    throw std::runtime_error(
        "Hook must be initialized, created, and enabled"
    );
}
