#pragma once

#include "plugin/Events.hpp"
#include "plugin/interfaces/IComponent.hpp"

#include <Windows.h>

class WinEventNotifier final : public IComponent<Event> {
public:
    WinEventNotifier() noexcept;
    ~WinEventNotifier() noexcept override;

private:
    void Update() noexcept override;

    HWND previousForegroundWindow;
};
