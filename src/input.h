#pragma once

#include <functional>
#include <memory>
#include <windows.h>

class InputManager {
public:
    using KeyEventCallback = std::function<void(int)>;

    explicit InputManager(
        int pollingRate = 0,
        HWND window = nullptr
    );
    ~InputManager();

    void SetTrackedWindow(HWND window) const;
    void RegisterOnKeyDown(const KeyEventCallback& callback) const;
    void RegisterOnKeyUp(const KeyEventCallback& callback) const;

    void Poll() const;
    void StartPolling(int pollingRate) const;
    void StopPolling() const;

private:
    struct Impl;
    std::unique_ptr<Impl> impl;
};
