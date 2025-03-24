#include "plugin/components/WinEventNotifier.hpp"
#include "plugin/Events.hpp"
#include "plugin/interfaces/IMediator.hpp"
#include "utils/ThreadWrapper.hpp"
#include "utils/Windows.hpp"
#include "utils/log/Logger.hpp"

#include <algorithm>
#include <memory>
#include <mutex>
#include <optional>
#include <thread>
#include <utility>
#include <variant>
#include <vector>

#include <Windows.h>

namespace {
class WinEventHook {
public:
    static void Register(WinEventNotifier* instance);
    static void Unregister(WinEventNotifier* instance) noexcept;

private:
    static void CALLBACK WinEventProc(
        HWINEVENTHOOK hWinEventHook, DWORD event, HWND hwnd,
        LONG idObject, LONG idChild, DWORD dwEventThread, DWORD dwmsEventTime);

    static void SetHook();
    static void MessageLoop() noexcept;
    static void ClearHook(std::thread& thread) noexcept;

    using Prologue = decltype(&SetHook);
    using Body = decltype(&MessageLoop);
    using Epilogue = decltype(&ClearHook);
    static inline HWINEVENTHOOK hWinEventHook {};

    static inline std::mutex mutex {};
    static inline std::vector<WinEventNotifier*> instances {};
    static inline std::optional<ThreadWrapper<
        Prologue, Body, Epilogue>> thread {};
};
} // namespace

void WinEventHook::Register(WinEventNotifier* instance) try {
    std::lock_guard lock { mutex };
    if (const auto it = std::ranges::find(instances, instance);
        it != instances.end()) {
        return;
    }
    instances.emplace_back(instance);
    if (!thread.has_value()) {
        thread.emplace(SetHook, MessageLoop, ClearHook);
    }
} catch (...) {
    Unregister(instance);
    throw;
}

void WinEventHook::Unregister(WinEventNotifier* instance) noexcept {
    std::lock_guard lock { mutex };
    if (const auto it = std::ranges::find(instances, instance);
        it != instances.end()) {
        instances.erase(it);
    }
    if (instances.empty()) {
        thread.reset();
    }
}

void CALLBACK WinEventHook::WinEventProc(
    const HWINEVENTHOOK hWinEventHook, const DWORD event, const HWND hwnd,
    const LONG idObject, const LONG idChild, const DWORD dwEventThread,
    const DWORD dwmsEventTime) {
    if (event != EVENT_SYSTEM_FOREGROUND) {
        return;
    }
    std::lock_guard lock { mutex };
    for (const auto instance : instances) {
        if (!instance->IsEnabled()) {
            continue;
        }
        if (const auto mediator = instance->GetMediator().lock()) {
            mediator->Notify(OnForegroundWindowChange { hwnd });
        }
    }
}

void WinEventHook::SetHook() {
    ThrowOnSystemError(hWinEventHook = SetWinEventHook(
        EVENT_SYSTEM_FOREGROUND,
        EVENT_SYSTEM_FOREGROUND,
        nullptr,
        WinEventProc,
        0,
        0,
        WINEVENT_OUTOFCONTEXT
    ));
}

void WinEventHook::MessageLoop() noexcept {
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

void WinEventHook::ClearHook(std::thread& thread) noexcept {
    const auto threadId = GetThreadId(thread.native_handle());
    PostThreadMessage(threadId, WM_QUIT, 0, 0);
    UnhookWinEvent(hWinEventHook);
    hWinEventHook = nullptr;
}

WinEventNotifier::WinEventNotifier(
    std::weak_ptr<IMediator<Event>> mediator) try
    : IComponent { std::move(mediator) }
    , isEnabled { false } {
    WinEventHook::Register(this);
} catch (const std::exception& e) {
    LOG_E("Failed to create WinEventNotifier: {}", e.what());
    throw;
}

WinEventNotifier::~WinEventNotifier() noexcept {
    WinEventHook::Unregister(this);
}

bool WinEventNotifier::IsEnabled() const noexcept {
    return isEnabled;
}

void WinEventNotifier::SetEnabled(const bool value) noexcept {
    isEnabled = value;
}
