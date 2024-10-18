#pragma once

class Hook {
public:
    Hook(void** target, void* detour, bool enabled = false);
    ~Hook();

    [[nodiscard]] bool IsEnabled() const;
    bool Enable();
    bool Disable();

private:
    struct Impl;
    Impl* impl;
};
