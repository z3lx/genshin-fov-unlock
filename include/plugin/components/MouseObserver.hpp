#pragma once

#include "plugin/Events.hpp"
#include "plugin/interfaces/IComponent.hpp"

#include <optional>

class MouseObserver final : public IComponent<Event> {
public:
    MouseObserver() noexcept;
    ~MouseObserver() noexcept override;

private:
    void Update() noexcept override;

    std::optional<bool> isPreviousCursorVisible;
};
