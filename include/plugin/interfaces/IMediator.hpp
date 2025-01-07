#pragma once

#include "plugin/interfaces/IComponent.hpp"

#include <memory>
#include <vector>

template <typename T>
class IMediator {
public:
    IMediator() = default;
    virtual ~IMediator() = default;

    void Notify(const T& event);

protected:
    template <typename... Args>
    void AddComponents(Args&&... args);

private:
    std::vector<std::unique_ptr<IComponent<T>>> components;
};

template <typename T>
void IMediator<T>::Notify(const T& event) {
    for (const auto& component : components) {
        component->Handle(event);
    }
}

template <typename T>
template <typename... Args>
void IMediator<T>::AddComponents(Args&&... args) {
    (components.push_back(std::forward<Args>(args)), ...);
}
