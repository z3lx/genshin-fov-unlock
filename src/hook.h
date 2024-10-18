#pragma once

class Hook {
public:
    Hook(void** target, void* detour, bool enabled = false);
    ~Hook();

    bool Enable() const;
    bool Disable() const;

private:
    struct Impl;
    Impl* impl;
};
