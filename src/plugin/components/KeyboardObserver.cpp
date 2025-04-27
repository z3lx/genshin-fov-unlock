#include "plugin/components/KeyboardObserver.hpp"
#include "plugin/Events.hpp"
#include "utils/ThreadWrapper.hpp"
#include "utils/Windows.hpp"
#include "utils/log/Logger.hpp"

#include <algorithm>
#include <exception>
#include <mutex>
#include <optional>
#include <thread>
#include <unordered_map>
#include <utility>
#include <vector>

#include <Windows.h>

struct KeyboardObserver::Hook {
    static void Register(KeyboardObserver* instance);
    static void Unregister(KeyboardObserver* instance) noexcept;

private:
    static LRESULT CALLBACK KeyboardProc(
        int nCode, WPARAM wParam, LPARAM lParam) noexcept;

    static void SetHook();
    static void MessageLoop() noexcept;
    static void ClearHook(std::thread& thread) noexcept;
    static inline HHOOK hHook {};

    using Prologue = decltype(&SetHook);
    using Body = decltype(&MessageLoop);
    using Epilogue = decltype(&ClearHook);

    static inline std::mutex mutex {};
    static inline std::vector<KeyboardObserver*> instances {};
    static inline std::optional<ThreadWrapper<
        Prologue, Body, Epilogue>> thread {};
    static inline std::unordered_map<int, bool> keyHeldStates {};
};

void KeyboardObserver::Hook::Register(KeyboardObserver* instance) try {
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

void KeyboardObserver::Hook::Unregister(KeyboardObserver* instance) noexcept {
    std::lock_guard lock { mutex };
    if (const auto it = std::ranges::find(instances, instance);
        it != instances.end()) {
        instances.erase(it);
    }
    if (instances.empty()) {
        thread.reset();
    }
}

LRESULT CALLBACK KeyboardObserver::Hook::KeyboardProc(
    const int nCode, const WPARAM wParam, const LPARAM lParam) noexcept {
    const auto next = [nCode, wParam, lParam]() noexcept {
        return CallNextHookEx(hHook, nCode, wParam, lParam);
    };

    if (nCode != HC_ACTION) {
        return next();
    }

    Event event {};
    const auto keyboard = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);
    const auto key = static_cast<int>(keyboard->vkCode);
    bool* isKeyHeldPtr {};
    try {
        isKeyHeldPtr = &keyHeldStates[key];
    } catch (const std::exception& e) {
        LOG_E("Failed to get key state: {}", e.what());
        return next();
    }
    auto& isKeyHeld = *isKeyHeldPtr;

    switch (wParam) {
        case WM_KEYDOWN: case WM_SYSKEYDOWN: {
            if (isKeyHeld) {
                event.emplace<OnKeyHold>(key);
            } else {
                event.emplace<OnKeyDown>(key);
            }
            isKeyHeld = true;
            break;
        }
        case WM_KEYUP: case WM_SYSKEYUP: {
            event.emplace<OnKeyUp>(key);
            isKeyHeld = false;
            break;
        }
        default: break;
    }

    {
        std::lock_guard lock { mutex };
        for (const auto instance : instances) {
            std::lock_guard instanceLock { instance->mutex };
            instance->keyboardEvents.emplace_back(event);
        }
    }

    return next();
}

void KeyboardObserver::Hook::SetHook() {
    ThrowOnSystemError(hHook = SetWindowsHookEx(
        WH_KEYBOARD_LL, KeyboardProc, nullptr, 0
    ));
}

void KeyboardObserver::Hook::MessageLoop() noexcept {
    MSG msg {};
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

void KeyboardObserver::Hook::ClearHook(std::thread& thread) noexcept {
    const auto threadId = GetThreadId(thread.native_handle());
    PostThreadMessage(threadId, WM_QUIT, 0, 0);
    UnhookWindowsHookEx(hHook);
    hHook = nullptr;
}

KeyboardObserver::KeyboardObserver() try {
    Hook::Register(this);
} catch (const std::exception& e) {
    LOG_E("Failed to create KeyboardObserver: {}", e.what());
    throw;
}

KeyboardObserver::~KeyboardObserver() noexcept {
    Hook::Unregister(this);
}

void KeyboardObserver::Update() noexcept {
    std::lock_guard lock { mutex };
    for (const auto& event : keyboardEvents) {
        Notify(event);
    }
    keyboardEvents.clear();
}
