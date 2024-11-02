#include "input.h"

#include <atomic>
#include <chrono>
#include <functional>
#include <initializer_list>
#include <mutex>
#include <set>
#include <stdexcept>
#include <thread>
#include <unordered_map>
#include <vector>

#include <windows.h>

InputManager::InputManager(const DWORD process) noexcept
    : _trackedProcess(process)
    , _isPolling(false) { }

InputManager::~InputManager() noexcept {
    StopPolling();
}

void InputManager::SetTrackedProcess(const DWORD process) noexcept {
    std::lock_guard lock(_dataMutex);
    _trackedProcess = process;
}

void InputManager::RegisterKeys(const std::initializer_list<int> vKeys) {
    std::lock_guard lock(_dataMutex);
    for (const auto& key : vKeys) {
        _registeredKeys.insert(key);
    }
}

void InputManager::RegisterOnKeyDown(const KeyEventCallback& callback) {
    std::lock_guard lock(_dataMutex);
    _onKeyDownCallbacks.push_back(callback);
}

void InputManager::RegisterOnKeyUp(const KeyEventCallback& callback) {
    std::lock_guard lock(_dataMutex);
    _onKeyUpCallbacks.push_back(callback);
}

void InputManager::Poll() noexcept {
    std::lock_guard lock(_dataMutex);

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
    if (pollingRate <= 0) {
        throw std::invalid_argument("Polling rate must be greater than 0");
    }

    if (bool expected = false;
        _isPolling.compare_exchange_strong(expected, true)) {
        std::lock_guard lock(_stateMutex);
        const auto pollingFn = &InputManager::PollingThread;
        _pollingThread = std::thread(pollingFn, this, pollingRate);
    }
}

void InputManager::StopPolling() noexcept {
    if (bool expected = true;
        _isPolling.compare_exchange_strong(expected, false)) {
        std::lock_guard lock(_stateMutex);
        if (_pollingThread.joinable()) {
            _pollingThread.join();
        }
    }
}

void InputManager::TriggerOnKeyDown(const int vKey) const noexcept {
    for (const auto& callback : _onKeyDownCallbacks) {
        try {
            callback(vKey);
        } catch (const std::exception& e) {
            // TODO: LOG ERROR
        }
    }
}

void InputManager::TriggerOnKeyUp(const int vKey) const noexcept {
    for (const auto& callback : _onKeyUpCallbacks) {
        try {
            callback(vKey);
        } catch (const std::exception& e) {
            // TODO: LOG ERROR
        }
    }
}

void InputManager::PollingThread(const int pollingRate) noexcept {
    const auto duration = std::chrono::duration<double, std::milli>(
        1000.0 / pollingRate);
    while (_isPolling) {
        Poll();
        std::this_thread::sleep_for(duration);
    }
}
