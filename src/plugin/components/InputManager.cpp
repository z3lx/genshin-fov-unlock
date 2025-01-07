#include "plugin/components/InputManager.hpp"
#include "plugin/Events.hpp"
#include "plugin/interfaces/IComponent.hpp"
#include "plugin/interfaces/IMediator.hpp"
#include "utils/log/Logger.hpp"

#include <algorithm>
#include <future>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <vector>

#include <Windows.h>

HHOOK InputManager::hHook = nullptr;
std::mutex InputManager::mutex {};
std::unordered_set<InputManager*> InputManager::instances {};
std::unordered_map<int, bool> InputManager::keyStates {};

InputManager::InputManager(
    const std::weak_ptr<IMediator<Event>>& mediator,
    const std::vector<HWND>& targetWindows) try
    : IComponent(mediator), isEnabled(false), targetWindows(targetWindows) {
    std::lock_guard lock { mutex };
    instances.emplace(this);
    if (instances.size() == 1) {
        hHook = SetHook();
    }
} catch (const std::exception& e) {
    LOG_E("Failed to create InputManager: {}", e.what());
    instances.erase(this);
    throw;
}

InputManager::~InputManager() noexcept {
    try {
        std::lock_guard lock { mutex };
        instances.erase(this);
        if (instances.empty()) {
            RemoveHook(hHook);
        }
    } catch (const std::exception& e) {
        // Hook automatically removed when DLL is unloaded
        // Ignore exception here
        // LOG_E("Failed to destroy InputManager: {}", e.what());
    }
}

void InputManager::SetEnable(const bool value) noexcept {
    isEnabled = value;
}

HHOOK InputManager::SetHook() {
    HHOOK handle = nullptr;
    std::promise<void> promise {};
    std::future<void> future = promise.get_future();
    std::thread {[&handle, &promise]() {
        handle = SetWindowsHookEx(
            WH_KEYBOARD_LL, KeyboardProc, nullptr, 0
        );
        if (!handle) {
            promise.set_exception(std::make_exception_ptr(
                std::runtime_error { std::to_string(GetLastError()) }
            ));
            return;
        }
        promise.set_value();

        MSG msg;
        while (GetMessage(&msg, nullptr, 0, 0)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }}.detach();
    future.get();
    return handle;
}

void InputManager::RemoveHook(HHOOK& handle) {
    if (!UnhookWindowsHookEx(handle)) {
        throw std::runtime_error { std::to_string(GetLastError()) };
    }
    handle = nullptr;
}

LRESULT CALLBACK InputManager::KeyboardProc(
    const int nCode, const WPARAM wParam, const LPARAM lParam) noexcept {
    if (nCode != HC_ACTION) {
        return CallNextHookEx(hHook, nCode, wParam, lParam);
    }

    try {
        const auto pKeyboard = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);
        const auto vKey = static_cast<int>(pKeyboard->vkCode);
        Event event {};
        if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
            if (keyStates[vKey]) {
                event.emplace<OnKeyHold>(vKey);
            } else {
                event.emplace<OnKeyDown>(vKey);
            }
            keyStates[vKey] = true;
        } else if (wParam == WM_KEYUP || wParam == WM_SYSKEYUP) {
            event.emplace<OnKeyUp>(vKey);
            keyStates[vKey] = false;
        }
        Notify(event);
    } catch (const std::exception& e) {
        LOG_E("Failed to process keyboard event: {}", e.what());
    }
    return CallNextHookEx(hHook, nCode, wParam, lParam);
}

void InputManager::Notify(const Event& event) noexcept try {
    std::lock_guard lock { mutex };
    HWND currentWindow = GetForegroundWindow();
    for (const auto& instance : instances) {
        if (!instance->isEnabled) {
            continue;
        }
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
void InputManager::Visitor::operator()(const OnPluginStart& event) const try {
    LOG_D("Handling OnPluginStart event");
    m.SetEnable(true);
    LOG_D("OnPluginStart event handled");
} catch (const std::exception& e) {
    LOG_E("Failed to handle OnPluginStart event: {}", e.what());
}

template <>
void InputManager::Visitor::operator()(const OnPluginEnd& event) const try {
    LOG_D("Handling OnPluginEnd event");
    m.SetEnable(false);
    LOG_D("OnPluginEnd event handled");
} catch (const std::exception& e) {
    LOG_E("Failed to handle OnPluginEnd event: {}", e.what());
}
