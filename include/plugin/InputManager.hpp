#pragma once

#include "plugin/Events.hpp"
#include "plugin/IComponent.hpp"
#include "plugin/IMediator.hpp"

#include <memory>
#include <mutex>
#include <vector>

#include <Windows.h>

class InputManager final : public IComponent<Event> {
public:
    explicit InputManager(
        const std::weak_ptr<IMediator<Event>>& mediator,
        const std::vector<HWND>& targetWindows = {}
    ) noexcept;
    ~InputManager() noexcept override;

    [[nodiscard]] bool IsHooked() const noexcept;
    void Hook(bool value);

    void Handle(const Event& event) override;

private:
    struct Visitor {
        InputManager& m;

        template <typename T>
        void operator()(const T& event) const;
    };

    static HHOOK SetHook();
    static void RemoveHook(HHOOK& handle) noexcept;
    static void ProcessMessages() noexcept;

    static LRESULT CALLBACK KeyboardProc(
        int nCode, WPARAM wParam, LPARAM lParam
    ) noexcept;
    static void Notify(const Event& event) noexcept;

    static HHOOK hHook;
    static std::mutex mutex;
    static std::vector<InputManager*> instances;

    std::vector<HWND> targetWindows;
};
