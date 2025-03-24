#pragma once

#include "plugin/Events.hpp"
#include "plugin/interfaces/IComponent.hpp"
#include "plugin/interfaces/IMediator.hpp"

#include <memory>

class WinEventNotifier final : public IComponent<Event> {
public:
    explicit WinEventNotifier(std::weak_ptr<IMediator<Event>> mediator);
    ~WinEventNotifier() noexcept override;

    [[nodiscard]] bool IsEnabled() const noexcept;
    void SetEnabled(bool value) noexcept;

private:
    bool isEnabled;
};
