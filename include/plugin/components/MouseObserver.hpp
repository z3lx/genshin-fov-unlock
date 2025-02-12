#pragma once

#include "plugin/Events.hpp"
#include "plugin/interfaces/IComponent.hpp"
#include "plugin/interfaces/IMediator.hpp"
#include "utils/VehHook.hpp"

#include <memory>
#include <mutex>
#include <optional>
#include <thread>
#include <unordered_set>

class MouseObserver final : public IComponent<Event> {
public:
    explicit MouseObserver(
        const std::weak_ptr<IMediator<Event>>& mediator,
        const std::unordered_set<HWND>& targetWindows
    );
    ~MouseObserver() noexcept override;

    void SetEnable(bool value) noexcept;

    void Handle(const Event& event) override;

private:
    bool isEnabled;
    std::optional<bool> isCursorVisible;
    std::unordered_set<HWND> targetWindows;

    struct Visitor;

    static std::unordered_set<MouseObserver*> instances;
    static std::mutex mutex;

    static void StartPolling();
    static void StopPolling() noexcept;
    static void Poll() noexcept;
    static bool isPolling;
    static std::thread pollingThread;
};

struct MouseObserver::Visitor {
    MouseObserver& m;

    template <typename T>
    void operator()(const T& event) const;
};
