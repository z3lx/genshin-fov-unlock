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

    explicit InputManager(DWORD process = 0);
    ~InputManager();

    void SetTrackedProcess(DWORD process);
    void RegisterKeys(std::initializer_list<int> vKeys);
    void RegisterOnKeyDown(const KeyEventCallback& callback);
    void RegisterOnKeyUp(const KeyEventCallback& callback);

    void Poll();
    void StartPolling(int pollingRate);
    void StopPolling();

private:
    void TriggerOnKeyDown(int vKey) const;
    void TriggerOnKeyUp(int vKey) const;
    void PollingThread(int pollingRate);

    DWORD _trackedProcess;
    std::set<int> _registeredKeys;
    std::unordered_map<int, bool> _keyStates;
    std::vector<KeyEventCallback> _onKeyDownCallbacks;
    std::vector<KeyEventCallback> _onKeyUpCallbacks;

    std::atomic<bool> _isPolling;
    std::thread _pollingThread;
    std::mutex _pollingMutex;
};
