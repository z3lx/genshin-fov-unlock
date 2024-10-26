#pragma once

class Hook {
public:
    Hook();
    Hook(void** target, void* detour, bool enabled = false);
    ~Hook();

    [[nodiscard]] bool IsEnabled() const;
    bool Enable();
    bool Disable();

private:
    bool _isEnabled;
    void* _target;
    static int _count;
};
