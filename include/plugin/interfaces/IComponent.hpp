#pragma once

#include "plugin/interfaces/IMediator.hpp"

template <typename Event>
class IComponent {
    friend class IMediator<Event>;
public:
    IComponent() noexcept = default;
    virtual ~IComponent() noexcept = default;

protected:
    virtual void Start() noexcept;
    virtual void Update() noexcept;
    void Notify(const Event& event) noexcept;

private:
    [[nodiscard]] IMediator<Event>* GetMediator() const noexcept;
    void SetMediator(IMediator<Event>* mediator) noexcept;

    IMediator<Event>* mediator;
};

template <typename Event>
void IComponent<Event>::Start() noexcept {}

template <typename Event>
void IComponent<Event>::Update() noexcept {}

template <typename Event>
IMediator<Event>* IComponent<Event>::GetMediator() const noexcept {
    return mediator;
}

template <typename Event>
void IComponent<Event>::SetMediator(IMediator<Event>* mediator) noexcept {
    this->mediator = mediator;
}

template <typename Event>
void IComponent<Event>::Notify(const Event& event) noexcept {
    if (mediator) {
        mediator->Notify(event);
    }
}
