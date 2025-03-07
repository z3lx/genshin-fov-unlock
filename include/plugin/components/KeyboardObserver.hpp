#pragma once

#include "plugin/Events.hpp"
#include "plugin/interfaces/IComponent.hpp"
#include "plugin/interfaces/IMediator.hpp"

#include <memory>
#include <unordered_set>

#include <Windows.h>

class KeyboardObserver final : public IComponent<Event> {
public:
    // TODO: Replace targetWindows with iterator?
    explicit KeyboardObserver(
        std::weak_ptr<IMediator<Event>> mediator,
        const std::unordered_set<HWND>& targetWindows
    );
    ~KeyboardObserver() noexcept override;

    [[nodiscard]] bool IsEnabled() const noexcept;
    [[nodiscard]] const std::unordered_set<HWND>& GetTargetWindows() const noexcept;
    void SetEnabled(bool value) noexcept;
    void SetTargetWindows(const std::unordered_set<HWND>& windows) noexcept;

    void Handle(const Event& event) noexcept override;

private:
    bool isEnabled;
    std::unordered_set<HWND> targetWindows;
};
