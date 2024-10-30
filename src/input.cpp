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

InputManager::InputManager(const DWORD process)
    : _trackedProcess(process)
    , _isPolling(false) { }

InputManager::~InputManager() {
    StopPolling();
}

void InputManager::SetTrackedProcess(const DWORD process) {
    std::lock_guard lock(_pollingMutex);
    _trackedProcess = process;
}

void InputManager::RegisterKeys(const std::initializer_list<int> vKeys) {
    std::lock_guard lock(_pollingMutex);
    for (const auto& key : vKeys) {
        _registeredKeys.insert(key);
    }
}

void InputManager::RegisterOnKeyDown(const KeyEventCallback& callback) {
    std::lock_guard lock(_pollingMutex);
    _onKeyDownCallbacks.push_back(callback);
}

void InputManager::RegisterOnKeyUp(const KeyEventCallback& callback) {
    std::lock_guard lock(_pollingMutex);
    _onKeyUpCallbacks.push_back(callback);
}

void InputManager::Poll() {
    std::lock_guard lock(_pollingMutex);

    DWORD foregroundProcess;
    GetWindowThreadProcessId(GetForegroundWindow(), &foregroundProcess);
    if (_trackedProcess != 0 &&
        _trackedProcess != foregroundProcess) {
        return;
    }

    for (const auto key : _registeredKeys) {
        if (GetAsyncKeyState(key) & 0x8000) {
            if (!_keyStates[key]) {
                _keyStates[key] = true;
                TriggerOnKeyDown(key);
            }
        } else {
            if (_keyStates[key]) {
                _keyStates[key] = false;
                TriggerOnKeyUp(key);
            }
        }
    }
}

void InputManager::StartPolling(const int pollingRate) {
    if (bool expected = false;
        _isPolling.compare_exchange_strong(expected, true)) {
        const auto pollingFn = &InputManager::PollingThread;
        _pollingThread = std::thread(pollingFn, this, pollingRate);
    }
}

void InputManager::StopPolling() {
    if (bool expected = true;
        _isPolling.compare_exchange_strong(expected, false)) {
        _pollingThread.join();
    }
}

void InputManager::TriggerOnKeyDown(const int vKey) const {
    for (const auto& callback : _onKeyDownCallbacks) {
        callback(vKey);
    }
}

void InputManager::TriggerOnKeyUp(const int vKey) const {
    for (const auto& callback : _onKeyUpCallbacks) {
        callback(vKey);
    }
}

void InputManager::PollingThread(const int pollingRate) {
    const auto duration = std::chrono::duration<double, std::milli>(
        1000.0 / pollingRate);
    while (_isPolling) {
        Poll();
        std::this_thread::sleep_for(duration);
    }
}
