#pragma once

#include <MinHook.h>

#include <format>
#include <source_location>
#include <stdexcept>

namespace detail {
    class HookShared {
    protected:
        static inline int _count = 0;
    };
}

template <typename Ret, typename... Args>
class Hook : detail::HookShared {
public:
    Hook();
    ~Hook();

    Hook(const Hook&) noexcept = delete;
    Hook& operator=(const Hook&) noexcept = delete;

    Hook(Hook&& other) noexcept ;
    Hook& operator=(Hook&& other) noexcept ;

    [[nodiscard]] bool IsCreated() const noexcept;
    void Create(void* target, void* detour, bool enable = false);
    void Remove();

    [[nodiscard]] bool IsEnabled() const noexcept;
    void Enable();
    void Disable();

    Ret CallOriginal(Args... args) const;
private:
    static void CheckStatus(
        MH_STATUS status,
        const std::source_location& location
        = std::source_location::current()
    );

    using FuncPtr = Ret(*)(Args...);

    bool _isCreated;
    bool _isEnabled;
    void* _target;
    FuncPtr _original;
};

template <typename Ret, typename... Args>
Hook<Ret, Args...>::Hook()
    : _isCreated(false)
    , _isEnabled(false)
    , _target(nullptr)
    , _original(nullptr) {
    if (_count++ == 0) {
        CheckStatus(MH_Initialize());
    }
}

template <typename Ret, typename... Args>
Hook<Ret, Args...>::~Hook() {
    Remove();
    if (--_count == 0) {
        CheckStatus(MH_Uninitialize());
    }
}

template <typename Ret, typename... Args>
Hook<Ret, Args...>::Hook(Hook&& other) noexcept
    : _isCreated(other._isCreated)
    , _isEnabled(other._isEnabled)
    , _target(other._target)
    , _original(other._original) {
    ++_count;
    other._isCreated = false;
    other._isEnabled = false;
    other._target = nullptr;
    other._original = nullptr;
}

template <typename Ret, typename... Args>
Hook<Ret, Args...>& Hook<Ret, Args...>::operator=(Hook&& other) noexcept {
    if (this == &other) {
        return *this;
    }
    _isCreated = other._isCreated;
    _isEnabled = other._isEnabled;
    _target = other._target;
    _original = other._original;
    other._isCreated = false;
    other._isEnabled = false;
    other._target = nullptr;
    other._original = nullptr;
    return *this;
}

template <typename Ret, typename... Args>
bool Hook<Ret, Args...>::IsCreated() const noexcept {
    return _isCreated;
}

template <typename Ret, typename... Args>
void Hook<Ret, Args...>::Create(
    void* target,
    void* detour,
    const bool enable) {
    if (!target || !detour) {
        throw std::invalid_argument("Target and detour must not be null");
    }

    Remove();

    _target = target;
    void* original {};
    CheckStatus(MH_CreateHook(_target, detour, &original));
    _original = reinterpret_cast<FuncPtr>(original);
    _isCreated = true;

    if (enable) {
        Enable();
    }
}

template <typename Ret, typename... Args>
void Hook<Ret, Args...>::Remove() {
    if (!_isCreated) {
        return;
    }
    Disable();
    CheckStatus(MH_RemoveHook(_target));
    _isCreated = false;
}

template <typename Ret, typename... Args>
bool Hook<Ret, Args...>::IsEnabled() const noexcept {
    return _isEnabled;
}

template <typename Ret, typename... Args>
void Hook<Ret, Args...>::Enable() {
    if (!_isCreated || _isEnabled) {
        return;
    }
    CheckStatus(MH_EnableHook(_target));
    _isEnabled = true;
}

template <typename Ret, typename... Args>
void Hook<Ret, Args...>::Disable() {
    if (!_isCreated || !_isEnabled) {
        return;
    }
    CheckStatus(MH_DisableHook(_target));
    _isEnabled = false;
}

template <typename Ret, typename... Args>
Ret Hook<Ret, Args...>::CallOriginal(Args... args) const {
    if (!_isCreated || !_isEnabled) {
        throw std::runtime_error("Hook must be created and enabled");
    }
    return _original(std::forward<Args>(args)...);
}

template <typename Ret, typename... Args>
void Hook<Ret, Args...>::CheckStatus(
    const MH_STATUS status,
    const std::source_location& location) {
    if (status == MH_OK) {
        return;
    }
    throw std::runtime_error(std::format(
        "MinHook failed with status {} in {} at {}:{}",
        MH_StatusToString(status),
        location.function_name(),
        location.file_name(),
        location.line()
    ));
}
