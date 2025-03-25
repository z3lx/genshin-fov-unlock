#pragma once

#include <filesystem>
#include <system_error>
#include <vector>

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

void AllocateConsole();
std::filesystem::path GetModulePath(const void* address = nullptr);
std::vector<HWND> GetProcessWindows(DWORD processId = 0);
