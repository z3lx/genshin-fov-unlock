#include "plugin/components/InputManager.hpp"
#include "plugin/Events.hpp"
#include "plugin/interfaces/IComponent.hpp"
#include "plugin/interfaces/IMediator.hpp"
#include "utils/WinHook.hpp"
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

#include <Windows.h>

namespace {
    std::mutex mutex {};
    std::unordered_set<InputManager*> instances {};
    std::unique_ptr<WinHook> keyboardHook {};
    std::unordered_map<int, bool> keyStates {};
}

InputManager::InputManager(
    const std::weak_ptr<IMediator<Event>>& mediator,
    const std::unordered_set<HWND>& targetWindows) try
    : IComponent { mediator }
    , isEnabled { false }
    , targetWindows { targetWindows } {
    std::lock_guard lock { mutex };
    if (keyboardHook == nullptr) {
        keyboardHook = std::make_unique<WinHook>(
            HookProcedure::WhKeyboardLl, KeyboardProc
        );
    }
    instances.emplace(this);
} catch (const std::exception& e) {
    LOG_E("Failed to create InputManager: {}", e.what());
    throw;
}

InputManager::~InputManager() noexcept {
    std::lock_guard lock { mutex };
    instances.erase(this);
    if (instances.empty()) {
        keyboardHook = nullptr;
    }
}

void InputManager::SetEnable(const bool value) noexcept {
    isEnabled = value;
}

LRESULT CALLBACK InputManager::KeyboardProc(
    const int nCode, const WPARAM wParam, const LPARAM lParam) noexcept {
    const auto hHook = keyboardHook ? keyboardHook->GetHandle() : nullptr;
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
        if (!instance->isEnabled ||
            !instance->targetWindows.contains(currentWindow)) {
            continue;
        }
        const auto mediator = instance->weakMediator.lock();
        if (!mediator) {
            LOG_E("Mediator is expired");
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
