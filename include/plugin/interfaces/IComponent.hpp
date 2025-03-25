#pragma once

#include <memory>

template <typename Event>
class IMediator;

template <typename Event>
class IComponent {
public:
    using MediatorPtr = std::weak_ptr<IMediator<Event>>;

    explicit IComponent(MediatorPtr mediator) noexcept;
    IComponent() noexcept = default;
    virtual ~IComponent() noexcept = default;

    [[nodiscard]] MediatorPtr GetMediator() const noexcept;
    void SetMediator(MediatorPtr mediator) noexcept;

protected:
    void Notify(const Event& event) noexcept;

private:
    MediatorPtr mediator;
};

template <typename Event>
IComponent<Event>::IComponent(MediatorPtr mediator) noexcept
    : mediator { mediator } { }

template <typename Event>
typename IComponent<Event>::MediatorPtr
IComponent<Event>::GetMediator() const noexcept {
    return mediator;
}

template <typename Event>
void IComponent<Event>::SetMediator(MediatorPtr mediator) noexcept {
    this->mediator = mediator;
}

template <typename Event>
void IComponent<Event>::Notify(const Event& event) noexcept {
    if (const auto mediator = GetMediator().lock()) {
        mediator->Notify(event);
    }
}
