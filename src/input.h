#pragma once

#include <atomic>
#include <functional>
#include <initializer_list>
#include <mutex>
#include <set>
#include <thread>
#include <unordered_map>
#include <vector>

#include <windows.h>

class InputManager {
public:
    using KeyEventCallback = std::function<void(int)>;

    explicit InputManager() noexcept;
    ~InputManager() noexcept;

    void SetTrackedProcess(DWORD process) noexcept;
    void RegisterKeys(std::initializer_list<int> vKeys);
    void RegisterOnKeyDown(const KeyEventCallback& callback);
    void RegisterOnKeyUp(const KeyEventCallback& callback);

    void Poll() noexcept;
    void StartPolling(int pollingRate);
    void StopPolling() noexcept;

private:
    void TriggerOnKeyDown(int vKey) const noexcept;
    void TriggerOnKeyUp(int vKey) const noexcept;
    void PollingThread(int pollingRate) noexcept;

    DWORD trackedProcess;
    std::set<int> registeredKeys;
    std::unordered_map<int, bool> keyStates;
    std::vector<KeyEventCallback> onKeyDownCallbacks;
    std::vector<KeyEventCallback> onKeyUpCallbacks;

    std::atomic<bool> isPolling;
    std::thread pollingThread;
    std::mutex dataMutex;
    std::mutex stateMutex;
};
