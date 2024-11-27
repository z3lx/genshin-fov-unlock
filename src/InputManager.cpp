#include "InputManager.h"

#include <atomic>
#include <chrono>
#include <exception>
#include <functional>
#include <initializer_list>
#include <mutex>
#include <set>
#include <stdexcept>
#include <thread>
#include <unordered_map>
#include <vector>

#include <windows.h>

InputManager::InputManager() noexcept
    : trackedProcess(0)
    , isPolling(false) { }

InputManager::~InputManager() noexcept {
    StopPolling();
}

void InputManager::SetTrackedProcess(const DWORD process) noexcept {
    std::lock_guard lock(dataMutex);
    trackedProcess = process;
}

void InputManager::RegisterKeys(const std::initializer_list<int> vKeys) {
    std::lock_guard lock(dataMutex);
    for (const auto& key : vKeys) {
        registeredKeys.insert(key);
    }
}

void InputManager::RegisterOnKeyDown(const KeyEventCallback& callback) {
    std::lock_guard lock(dataMutex);
    onKeyDownCallbacks.push_back(callback);
}

void InputManager::RegisterOnKeyUp(const KeyEventCallback& callback) {
    std::lock_guard lock(dataMutex);
    onKeyUpCallbacks.push_back(callback);
}

void InputManager::Poll() noexcept {
    std::lock_guard lock(dataMutex);

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

void InputManager::StartPolling(const int pollingRate) {
    if (pollingRate <= 0) {
        throw std::invalid_argument("Polling rate must be greater than 0");
    }

    if (bool expected = false;
        isPolling.compare_exchange_strong(expected, true)) {
        std::lock_guard lock(stateMutex);
        const auto pollingFn = &InputManager::PollingThread;
        pollingThread = std::thread(pollingFn, this, pollingRate);
    }
}

void InputManager::StopPolling() noexcept {
    if (bool expected = true;
        isPolling.compare_exchange_strong(expected, false)) {
        std::lock_guard lock(stateMutex);
        if (pollingThread.joinable()) {
            pollingThread.join();
        }
    }
}

void InputManager::TriggerOnKeyDown(const int vKey) const noexcept {
    for (const auto& callback : onKeyDownCallbacks) {
        try {
            callback(vKey);
        } catch (const std::exception& e) {
            // TODO: LOG ERROR
        }
    }
}

void InputManager::TriggerOnKeyUp(const int vKey) const noexcept {
    for (const auto& callback : onKeyUpCallbacks) {
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
    while (isPolling) {
        Poll();
        std::this_thread::sleep_for(duration);
    }
}
