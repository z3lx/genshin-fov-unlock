#include "input.h"
#include <chrono>
#include <functional>
#include <unordered_map>
#include <vector>
#include <windows.h>
#include <winuser.h>

struct InputManager::Impl {
    Impl();
    explicit Impl(DWORD process);

    void SetTrackedProcess(DWORD process);
    void RegisterOnKeyDown(const KeyEventCallback& callback);
    void RegisterOnKeyUp(const KeyEventCallback& callback);
    void Poll();

    void TriggerOnKeyDown(int vKey) const;
    void TriggerOnKeyUp(int vKey) const;

    DWORD trackedProcess;
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

void InputManager::Impl::RegisterOnKeyDown(
    const KeyEventCallback& callback
) {
    onKeyDownCallbacks.push_back(callback);
}

void InputManager::Impl::RegisterOnKeyUp(
    const KeyEventCallback& callback
) {
    onKeyUpCallbacks.push_back(callback);
}

void InputManager::Impl::Poll() {
    DWORD foregroundProcess;
    GetWindowThreadProcessId(GetForegroundWindow(), &foregroundProcess);
    if (trackedProcess != 0 &&
        trackedProcess != foregroundProcess) {
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

void InputManager::RegisterOnKeyDown(const KeyEventCallback& callback) {
    impl->RegisterOnKeyDown(callback);
}

void InputManager::RegisterOnKeyUp(const KeyEventCallback& callback) {
    impl->RegisterOnKeyUp(callback);
}

void InputManager::Poll() const {
    impl->Poll();
}
