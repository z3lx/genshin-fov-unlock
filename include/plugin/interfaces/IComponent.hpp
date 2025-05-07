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

#include "plugin/interfaces/IComponentInl.hpp"
