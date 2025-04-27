#include "plugin/components/WinEventNotifier.hpp"
#include "plugin/Events.hpp"

#include <Windows.h>

WinEventNotifier::WinEventNotifier() noexcept
    : previousForegroundWindow { nullptr } {}

WinEventNotifier::~WinEventNotifier() noexcept = default;

void WinEventNotifier::Update() noexcept {
    if (const auto currentForegroundWindow = GetForegroundWindow();
        currentForegroundWindow != previousForegroundWindow) {
        previousForegroundWindow = currentForegroundWindow;
        Notify(OnForegroundWindowChange { currentForegroundWindow });
    }
}
