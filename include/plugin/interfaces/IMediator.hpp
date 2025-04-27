#pragma once

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
    requires std::derived_from<Component, IComponent<Event>>
    [[nodiscard]] Component& GetComponent() const;

    template <typename Component, typename... Args>
    requires std::derived_from<Component, IComponent<Event>>
    void SetComponent(Args&&... args);

private:
    void StartThread();
    void StopThread() noexcept;

    std::atomic<bool> stopFlag;
    std::thread thread;
    std::vector<std::unique_ptr<IComponent<Event>>> components;
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
requires std::derived_from<Component, IComponent<Event>>
Component& IMediator<Event>::GetComponent() const {
    for (const auto& component : components) {
        if (auto casted = dynamic_cast<Component*>(component.get())) {
            return *casted;
        }
    }
    throw std::runtime_error { "Component not set" };
}

template <typename Event>
template <typename Component, typename... Args>
requires std::derived_from<Component, IComponent<Event>>
void IMediator<Event>::SetComponent(Args&&... args) {
    auto component = std::make_unique<Component>(std::forward<Args>(args)...);
    component->SetMediator(this);
    component->Start();
    components.push_back(std::move(component));
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
            std::this_thread::sleep_for(std::chrono::milliseconds { 100 });
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
