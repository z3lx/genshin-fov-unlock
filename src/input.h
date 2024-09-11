#pragma once

#include <functional>
#include <windows.h>

class InputManager {
public:
    using KeyEventCallback = std::function<void(int)>;

    explicit InputManager(int pollingRate = 0, DWORD process = 0);
    ~InputManager();

    void SetTrackedProcess(DWORD process) const;
    void RegisterOnKeyDown(const KeyEventCallback& callback) const;
    void RegisterOnKeyUp(const KeyEventCallback& callback) const;

    void Poll() const;
    void StartPolling(int pollingRate) const;
    void StopPolling() const;

private:
    struct Impl;
    Impl* impl;
};
