#pragma once

#include <algorithm>
#include <atomic>
#include <chrono>
#include <memory>
#include <stdexcept>
#include <thread>
#include <type_traits>
#include <utility>
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

template <typename Event>
IMediator<Event>::IMediator() {
    StartThread();
}

template <typename Event>
IMediator<Event>::~IMediator() noexcept {
    StopThread();
}

template <typename Event>
void IMediator<Event>::Start() noexcept {}

template <typename Event>
void IMediator<Event>::Update() noexcept {}

template <typename Event>
template <typename Component>
requires IsComponent<Component, Event>
Component* IMediator<Event>::TryGetComponent() const noexcept {
    Component* component = nullptr;
    if (const auto it = FindComponent<Component>(); it != components.end()) {
        component = dynamic_cast<Component*>(it->get());
    }
    return component;
}

template <typename Event>
template <typename Component>
requires IsComponent<Component, Event>
Component& IMediator<Event>::GetComponent() const {
    if (const auto component = TryGetComponent<Component>()) {
        return *component;
    }
    throw std::runtime_error { "Component not set" };
}

template <typename Event>
template <typename Component, typename... Args>
requires IsComponent<Component, Event>
void IMediator<Event>::SetComponent(Args&&... args) {
    auto component = std::make_unique<Component>(std::forward<Args>(args)...);
    component->SetMediator(this);
    component->Start();
    components.push_back(std::move(component));
}

template <typename Event>
template <typename Component>
requires IsComponent<Component, Event>
void IMediator<Event>::ClearComponent() {
    if (const auto it = FindComponent<Component>(); it != components.end()) {
        components.erase(it);
        return;
    }
    throw std::runtime_error { "Component not set" };
}

template <typename Event>
template <typename Component>
requires IsComponent<Component, Event>
typename IMediator<Event>::ComponentIterator
IMediator<Event>::FindComponent() noexcept {
    return std::ranges::find_if(components, [](const auto& component) {
        return dynamic_cast<Component*>(component.get());
    });
}

template <typename Event>
template <typename Component>
requires IsComponent<Component, Event>
typename IMediator<Event>::ConstComponentIterator
IMediator<Event>::FindComponent() const noexcept {
    return const_cast<IMediator*>(this)->FindComponent<Component>();
}

template <typename Event>
void IMediator<Event>::StartThread() {
    stopFlag.store(false);
    thread = std::thread([this]() {
        Start();
        while (!stopFlag.load()) {
            for (auto& component : components) {
                component->Update();
            }
            Update();
            // Wait until the next scheduler tick
            std::this_thread::sleep_for(
                std::chrono::milliseconds { 1 });
        }
    });
}

template <typename Event>
void IMediator<Event>::StopThread() noexcept {
    stopFlag.store(true);
    if (thread.joinable()) {
        thread.join();
    }
}
