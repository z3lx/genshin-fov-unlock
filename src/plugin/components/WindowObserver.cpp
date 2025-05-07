#include "plugin/components/WindowObserver.hpp"
#include "plugin/Events.hpp"

#include <Windows.h>

WindowObserver::WindowObserver() noexcept
    : previousForegroundWindow { nullptr } {}

WindowObserver::~WindowObserver() noexcept = default;

void WindowObserver::Update() noexcept {
    if (const auto currentForegroundWindow = GetForegroundWindow();
        currentForegroundWindow != previousForegroundWindow) {
        previousForegroundWindow = currentForegroundWindow;
        Notify(OnForegroundWindowChange { currentForegroundWindow });
    }
}
