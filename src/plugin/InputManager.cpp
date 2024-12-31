#include "plugin/InputManager.h"
#include "plugin/Events.h"
#include "plugin/IComponent.h"
#include "plugin/IMediator.h"
#include "utils/log/Logger.h"

#include <algorithm>
#include <future>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <thread>
#include <variant>
#include <vector>

#include <Windows.h>

HHOOK InputManager::hHook = nullptr;
std::mutex InputManager::mutex {};
std::vector<InputManager*> InputManager::instances {};

InputManager::InputManager(
    const std::weak_ptr<IMediator<Event>>& mediator,
    const std::vector<HWND>& targetWindows) try
    : IComponent(mediator), targetWindows(targetWindows) {
    LOG_D("Initializing InputManager");
    std::lock_guard lock { mutex };

    std::promise<void> promise {};
    std::future<void> future = promise.get_future();
    std::thread {[&promise, this]() {
        if (hHook) {
            return;
        }
        try { hHook = SetHook(); } catch (...) {
            promise.set_exception(std::current_exception());
            return;
        }
        promise.set_value();
        ProcessMessages();
    }}.detach();
    future.get();

    instances.push_back(this);
    LOG_I("InputManager initialized");
} catch (const std::exception& e) {
    LOG_E("Failed to initialize InputManager: {}", e.what());
    throw;
}

InputManager::~InputManager() try {
    LOG_D("Uninitializing InputManager");
    std::lock_guard lock { mutex };
    std::erase(instances, this);
    if (instances.empty()) {
        RemoveHook(hHook);
    }
    LOG_I("InputManager uninitialized");
} catch (const std::exception& e) {
    LOG_E("Failed to uninitialize InputManager: {}", e.what());
    throw;
}

HHOOK InputManager::SetHook() {
    HHOOK handle = SetWindowsHookEx(
        WH_KEYBOARD_LL, KeyboardProc, nullptr, 0
    );
    if (!handle) {
        throw std::runtime_error {
            std::to_string(GetLastError())
        };
    }
    return handle;
}

void InputManager::RemoveHook(HHOOK& handle) noexcept {
    UnhookWindowsHookEx(handle);
    handle = nullptr;
}

void InputManager::ProcessMessages() noexcept {
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

LRESULT CALLBACK InputManager::KeyboardProc(
    const int nCode, const WPARAM wParam, const LPARAM lParam) noexcept {
    if (nCode == HC_ACTION) {
        const auto pKeyboard = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);
        if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
            Notify(OnKeyDown { static_cast<int>(pKeyboard->vkCode) });
        } else if (wParam == WM_KEYUP || wParam == WM_SYSKEYUP) {
            Notify(OnKeyUp { static_cast<int>(pKeyboard->vkCode) });
        }
    }
    return CallNextHookEx(hHook, nCode, wParam, lParam);
}

void InputManager::Notify(const Event& event) noexcept try {
    std::lock_guard lock { mutex };
    HWND currentWindow = GetForegroundWindow();
    for (const auto& instance : instances) {
        const auto mediator = instance->weakMediator.lock();
        if (!mediator) {
            LOG_E("Mediator is expired");
            continue;
        }
        if (const auto targets = instance->targetWindows; !targets.empty() &&
            std::ranges::find(targets, currentWindow) == targets.end()) {
            continue;
        }
        try {
            mediator->Notify(event);
        } catch (const std::exception& e) {
            LOG_E("Failed to notify instance: {}", e.what());
        }
    }
} catch (const std::exception& e) {
    LOG_E("Failed to notify instances: {}", e.what());
}

void InputManager::Handle(const Event& event) {
    std::visit(Visitor { *this }, event);
}

template <typename T>
void InputManager::Visitor::operator()(const T& event) const { }
