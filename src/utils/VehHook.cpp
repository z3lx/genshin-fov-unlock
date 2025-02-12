#include "utils/VehHook.hpp"

#include <cstdint>
#include <ranges>
#include <stdexcept>
#include <system_error>
#include <unordered_map>

#include <Windows.h>

// TODO: ADD HOOK ENABLE/DISABLE
// TODO: THREAD SAFETY?

SYSTEM_INFO systemInfo {};
PVOID exceptionHandler {};
std::unordered_map<void*, void*> hooks;

template <typename T> void ThrowOnSystemError(T&& t) {
    if (!t) {
        throw std::system_error {
            static_cast<int>(GetLastError()), std::system_category()
        };
    }
}

DWORD details::SetProtection(void* target, DWORD protection) noexcept {
    DWORD oldProtect {};
    VirtualProtect(target, systemInfo.dwPageSize, protection, &oldProtect);
    return oldProtect;
}

DWORD details::SetPageGuard(void* target) noexcept {
    return details::SetProtection(target, PAGE_EXECUTE_READ | PAGE_GUARD);
}

DWORD details::ClearPageGuard(void* target) noexcept {
    return details::SetProtection(target, PAGE_EXECUTE_READ);
}

LONG VectorizedExceptionHandler(PEXCEPTION_POINTERS exceptionPointers) {
    auto& exceptionRecord = *exceptionPointers->ExceptionRecord;
    auto& contextRecord = *exceptionPointers->ContextRecord;
    const auto exceptionAddress = exceptionRecord.ExceptionAddress;

    switch (exceptionRecord.ExceptionCode) {
        case EXCEPTION_GUARD_PAGE: {
            if (const auto it = hooks.find(exceptionAddress);
                it != hooks.end()) {
                const auto detour = reinterpret_cast<DWORD64>(it->second);
                contextRecord.Rip = detour;
            }
            contextRecord.EFlags |= PAGE_GUARD;
            return EXCEPTION_CONTINUE_EXECUTION;
        }
        case EXCEPTION_SINGLE_STEP: {
            for (const auto& target : hooks | std::views::keys) {
                details::SetPageGuard(target);
            }
            return EXCEPTION_CONTINUE_EXECUTION;
        }
        default: {
            return EXCEPTION_CONTINUE_SEARCH;
        }
    }
}

size_t details::VehHookBackend::Count() noexcept {
    return hooks.size();
}

bool details::VehHookBackend::IsInitialized() noexcept {
    return exceptionHandler != nullptr;
}

void details::VehHookBackend::Initialize() {
    if (IsInitialized()) {
        return;
    }

    GetSystemInfo(&systemInfo);
    ThrowOnSystemError(exceptionHandler = AddVectoredExceptionHandler(
        1, VectorizedExceptionHandler
    ));
}

void details::VehHookBackend::Uninitialize() noexcept {
    if (!IsInitialized()) {
        return;
    }

    for (const auto& target : hooks | std::views::keys) {
        ClearPageGuard(target);
    }
    hooks.clear();
    RemoveVectoredExceptionHandler(exceptionHandler);
    exceptionHandler = nullptr;
}

bool details::VehHookBackend::IsCreated(void* target) noexcept {
    return hooks.contains(target);
}

void details::VehHookBackend::Create(void* target, void* detour) {
    if (!target || !detour) {
        throw std::invalid_argument { "Target and detour must be non-null" };
    }
    if (!IsInitialized()) {
        throw std::runtime_error { "Backend not initialized" };
    }
    if (IsCreated(target)) {
        throw std::invalid_argument { "Hook already created" };
    }

    SetPageGuard(target);
    hooks[target] = detour;
}

void details::VehHookBackend::Remove(void* target) noexcept {
    if (!IsCreated(target)) {
        return;
    }

    ClearPageGuard(target);
    hooks.erase(target);
}
