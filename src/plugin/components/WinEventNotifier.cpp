#include "plugin/components/WinEventNotifier.hpp"
#include "plugin/Events.hpp"
#include "plugin/interfaces/IMediator.hpp"
#include "utils/ThreadWrapper.hpp"
#include "utils/Windows.hpp"
#include "utils/log/Logger.hpp"

#include <chrono>
#include <memory>
#include <mutex>
#include <optional>
#include <thread>
#include <utility>
#include <variant>

#include <Windows.h>

WinEventNotifier::WinEventNotifier(
    std::weak_ptr<IMediator<Event>> mediator) try
    : IComponent { std::move(mediator) }
    , isEnabled { false }
    , isPolling { false }
    , previousForegroundWindow { nullptr } {
    StartPolling();
} catch (const std::exception& e) {
    LOG_E("Failed to create WinEventNotifier: {}", e.what());
    throw;
}

WinEventNotifier::~WinEventNotifier() noexcept {
    StopPolling();
}

void WinEventNotifier::StartPolling() {
    std::lock_guard guard { mutex };
    if (isPolling) {
        return;
    }
    isPolling = true;
    thread = std::thread { [this] { PollingLoop(); } };
}

void WinEventNotifier::StopPolling() noexcept {
    std::lock_guard guard { mutex };
    if (!isPolling) {
        return;
    }
    isPolling = false;
    thread.join();
}

void WinEventNotifier::PollingLoop() noexcept {
    const auto poll = [this]() noexcept {
        std::lock_guard guard { mutex };

        if (!isEnabled) {
            return;
        }

        if (const auto foregroundWindow = GetForegroundWindow();
            foregroundWindow != previousForegroundWindow) {
            previousForegroundWindow = foregroundWindow;
            Notify(OnForegroundWindowChange { foregroundWindow });
        }
    };

    while (isPolling) {
        poll();
        constexpr auto pollingInterval = // 60 hz
            std::chrono::duration<double, std::milli> { 1000.0 / 60.0 };
        std::this_thread::sleep_for(pollingInterval);
    }
}

bool WinEventNotifier::IsEnabled() const noexcept {
    return isEnabled;
}

void WinEventNotifier::SetEnabled(const bool value) noexcept {
    isEnabled = value;
}
