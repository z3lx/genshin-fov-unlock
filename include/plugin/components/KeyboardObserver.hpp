#pragma once

#include "plugin/Events.hpp"
#include "plugin/interfaces/IComponent.hpp"
#include "plugin/interfaces/IMediator.hpp"

#include <memory>

class KeyboardObserver final : public IComponent<Event> {
public:
    explicit KeyboardObserver(std::weak_ptr<IMediator<Event>> mediator);
    ~KeyboardObserver() noexcept override;

    [[nodiscard]] bool IsEnabled() const noexcept;
    void SetEnabled(bool value) noexcept;

private:
    bool isEnabled;
};
