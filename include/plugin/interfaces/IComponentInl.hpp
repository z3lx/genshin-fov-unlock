#pragma once

#include "plugin/interfaces/IComponent.hpp"

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
        mediator->events.push_back(event);
    }
}
