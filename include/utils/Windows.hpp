#pragma once

#include <system_error>

#include <Windows.h>

template <typename T>
void ThrowOnSystemError(T&& condition) {
    if (condition) {
        return;
    }
    throw std::system_error {
        static_cast<int>(GetLastError()),
        std::system_category()
    };
}
