#pragma once

#include <cstdint>

namespace details {
    class HookBackend {
    protected:
        [[nodiscard]] static size_t Count() noexcept;

        [[nodiscard]] static bool IsInitialized() noexcept;
        static void Initialize();
        static void Uninitialize();

        [[nodiscard]] static bool IsCreated(void* target) noexcept;
        static void Create(void* target, void* detour, void** original);
        static void Remove(void* target);

        [[nodiscard]] static bool IsEnabled(void* target) noexcept;
        static void Enable(void* target);
        static void Disable(void* target);
    };
}

template <typename Ret, typename... Args>
class Hook : public details::HookBackend {
public:
    Hook();
    Hook(void* target, void* detour, bool enable = false);
    ~Hook();

    [[nodiscard]] bool IsCreated() const noexcept;
    void Create(void* target, void* detour, bool enable = false);
    void Remove() const;

    [[nodiscard]] bool IsEnabled() const noexcept;
    void Enable() const;
    void Disable() const;

    Ret CallOriginal(Args... args) const;

private:
    using FuncPtr = Ret(*)(Args...);

    void* target;
    void* original;
};

#include "utils/HookInl.h"
