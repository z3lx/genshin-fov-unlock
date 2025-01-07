#pragma once

#include <memory>

template <typename T>
class IMediator;

template <typename T>
class IComponent {
public:
    explicit IComponent(std::weak_ptr<IMediator<T>> mediator)
        : weakMediator(mediator) { }
    IComponent() = delete;
    virtual ~IComponent() = default;

    virtual void Handle(const T& event) = 0;

protected:
    std::weak_ptr<IMediator<T>> weakMediator;
};
