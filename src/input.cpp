#include "input.h"
#include <atomic>
#include <chrono>
#include <mutex>
#include <thread>
#include <unordered_map>
#include <vector>
#include <windows.h>
#include <winuser.h>

struct InputManager::Impl {
public:
    Impl();
    Impl(int pollingRate, HWND window);
    ~Impl();

    void SetTrackedWindow(HWND window);
    void RegisterOnKeyDown(const KeyEventCallback& callback);
    void RegisterOnKeyUp(const KeyEventCallback& callback);

    void Poll();
    void StartPolling(int pollingRate);
    void StopPolling();

private:
    HWND trackedWindow;
    int pollingRate;
    std::atomic<bool> isPolling;
    std::thread pollingThread;
    std::mutex pollingMutex;

    std::unordered_map<int, bool> keyStates;
    std::vector<KeyEventCallback> onKeyDownCallbacks;
    std::vector<KeyEventCallback> onKeyUpCallbacks;

    void PollingLoop();

    void TriggerOnKeyDown(int vKey) const;
    void TriggerOnKeyUp(int vKey) const;
};

// Not thread-safe
InputManager::Impl::Impl(
) : trackedWindow(nullptr)
  , pollingRate(0)
  , isPolling(false) {
}

// Not thread-safe
InputManager::Impl::Impl(
    const int pollingRate,
    const HWND window
) : Impl() {
    SetTrackedWindow(window);
    StartPolling(pollingRate);
}

// Not thread-safe
InputManager::Impl::~Impl() {
    StopPolling();
}

void InputManager::Impl::SetTrackedWindow(const HWND window) {
    std::lock_guard lock(pollingMutex);
    trackedWindow = window;
}

void InputManager::Impl::RegisterOnKeyDown(
    const KeyEventCallback& callback
) {
    std::lock_guard lock(pollingMutex);
    onKeyDownCallbacks.push_back(callback);
}

void InputManager::Impl::RegisterOnKeyUp(
    const KeyEventCallback& callback
) {
    std::lock_guard lock(pollingMutex);
    onKeyUpCallbacks.push_back(callback);
}

void InputManager::Impl::Poll() {
    std::lock_guard lock(pollingMutex);
    if (trackedWindow != nullptr &&
        trackedWindow != GetForegroundWindow()) {
        return;
    }

    for (int vKey = 0x01; vKey <= 0xFE; ++vKey) {
        if (GetAsyncKeyState(vKey) & 0x8000) {
            if (!keyStates[vKey]) {
                keyStates[vKey] = true;
                TriggerOnKeyDown(vKey);
            }
        } else {
            if (keyStates[vKey]) {
                keyStates[vKey] = false;
                TriggerOnKeyUp(vKey);
            }
        }
    }
}

// Not thread-safe
void InputManager::Impl::StartPolling(const int pollingRate) {
    StopPolling();
    if (pollingRate <= 0) {
        return;
    }
    isPolling.store(true);
    this->pollingRate = pollingRate;
    pollingThread = std::thread(&Impl::PollingLoop, this);
}

// Not thread-safe
void InputManager::Impl::StopPolling() {
    isPolling.store(false);
    if (pollingThread.joinable()) {
        pollingThread.join();
    }
}

void InputManager::Impl::PollingLoop() {
    while (isPolling.load()) {
        Poll();
        const auto duration = std::chrono::milliseconds(
            static_cast<long long>(1000.0 / pollingRate)
        );
        std::this_thread::sleep_for(duration);
    }
}

void InputManager::Impl::TriggerOnKeyDown(const int vKey) const {
    for (const auto& callback : onKeyDownCallbacks) {
        callback(vKey);
    }
}

void InputManager::Impl::TriggerOnKeyUp(const int vKey) const {
    for (const auto& callback : onKeyUpCallbacks) {
        callback(vKey);
    }
}

InputManager::InputManager(
) : impl(std::make_unique<Impl>()) {
}

InputManager::InputManager(
    const int pollingRate,
    const HWND window
) : impl(std::make_unique<Impl>(pollingRate, window)) {
}

InputManager::~InputManager() = default;

void InputManager::SetTrackedWindow(const HWND window) const {
    impl->SetTrackedWindow(window);
}

void InputManager::RegisterOnKeyDown(
    const KeyEventCallback& callback
) const {
    impl->RegisterOnKeyDown(callback);
}

void InputManager::RegisterOnKeyUp(
    const KeyEventCallback& callback
) const {
    impl->RegisterOnKeyUp(callback);
}

void InputManager::Poll() const {
    impl->Poll();
}

void InputManager::StartPolling(const int pollingRate) const {
    impl->StartPolling(pollingRate);
}

void InputManager::StopPolling() const {
    impl->StopPolling();
}
