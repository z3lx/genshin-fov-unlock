#pragma once

#include "plugin/Events.hpp"
#include "plugin/interfaces/IComponent.hpp"
#include "plugin/interfaces/IMediator.hpp"

#include <memory>
#include <mutex>
#include <optional>
#include <thread>

class MouseObserver final : public IComponent<Event> {
public:
    explicit MouseObserver(std::weak_ptr<IMediator<Event>> mediator);
    ~MouseObserver() noexcept override;

    [[nodiscard]] bool IsEnabled() const noexcept;
    [[nodiscard]] std::optional<bool> IsCursorVisible() const noexcept;
    void SetEnabled(bool value) noexcept;

    void Handle(const Event& event) noexcept override;

private:
    void StartPolling();
    void StopPolling() noexcept;
    void PollingLoop() noexcept;

    bool isEnabled;
    bool isPolling;
    std::optional<bool> isPreviousCursorVisible;

    std::mutex mutex;
    std::thread thread;
};
