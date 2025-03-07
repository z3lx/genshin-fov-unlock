#include "plugin/components/KeyboardObserver.hpp"
#include "plugin/Events.hpp"
#include "plugin/interfaces/IComponent.hpp"
#include "plugin/interfaces/IMediator.hpp"
#include "utils/WinHook.hpp"
#include "utils/log/Logger.hpp"

#include <exception>
#include <memory>
#include <mutex>
#include <optional>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <variant>

#include <Windows.h>

namespace {
class KeyboardHook {
public:
    static void Register(KeyboardObserver* instance);
    static void Unregister(KeyboardObserver* instance) noexcept;

private:
    static LRESULT CALLBACK KeyboardProc(
        int nCode, WPARAM wParam, LPARAM lParam) noexcept;

    static inline std::mutex mutex {};
    static inline std::unordered_set<KeyboardObserver*> instances {};
    static inline std::optional<WinHook> keyboardHook {};
    static inline std::unordered_map<int, bool> keyHeldStates {};
};
} // namespace

void KeyboardHook::Register(KeyboardObserver* instance) try {
    std::lock_guard lock { mutex };
    instances.emplace(instance);
    if (!keyboardHook.has_value()) {
        keyboardHook.emplace(HookProcedure::WhKeyboardLl, KeyboardProc);
    }
} catch (...) {
    Unregister(instance);
    throw;
}

void KeyboardHook::Unregister(KeyboardObserver* instance) noexcept {
    std::lock_guard lock { mutex };
    instances.erase(instance);
    if (instances.empty()) {
        keyboardHook.reset();
    }
}

LRESULT CALLBACK KeyboardHook::KeyboardProc(
    const int nCode, const WPARAM wParam, const LPARAM lParam) noexcept {
    const auto hHook = keyboardHook ? keyboardHook->GetHandle() : nullptr;
    const auto next = [hHook, nCode, wParam, lParam]() noexcept {
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

    // Notify
    std::unique_lock lock { mutex };
    const auto currentWindow = GetForegroundWindow();
    for (const auto instance : instances) {
        if (!instance->IsEnabled() ||
            !instance->GetTargetWindows().contains(currentWindow)) {
            continue;
        }
        if (const auto mediator = instance->GetMediator().lock()) {
            mediator->Notify(event);
        }
    }
    lock.unlock();

    return next();
}

namespace {
struct Visitor {
    KeyboardObserver& instance;

    void operator()(const OnPluginStart& event) const noexcept;
    void operator()(const OnPluginEnd& event) const noexcept;
    template <typename T> void operator()(const T& event) const noexcept;
};
} // namespace

void Visitor::operator()(const OnPluginStart& event) const noexcept {
    instance.SetEnabled(true);
}

void Visitor::operator()(const OnPluginEnd& event) const noexcept {
    instance.SetEnabled(false);
}

template <typename T>
void Visitor::operator()(const T& event) const noexcept { }

KeyboardObserver::KeyboardObserver(
    std::weak_ptr<IMediator<Event>> mediator,
    const std::unordered_set<HWND>& targetWindows) try
    : IComponent { std::move(mediator) }
    , isEnabled { false }
    , targetWindows { targetWindows } {
    KeyboardHook::Register(this);
} catch (const std::exception& e) {
    LOG_E("Failed to create KeyboardObserver: {}", e.what());
    throw;
}

KeyboardObserver::~KeyboardObserver() noexcept {
    KeyboardHook::Unregister(this);
}

bool KeyboardObserver::IsEnabled() const noexcept {
    return isEnabled;
}

const std::unordered_set<HWND>&
    KeyboardObserver::GetTargetWindows() const noexcept {
    return targetWindows;
}

void KeyboardObserver::SetEnabled(const bool value) noexcept {
    isEnabled = value;
}

void KeyboardObserver::SetTargetWindows(
    const std::unordered_set<HWND>& windows) noexcept {
    targetWindows = windows;
}

void KeyboardObserver::Handle(const Event& event) noexcept {
    std::visit(Visitor { *this }, event);
}
