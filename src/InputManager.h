#pragma once

#include "Events.h"
#include "IComponent.h"
#include "IMediator.h"

#include <atomic>
#include <memory>
#include <mutex>
#include <set>
#include <thread>
#include <unordered_map>

#include <Windows.h>

class InputManager final : public IComponent<Event> {
public:
    explicit InputManager(
        const std::weak_ptr<IMediator<Event>>& mediator) noexcept;
    ~InputManager() noexcept override;

    void SetTrackedProcess(DWORD process) noexcept;
    void RegisterKeys(const std::vector<int>& vKeys);

    void Poll() noexcept;
    void StartPolling(int pollingRate);
    void StopPolling() noexcept;

    void Handle(const Event& event) override;

private:
    struct Visitor {
        InputManager& m;

        template <typename T>
        void operator()(const T& event) const;
    };

    void PollingThread(int pollingRate) noexcept;

    DWORD trackedProcess;
    std::set<int> registeredKeys;
    std::unordered_map<int, bool> keyStates;

    std::atomic<bool> isPolling;
    std::thread pollingThread;
    std::mutex dataMutex;
    std::mutex stateMutex;
};
