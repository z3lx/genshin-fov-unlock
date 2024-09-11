#pragma once

#include <memory>

class Hook {
public:
    Hook(void** target, void* detour);
    ~Hook();

private:
    struct Impl;
    std::unique_ptr<Impl> impl;
};
