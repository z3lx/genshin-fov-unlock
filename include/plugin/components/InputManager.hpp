#pragma once

#include "plugin/Events.hpp"
#include "plugin/interfaces/IComponent.hpp"
#include "plugin/interfaces/IMediator.hpp"

#include <memory>
#include <mutex>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <Windows.h>

class InputManager final : public IComponent<Event> {
public:
    explicit InputManager(
        const std::weak_ptr<IMediator<Event>>& mediator,
        const std::vector<HWND>& targetWindows = {}
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

    static HHOOK SetHook();
    static void RemoveHook(HHOOK& handle);

    static LRESULT CALLBACK KeyboardProc(
        int nCode, WPARAM wParam, LPARAM lParam
    ) noexcept;
    static void Notify(const Event& event) noexcept;

    static HHOOK hHook;
    static std::mutex mutex;
    static std::unordered_set<InputManager*> instances;
    static std::unordered_map<int, bool> keyStates;

    bool isEnabled;
    std::vector<HWND> targetWindows;
};
