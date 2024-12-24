#include "plugin/InputManager.h"
#include "plugin/Events.h"
#include "plugin/IComponent.h"
#include "plugin/IMediator.h"

#include <atomic>
#include <chrono>
#include <memory>
#include <mutex>
#include <set>
#include <stdexcept>
#include <thread>
#include <unordered_map>
#include <variant>

#include <Windows.h>

InputManager::InputManager(
    const std::weak_ptr<IMediator<Event>>& mediator) noexcept
    : IComponent(mediator)
    , trackedProcess(0)
    , isPolling(false) { }

InputManager::~InputManager() noexcept {
    StopPolling();
}

void InputManager::SetTrackedProcess(const DWORD process) noexcept {
    std::lock_guard lock(dataMutex);
    trackedProcess = process;
}

void InputManager::RegisterKeys(const std::vector<int>& vKeys) {
    std::lock_guard lock(dataMutex);
    for (const auto& key : vKeys) {
        registeredKeys.insert(key);
    }
}

void InputManager::Poll() noexcept {
    std::lock_guard lock(dataMutex);

    const auto mediator = weakMediator.lock();
    if (!mediator) {
        return;
    }

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
                mediator->Notify(OnKeyDown { key });
            }
        } else {
            if (keyStates[key]) {
                keyStates[key] = false;
                mediator->Notify(OnKeyUp { key });
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

void InputManager::PollingThread(const int pollingRate) noexcept {
    const auto duration = std::chrono::duration<double, std::milli>(
        1000.0 / pollingRate);
    while (isPolling) {
        Poll();
        std::this_thread::sleep_for(duration);
    }
}

void InputManager::Handle(const Event& event) {
    std::visit(Visitor { *this }, event);
}

template <typename T>
void InputManager::Visitor::operator()(const T& event) const { }

template <>
void InputManager::Visitor::operator()(const OnKeyBindChange& event) const {
    m.RegisterKeys(event.vKeys);
}
