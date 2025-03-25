#include "utils/Windows.hpp"

#include <chrono>
#include <filesystem>
#include <iostream>
#include <ranges>
#include <thread>
#include <tuple>
#include <vector>

#include <Windows.h>

namespace fs = std::filesystem;

void AllocateConsole() {
    ThrowOnSystemError(AllocConsole());
    FILE* file {};
    freopen_s(&file, "CONOUT$", "w", stdout);
    freopen_s(&file, "CONOUT$", "w", stderr);
    freopen_s(&file, "CONIN$", "r", stdin);
    std::cout.clear();
    std::cerr.clear();
    std::cin.clear();
    std::wcout.clear();
    std::wcerr.clear();
    std::wcin.clear();
}

fs::path GetModulePath(const void* address) {
    if (!address) {
        address = reinterpret_cast<void*>(GetModulePath);
    }

    HMODULE module {};
    ThrowOnSystemError(GetModuleHandleEx(
        GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
        GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
        static_cast<LPCTSTR>(address), &module
    ));

    char buffer[MAX_PATH];
    ThrowOnSystemError(GetModuleFileName(
        module, buffer, MAX_PATH
    ));
    return fs::path { buffer };
}

std::vector<HWND> GetProcessWindows(DWORD processId) {
    if (!processId) {
        processId = GetCurrentProcessId();
    }

    std::vector<HWND> windows {};
    std::tuple params = std::tie(processId, windows);

    const auto callback = [](HWND hwnd, LPARAM lParam) -> BOOL {
        auto& [targetProcessId, targetWindows] = *reinterpret_cast<
            std::tuple<DWORD&, std::vector<HWND>&>*>(lParam);

        DWORD processId {};
        if (!GetWindowThreadProcessId(hwnd, &processId)) {
            return FALSE;
        }
        if (processId == targetProcessId &&
            !std::ranges::contains(targetWindows, hwnd)) {
            targetWindows.push_back(hwnd);
        }
        return TRUE;
    };

    while (windows.empty()) {
        ThrowOnSystemError(EnumWindows(
            callback, reinterpret_cast<LPARAM>(&params)
        ));
        std::this_thread::sleep_for(std::chrono::seconds { 1 });
    }
    return windows;
}
