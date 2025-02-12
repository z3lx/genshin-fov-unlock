#pragma once

#include <cstdint>

#include <Windows.h>

namespace details {
    class VehHookBackend {
    protected:
        [[nodiscard]] static size_t Count() noexcept;

        [[nodiscard]] static bool IsInitialized() noexcept;
        static void Initialize();
        static void Uninitialize() noexcept;

        [[nodiscard]] static bool IsCreated(void* target) noexcept;
        static void Create(void* target, void* detour);
        static void Remove(void* target) noexcept;
    };

    DWORD SetProtection(void* target, DWORD protection) noexcept;
    DWORD SetPageGuard(void* target) noexcept;
    DWORD ClearPageGuard(void* target) noexcept;
}

// TODO: ADD HOOK ENABLE/DISABLE
template <typename Ret, typename... Args>
class VehHook : public details::VehHookBackend {
public:
    VehHook();
    VehHook(void* target, void* detour);
    ~VehHook() noexcept;

    [[nodiscard]] bool IsCreated() const noexcept;
    void Create(void* target, void* detour);
    void Remove() noexcept;

    // [[nodiscard]] bool IsEnabled() const noexcept;
    // void Enable();
    // void Disable() noexcept;

    Ret CallOriginal(Args... args) const;

private:
    using FuncPtr = Ret(*)(Args...);

    void* target;
};

#include "utils/VehHookInl.hpp"
