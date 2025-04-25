#pragma once

#include "plugin/Events.hpp"
#include "plugin/interfaces/IComponent.hpp"
#include "plugin/interfaces/IMediator.hpp"

#include <memory>
#include <mutex>
#include <thread>

#include <Windows.h>

class WinEventNotifier final : public IComponent<Event> {
public:
    explicit WinEventNotifier(std::weak_ptr<IMediator<Event>> mediator);
    ~WinEventNotifier() noexcept override;

    [[nodiscard]] bool IsEnabled() const noexcept;
    void SetEnabled(bool value) noexcept;

private:
    void StartPolling();
    void StopPolling() noexcept;
    void PollingLoop() noexcept;

    bool isEnabled;
    bool isPolling;
    HWND previousForegroundWindow;

    std::mutex mutex;
    std::thread thread;
};
