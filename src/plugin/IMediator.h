#pragma once

#include "plugin/IComponent.h"

#include <memory>
#include <vector>

template <typename T>
class IMediator {
public:
    IMediator() = default;
    virtual ~IMediator() = default;

    void Notify(const T& event);

protected:
    std::vector<std::unique_ptr<IComponent<T>>> components;
};

template <typename T>
void IMediator<T>::Notify(const T& event) {
    for (const auto& component : components) {
        component->Handle(event);
    }
}
