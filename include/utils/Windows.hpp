#pragma once

#include <system_error>
#include <tuple>
#include <type_traits>

#include <Windows.h>

template <typename T> void ThrowOnSystemError(T&& condition) {
    if (!condition) {
        throw std::system_error {
            static_cast<int>(GetLastError()), std::system_category()
        };
    }
}

template <typename Callable, typename... Args>
requires std::is_nothrow_invocable_r_v<void, Callable, Args...> &&
    std::is_default_constructible_v<Callable>
class ResourceGuard {
public:
    explicit ResourceGuard(Args... args) noexcept
        : args { std::make_tuple(args...) } { }

    ~ResourceGuard() noexcept {
        std::apply(Callable {}, args);
    }

    ResourceGuard(const ResourceGuard&) = delete;
    ResourceGuard& operator=(const ResourceGuard&) = delete;
    ResourceGuard(ResourceGuard&&) = delete;
    ResourceGuard& operator=(ResourceGuard&&) = delete;

private:
    std::tuple<Args...> args;
};

using HandleDeleter = decltype([] (HANDLE handle) noexcept {
    if (handle) {
        CloseHandle(handle);
    }
});

using VirtualMemoryDeleter = decltype([] (
    HANDLE processHandle, LPVOID address, SIZE_T size, DWORD freeType) noexcept {
    if (address) {
        VirtualFreeEx(processHandle, address, size, freeType);
    }
});

using HandleGuard = ResourceGuard<HandleDeleter, HANDLE>;
using VirtualMemoryGuard = ResourceGuard<VirtualMemoryDeleter, HANDLE, LPVOID, SIZE_T, DWORD>;
