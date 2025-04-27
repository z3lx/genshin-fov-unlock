#pragma once

#include "plugin/Events.hpp"
#include "plugin/interfaces/IComponent.hpp"

#include <Windows.h>

class WindowObserver final : public IComponent<Event> {
public:
    WindowObserver() noexcept;
    ~WindowObserver() noexcept override;

private:
    void Update() noexcept override;

    HWND previousForegroundWindow;
};
