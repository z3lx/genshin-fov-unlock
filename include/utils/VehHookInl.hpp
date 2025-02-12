#pragma once

#include <type_traits>
#include <utility>

template <typename Ret, typename... Args>
VehHook<Ret, Args...>::VehHook()
    : target { nullptr } {
    if (VehHookBackend::Count() == 0) {
        VehHookBackend::Initialize();
    }
}

template <typename Ret, typename... Args>
VehHook<Ret, Args...>::VehHook(void* target, void* detour)
    : VehHook { } {
    VehHookBackend::Create(target, detour);
}

template <typename Ret, typename... Args>
VehHook<Ret, Args...>::~VehHook() noexcept {
    VehHookBackend::Remove(target);
    if (VehHookBackend::Count() == 0) {
        VehHookBackend::Uninitialize();
    }
}

template <typename Ret, typename... Args>
bool VehHook<Ret, Args...>::IsCreated() const noexcept {
    return VehHookBackend::IsCreated(target);
}

template <typename Ret, typename... Args>
void VehHook<Ret, Args...>::Create(void* target, void* detour) {
    VehHookBackend::Create(target, detour);
    VehHookBackend::Remove(this->target);
    this->target = target;
}

template <typename Ret, typename... Args>
void VehHook<Ret, Args...>::Remove() noexcept {
    VehHookBackend::Remove(target);
    target = nullptr;
}

template <typename Ret, typename... Args>
Ret VehHook<Ret, Args...>::CallOriginal(Args... args) const {
    const auto protection = details::ClearPageGuard(target);
    const auto originalFunction = reinterpret_cast<FuncPtr>(target);
    if constexpr (std::is_void_v<Ret>) {
        originalFunction(std::forward<Args>(args)...);
        details::SetProtection(target, protection);
        return;
    } else {
        Ret ret = originalFunction(std::forward<Args>(args)...);
        details::SetProtection(target, protection);
        return ret;
    }
}
