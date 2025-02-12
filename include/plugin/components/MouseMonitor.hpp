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

class MouseMonitor final : public IComponent<Event> {
public:
    explicit MouseMonitor(
        const std::weak_ptr<IMediator<Event>>& mediator,
        const std::unordered_set<HWND>& targetWindows
    );
    ~MouseMonitor() noexcept override;

    void SetEnable(bool value) noexcept;

    void Handle(const Event& event) override;

private:
    bool isEnabled;
    std::optional<bool> isCursorVisible;
    std::unordered_set<HWND> targetWindows;

    struct Visitor;

    static std::unordered_set<MouseMonitor*> instances;
    static std::mutex mutex;

    static void StartPolling();
    static void StopPolling() noexcept;
    static void Poll() noexcept;
    static bool isPolling;
    static std::thread pollingThread;
};

struct MouseMonitor::Visitor {
    MouseMonitor& m;

    template <typename T>
    void operator()(const T& event) const;
};
