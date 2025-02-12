#include "plugin/components/MouseObserver.hpp"
#include "plugin/Events.hpp"
#include "plugin/interfaces/IMediator.hpp"
#include "utils/log/Logger.hpp"

#include <chrono>
#include <exception>
#include <memory>
#include <mutex>
#include <optional>
#include <thread>
#include <unordered_set>
#include <variant>

#include <Windows.h>

std::unordered_set<MouseObserver*> MouseObserver::instances {};
std::mutex MouseObserver::mutex {};
bool MouseObserver::isPolling { false };
std::thread MouseObserver::pollingThread {};

void MouseObserver::StartPolling() {
    std::lock_guard guard { mutex };
    if (isPolling) {
        return;
    }
    isPolling = true;

    const auto loop = []() {
        while (isPolling) {
            Poll();
            constexpr auto pollingInterval = // 60 hz
                std::chrono::duration<double, std::milli> { 1000.0 / 60.0 };
            std::this_thread::sleep_for(pollingInterval);
        }
    };
    pollingThread = std::thread { loop };
}

void MouseObserver::StopPolling() noexcept {
    std::lock_guard guard { mutex };
    if (!isPolling) {
        return;
    }
    isPolling = false;

    if (pollingThread.joinable()) {
        pollingThread.join();
    }
}

// Will only notify if the cursor visibility changes
// Unfocusing a window is will interpret as the cursor being shown
// Will always notify the first time an instance is added
void MouseObserver::Poll() noexcept {
    CURSORINFO cursorinfo { .cbSize = sizeof(cursorinfo) };
    bool isCursorVisible {};
    if (GetCursorInfo(&cursorinfo)) {
        isCursorVisible = (cursorinfo.flags & CURSOR_SHOWING) != 0;
    } else {
        LOG_E("Failed to get cursor info");
    }
    HWND currentWindow = GetForegroundWindow();

    for (const auto& instance : instances) {
        if (!instance->isEnabled) {
            continue;
        }

        const auto isCurrentCursorVisible = isCursorVisible ||
            !instance->targetWindows.contains(currentWindow);
        auto& isPreviousCursorVisible = instance->isCursorVisible;
        if (isCurrentCursorVisible == isPreviousCursorVisible) {
            continue;
        }
        isPreviousCursorVisible = isCurrentCursorVisible;

        const auto mediator = instance->weakMediator.lock();
        if (!mediator) {
            LOG_E("Mediator is expired");
            continue;
        }
        try {
            mediator->Notify(OnCursorVisibilityChange {
                isCurrentCursorVisible
            });
        } catch (const std::exception& e) {
            LOG_E("Failed to process mouse event: {}", e.what());
        }
    }
}

MouseObserver::MouseObserver(
    const std::weak_ptr<IMediator<Event>>& mediator,
    const std::unordered_set<HWND>& targetWindows) try
    : IComponent { mediator }
    , isEnabled { false }
    , targetWindows { targetWindows } {
    // TODO: Add synchronization, handle exceptions
    instances.emplace(this);
    if (instances.size() == 1) {
        StartPolling();
    }
} catch (const std::exception& e) {
    LOG_E("Failed to create MouseObserver: {}", e.what());
    throw;
}

MouseObserver::~MouseObserver() noexcept {
    if (instances.size() == 1) {
        StopPolling();
    }
    instances.erase(this);
}

void MouseObserver::SetEnable(const bool value) noexcept {
    isEnabled = value;
}

void MouseObserver::Handle(const Event& event) {
    std::visit(Visitor { *this }, event);
}

template <typename T>
void MouseObserver::Visitor::operator()(const T&) const { }

template <>
void MouseObserver::Visitor::operator()(const OnPluginStart&) const {
    m.SetEnable(true);
}

template <>
void MouseObserver::Visitor::operator()(const OnPluginEnd&) const {
    m.SetEnable(false);
}
