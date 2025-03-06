#pragma once

#include "plugin/interfaces/IComponent.hpp"

#include <memory>
#include <utility>
#include <vector>

template <typename Event>
class IMediator {
public:
    IMediator() noexcept = default;
    virtual ~IMediator() noexcept = default;

    void Notify(const Event& event) noexcept;

protected:
    void AddComponent(std::unique_ptr<IComponent<Event>> component);
    template <typename... Components>
    void AddComponents(Components&&... components);

private:
    std::vector<std::unique_ptr<IComponent<Event>>> components;
};

template <typename Event>
void IMediator<Event>::Notify(const Event& event) noexcept {
    for (const auto& component : components) {
        component->Handle(event);
    }
}

template <typename Event>
void IMediator<Event>::AddComponent(std::unique_ptr<IComponent<Event>> component) {
    components.push_back(std::move(component));
}

template <typename Event>
template <typename... Components>
void IMediator<Event>::AddComponents(Components&&... components) {
    (AddComponent(std::forward<Components>(components)), ...);
}
