#include "utils/windows/UniqueHandle.hpp"

#include <compare>
#include <cstddef>
#include <ostream>

#include <Windows.h>

void Delete(HANDLE& handle) noexcept {
    if (handle && handle != INVALID_HANDLE_VALUE) {
        CloseHandle(handle);
    }
    handle = nullptr;
}

UniqueHandle::UniqueHandle(const HANDLE handle) noexcept
    : handle(handle) { }

UniqueHandle::UniqueHandle(UniqueHandle&& other) noexcept
    : handle(other.handle) {
    other.handle = nullptr;
}

UniqueHandle::~UniqueHandle() noexcept {
    Delete(handle);
}

UniqueHandle& UniqueHandle::operator=(UniqueHandle&& other) noexcept {
    if (this != &other) {
        Delete(handle);
        handle = other.handle;
        other.handle = nullptr;
    }
    return *this;
}

UniqueHandle& UniqueHandle::operator=(std::nullptr_t) noexcept {
    Delete(handle);
    return *this;
}

HANDLE UniqueHandle::Get() const noexcept {
    return handle;
}

PHANDLE UniqueHandle::Put() noexcept {
    return &handle;
}

UniqueHandle::operator bool() const noexcept {
    return handle && handle != INVALID_HANDLE_VALUE;
}

bool UniqueHandle::operator==(const UniqueHandle& other) const noexcept {
    return handle == other.handle || (
        (handle == nullptr || handle == INVALID_HANDLE_VALUE) &&
        (other.handle == nullptr || other.handle == INVALID_HANDLE_VALUE));
}

std::strong_ordering UniqueHandle::operator<=>(const UniqueHandle& other) const noexcept {
    if (handle == other.handle) {
        return std::strong_ordering::equal;
    }
    return handle <=> other.handle;
}

std::ostream& operator<<(std::ostream& os, const UniqueHandle& handle) {
    return os << handle.Get();
}
