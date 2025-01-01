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
    const std::vector<HWND>& targetWindows) noexcept
    : IComponent(mediator), targetWindows(targetWindows) { }

InputManager::~InputManager() noexcept = default;

bool InputManager::IsHooked() const noexcept {
    return hHook && std::ranges::find(instances, this) != instances.end();
}

void InputManager::Hook(const bool value) {
    std::lock_guard lock { mutex };

    if (!value) {
        std::erase(instances, this);
        if (instances.empty()) {
            RemoveHook(hHook);
        }
        return;
    }

    if (std::ranges::find(instances, this) != instances.end()) {
        return;
    }

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

template <>
void InputManager::Visitor::operator()(const OnPluginInitialize& event) const try {
    LOG_D("Handling OnPluginInitialize event");
    m.Hook(true);
    LOG_D("OnPluginInitialize event handled");
} catch (const std::exception& e) {
    LOG_E("Failed to handle OnPluginInitialize event: {}", e.what());
}

template <>
void InputManager::Visitor::operator()(const OnPluginUninitialize& event) const try {
    LOG_D("Handling OnPluginUninitialize event");
    m.Hook(false);
    LOG_D("OnPluginUninitialize event handled");
} catch (const std::exception& e) {
    LOG_E("Failed to handle OnPluginUninitialize event: {}", e.what());
}
