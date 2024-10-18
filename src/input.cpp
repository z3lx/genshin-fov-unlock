#include "input.h"
#include <atomic>
#include <chrono>
#include <functional>
#include <initializer_list>
#include <mutex>
#include <set>
#include <thread>
#include <unordered_map>
#include <vector>
#include <windows.h>
#include <winuser.h>

struct InputManager::Impl {
    Impl();
    explicit Impl(DWORD process);
    ~Impl();

    // Public Methods
    void SetTrackedProcess(DWORD process);
    void RegisterKeys(std::initializer_list<int> vKeys);
    void RegisterOnKeyDown(const KeyEventCallback& callback);
    void RegisterOnKeyUp(const KeyEventCallback& callback);

    void Poll();
    void StartPolling(int pollingRate);
    void StopPolling();

    // Private Methods
    void TriggerOnKeyDown(int vKey) const;
    void TriggerOnKeyUp(int vKey) const;
    void PollingThread(int pollingRate);

    // Private Members
    DWORD trackedProcess;
    std::set<int> registeredKeys;
    std::unordered_map<int, bool> keyStates;
    std::vector<KeyEventCallback> onKeyDownCallbacks;
    std::vector<KeyEventCallback> onKeyUpCallbacks;

    std::atomic<bool> isPolling;
    std::thread pollingThread;
    std::mutex pollingMutex;
};

InputManager::Impl::Impl()
    : trackedProcess(0)
    , isPolling(false) { }

InputManager::Impl::Impl(const DWORD process)
    : Impl() {
    SetTrackedProcess(process);
}

InputManager::Impl::~Impl() {
    StopPolling();
}

void InputManager::Impl::SetTrackedProcess(const DWORD process) {
    std::lock_guard lock(pollingMutex);
    trackedProcess = process;
}

void InputManager::Impl::RegisterKeys(const std::initializer_list<int> vKeys) {
    std::lock_guard lock(pollingMutex);
    for (const auto& key : vKeys) {
        registeredKeys.insert(key);
    }
}

void InputManager::Impl::RegisterOnKeyDown(const KeyEventCallback& callback) {
    std::lock_guard lock(pollingMutex);
    onKeyDownCallbacks.push_back(callback);
}

void InputManager::Impl::RegisterOnKeyUp(const KeyEventCallback& callback) {
    std::lock_guard lock(pollingMutex);
    onKeyUpCallbacks.push_back(callback);
}

void InputManager::Impl::Poll() {
    std::lock_guard lock(pollingMutex);

    DWORD foregroundProcess;
    GetWindowThreadProcessId(GetForegroundWindow(), &foregroundProcess);
    if (trackedProcess != 0 &&
        trackedProcess != foregroundProcess) {
        return;
    }

    for (const auto key : registeredKeys) {
        if (GetAsyncKeyState(key) & 0x8000) {
            if (!keyStates[key]) {
                keyStates[key] = true;
                TriggerOnKeyDown(key);
            }
        } else {
            if (keyStates[key]) {
                keyStates[key] = false;
                TriggerOnKeyUp(key);
            }
        }
    }
}

void InputManager::Impl::StartPolling(const int pollingRate) {
    if (bool expected = false;
        isPolling.compare_exchange_strong(expected, true)) {
        pollingThread = std::thread(&Impl::PollingThread, this, pollingRate);
    }
}

void InputManager::Impl::StopPolling() {
    if (bool expected = true;
        isPolling.compare_exchange_strong(expected, false)) {
        pollingThread.join();
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

void InputManager::Impl::PollingThread(const int pollingRate) {
    const auto duration = std::chrono::duration<double, std::milli>(
        1000.0 / pollingRate
    );
    while (isPolling) {
        Poll();
        std::this_thread::sleep_for(duration);
    }
}

InputManager::InputManager(const DWORD process)
    : impl(new Impl(process)) { }

InputManager::~InputManager() {
    delete impl;
}

void InputManager::SetTrackedProcess(const DWORD process) {
    impl->SetTrackedProcess(process);
}

void InputManager::RegisterKeys(const std::initializer_list<int> vKeys) {
    impl->RegisterKeys(vKeys);
}

void InputManager::RegisterOnKeyDown(const KeyEventCallback& callback) {
    impl->RegisterOnKeyDown(callback);
}

void InputManager::RegisterOnKeyUp(const KeyEventCallback& callback) {
    impl->RegisterOnKeyUp(callback);
}

void InputManager::Poll() const {
    impl->Poll();
}

void InputManager::StartPolling(const int pollingRate) {
    impl->StartPolling(pollingRate);
}

void InputManager::StopPolling() {
    impl->StopPolling();
}
