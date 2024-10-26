#pragma once

class Hook {
public:
    Hook();
    Hook(void** target, void* detour, bool enabled = false);
    ~Hook();

    Hook(const Hook&) noexcept = delete;
    Hook& operator=(const Hook&) noexcept = delete;

    Hook(Hook&& other) noexcept ;
    Hook& operator=(Hook&& other) noexcept ;

    [[nodiscard]] bool IsEnabled() const;
    bool Enable();
    bool Disable();

private:
    bool _isEnabled;
    void* _target;
    static int _count;
};
