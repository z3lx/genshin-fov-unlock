#pragma once

#include <functional>
#include <initializer_list>
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

    void Poll() const;
    void StartPolling(int pollingRate);
    void StopPolling();

private:
    struct Impl;
    Impl* impl;
};
