#pragma once

#include <memory>
#include <utility>

template <typename Event>
class IMediator;

template <typename Event>
class IComponent {
public:
    explicit IComponent(std::weak_ptr<IMediator<Event>> mediator) noexcept;
    IComponent() = delete;
    virtual ~IComponent() noexcept = default;

    virtual void Handle(const Event& event) noexcept = 0;

protected:
    std::weak_ptr<IMediator<Event>> weakMediator;
};

template <typename Event>
IComponent<Event>::IComponent(std::weak_ptr<IMediator<Event>> mediator) noexcept
    : weakMediator { std::move(mediator) } { }
