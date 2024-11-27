#pragma once

#include <stdexcept>

namespace detail {
    class HookShared {
    protected:
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
class Hook : public detail::HookShared {
public:
    Hook() noexcept;
    ~Hook() noexcept;

    [[nodiscard]] bool IsInitialized() const noexcept;
    void Initialize() const;
    void Uninitialize() const;

    [[nodiscard]] bool IsCreated() const noexcept;
    void Create(void* target, void* detour);
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
