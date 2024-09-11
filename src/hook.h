#pragma once

class Hook {
public:
    Hook(void** target, void* detour);
    ~Hook();

private:
    struct Impl;
    Impl* impl;
};
