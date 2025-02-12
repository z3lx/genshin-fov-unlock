#include "utils/WinHook.hpp"

#include <future>
#include <system_error>
#include <thread>

#include <Windows.h>

HHOOK SetWinHook(HookProcedure procedure, HOOKPROC callback) {
    HHOOK handle = nullptr;
    std::promise<void> promise {};
    std::future<void> future = promise.get_future();
    auto setWinHook = [&handle, &promise, procedure, callback]() {
        handle = SetWindowsHookEx(
            static_cast<int>(procedure), callback, nullptr, 0
        );
        if (!handle) {
            promise.set_exception(std::make_exception_ptr(std::system_error {
                static_cast<int>(GetLastError()), std::system_category()
            }));
            return;
        }
        promise.set_value();

        MSG msg;
        while (GetMessage(&msg, nullptr, 0, 0)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    };
    std::thread { setWinHook }.detach();
    future.get();
    return handle;
}

void ClearWinHook(HHOOK& handle) noexcept {
    UnhookWindowsHookEx(handle);
    handle = nullptr;
}

WinHook::WinHook(const HookProcedure procedure, const HOOKPROC callback)
    : hHook { SetWinHook(procedure, callback) } { }

WinHook::~WinHook() noexcept {
    ClearWinHook(hHook);
}

[[nodiscard]] HHOOK WinHook::GetHandle() const noexcept {
    return hHook;
}
