#pragma once

class Hook {
public:
    Hook();
    ~Hook();

    Hook(const Hook&) noexcept = delete;
    Hook& operator=(const Hook&) noexcept = delete;

    Hook(Hook&& other) noexcept ;
    Hook& operator=(Hook&& other) noexcept ;

    [[nodiscard]] bool IsCreated() const noexcept;
    void Create(void** target, void* detour, bool enable = false);
    void Remove();

    [[nodiscard]] bool IsEnabled() const noexcept;
    void Enable();
    void Disable();

private:
    bool _isCreated;
    bool _isEnabled;
    void* _target;
    static int _count;
};
