#pragma once

#include "plugin/Events.hpp"
#include "plugin/interfaces/IComponent.hpp"
#include "plugin/interfaces/IMediator.hpp"

#include <memory>

class Unlocker final : public IComponent<Event> {
public:
    explicit Unlocker(std::weak_ptr<IMediator<Event>> mediator);
    ~Unlocker() noexcept override;

    void SetHook(bool value) const;
    void SetEnable(bool value) const noexcept;
    void SetFieldOfView(int value) noexcept;
    void SetSmoothing(float value) noexcept;
};
