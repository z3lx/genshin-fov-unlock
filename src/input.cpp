#include "input.h"
#include <chrono>
#include <functional>
#include <initializer_list>
#include <set>
#include <unordered_map>
#include <vector>
#include <windows.h>
#include <winuser.h>

struct InputManager::Impl {
    Impl();
    explicit Impl(DWORD process);

    void SetTrackedProcess(DWORD process);
    void RegisterKeys(std::initializer_list<int> vKeys);
    void RegisterOnKeyDown(const KeyEventCallback& callback);
    void RegisterOnKeyUp(const KeyEventCallback& callback);
    void Poll();

    void TriggerOnKeyDown(int vKey) const;
    void TriggerOnKeyUp(int vKey) const;

    DWORD trackedProcess;
    std::set<int> registeredKeys;
    std::unordered_map<int, bool> keyStates;
    std::vector<KeyEventCallback> onKeyDownCallbacks;
    std::vector<KeyEventCallback> onKeyUpCallbacks;
};

InputManager::Impl::Impl(
) : trackedProcess(0) { }

InputManager::Impl::Impl(
    const DWORD process
) : Impl() {
    SetTrackedProcess(process);
}

void InputManager::Impl::SetTrackedProcess(const DWORD process) {
    trackedProcess = process;
}

void InputManager::Impl::RegisterKeys(const std::initializer_list<int> vKeys) {
    for (const auto& key : vKeys) {
        registeredKeys.insert(key);
    }
}

void InputManager::Impl::RegisterOnKeyDown(const KeyEventCallback& callback) {
    onKeyDownCallbacks.push_back(callback);
}

void InputManager::Impl::RegisterOnKeyUp(const KeyEventCallback& callback) {
    onKeyUpCallbacks.push_back(callback);
}

void InputManager::Impl::Poll() {
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
    const DWORD process
) : impl(new Impl(process)) { }

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
