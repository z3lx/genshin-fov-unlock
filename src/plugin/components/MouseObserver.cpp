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
#include <utility>
#include <variant>

#include <Windows.h>

MouseObserver::MouseObserver(
    std::weak_ptr<IMediator<Event>> mediator) try
    : IComponent { std::move(mediator) }
    , isEnabled { false }
    , isPolling { false } {
    StartPolling();
} catch (const std::exception& e) {
    LOG_E("Failed to create MouseObserver: {}", e.what());
    throw;
}

MouseObserver::~MouseObserver() noexcept {
    StopPolling();
}

void MouseObserver::StartPolling() {
    std::lock_guard guard { mutex };
    if (isPolling) {
        return;
    }
    isPolling = true;
    thread = std::thread { [this] { PollingLoop(); } };
}

void MouseObserver::StopPolling() noexcept {
    std::lock_guard guard { mutex };
    if (!isPolling) {
        return;
    }
    isPolling = false;
    thread.join();
}

void MouseObserver::PollingLoop() noexcept {
    const auto poll = [this]() noexcept {
        std::lock_guard guard { mutex };

        if (!isEnabled) {
            isPreviousCursorVisible.reset();
            return;
        }

        CURSORINFO cursorinfo { .cbSize = sizeof(cursorinfo) };
        if (!GetCursorInfo(&cursorinfo)) {
            return;
        }
        const bool isCurrentCursorVisible =
            (cursorinfo.flags & CURSOR_SHOWING) != 0;

        if (isPreviousCursorVisible != isCurrentCursorVisible) {
            isPreviousCursorVisible = isCurrentCursorVisible;
            Notify(OnCursorVisibilityChange { isCurrentCursorVisible });
        }
    };

    while (isPolling) {
        poll();
        constexpr auto pollingInterval = // 60 hz
            std::chrono::duration<double, std::milli> { 1000.0 / 60.0 };
        std::this_thread::sleep_for(pollingInterval);
    }
}

bool MouseObserver::IsEnabled() const noexcept {
    return isEnabled;
}

std::optional<bool> MouseObserver::IsCursorVisible() const noexcept {
    return isPreviousCursorVisible;
}

void MouseObserver::SetEnabled(const bool value) noexcept {
    isEnabled = value;
}
