#pragma once

#include "plugin/Events.hpp"
#include "plugin/interfaces/IComponent.hpp"
#include "plugin/interfaces/IMediator.hpp"

#include <memory>
#include <mutex>
#include <unordered_set>

#include <Windows.h>

class InputManager final : public IComponent<Event> {
public:
    // TODO: Replace targetWindows with iterator?
    explicit InputManager(
        const std::weak_ptr<IMediator<Event>>& mediator,
        const std::unordered_set<HWND>& targetWindows
    );
    ~InputManager() noexcept override;

    void SetEnable(bool value) noexcept;

    void Handle(const Event& event) override;

private:
    struct Visitor {
        InputManager& m;

        template <typename T>
        void operator()(const T& event) const;
    };

    static void Notify(const Event& event) noexcept;

    static LRESULT CALLBACK KeyboardProc(
        int nCode, WPARAM wParam, LPARAM lParam
    ) noexcept;

    bool isEnabled;
    std::unordered_set<HWND> targetWindows;
};
