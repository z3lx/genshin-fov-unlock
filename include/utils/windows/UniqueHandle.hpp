#pragma once

#include <compare>
#include <cstddef>
#include <ostream>

#include <Windows.h>

class UniqueHandle {
public:
    explicit UniqueHandle(HANDLE handle = nullptr) noexcept;
    UniqueHandle(UniqueHandle&& other) noexcept;
    UniqueHandle(const UniqueHandle&) noexcept = delete;
    ~UniqueHandle() noexcept;

    UniqueHandle& operator=(UniqueHandle&& other) noexcept;
    UniqueHandle& operator=(std::nullptr_t) noexcept;
    UniqueHandle& operator=(const UniqueHandle&) noexcept = delete;

    [[nodiscard]] HANDLE Get() const noexcept;
    [[nodiscard]] PHANDLE Put() noexcept;
    explicit operator bool() const noexcept;

    bool operator==(const UniqueHandle& other) const noexcept;
    std::strong_ordering operator<=>(const UniqueHandle& other) const noexcept;
    friend std::ostream& operator<<(std::ostream& os, const UniqueHandle& handle);

private:
    HANDLE handle;
};
