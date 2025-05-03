#pragma once

#include <atomic>
#include <memory>
#include <thread>
#include <type_traits>
#include <vector>

template <typename Event>
class IComponent;

template <typename Component, typename Event>
concept IsComponent = std::derived_from<Component, IComponent<Event>>;

template <typename Event>
class IMediator {
    friend class IComponent<Event>;
public:
    IMediator();
    virtual ~IMediator() noexcept;

protected:
    virtual void Start() noexcept;
    virtual void Update() noexcept;
    virtual void Notify(const Event& event) noexcept = 0;

    template <typename Component>
    requires IsComponent<Component, Event>
    [[nodiscard]] Component* TryGetComponent() const noexcept;

    template <typename Component>
    requires IsComponent<Component, Event>
    [[nodiscard]] Component& GetComponent() const;

    template <typename Component, typename... Args>
    requires IsComponent<Component, Event>
    void SetComponent(Args&&... args);

    template <typename Component>
    requires IsComponent<Component, Event>
    void ClearComponent();

private:
    void StartThread();
    void StopThread() noexcept;

    std::atomic<bool> stopFlag;
    std::thread thread;
    std::vector<std::unique_ptr<IComponent<Event>>> components;
    std::vector<Event> events;

    using ComponentIterator =
        typename decltype(components)::iterator;
    using ConstComponentIterator =
        typename decltype(components)::const_iterator;

    template <typename Component>
    requires IsComponent<Component, Event>
    [[nodiscard]] ComponentIterator FindComponent() noexcept;

    template <typename Component>
    requires IsComponent<Component, Event>
    [[nodiscard]] ConstComponentIterator FindComponent() const noexcept;
};

#include "plugin/interfaces/IMediatorInl.hpp"
