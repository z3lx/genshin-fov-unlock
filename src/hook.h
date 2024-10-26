#pragma once

class Hook {
public:
    Hook() noexcept;
    Hook(void** target, void* detour, bool enable = false);
    ~Hook() noexcept;

    Hook(const Hook&) noexcept = delete;
    Hook& operator=(const Hook&) noexcept = delete;

    Hook(Hook&& other) noexcept ;
    Hook& operator=(Hook&& other) noexcept ;

    [[nodiscard]] bool IsEnabled() const noexcept;
    void Enable();
    void Disable();

private:
    bool _isEnabled;
    void* _target;
    static int _count;
};
