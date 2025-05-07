#pragma once

#include "components/ConfigManager.hpp"
#include "plugin/Events.hpp"
#include "plugin/interfaces/IMediator.hpp"

#include <vector>

#include <Windows.h>

class Plugin final : public IMediator<Event> {
public:
    Plugin();
    ~Plugin() override;

private:
    struct Visitor;

    void Start() noexcept override;
    void Notify(const Event& event) noexcept override;

    template <typename Event>
    void Handle(const Event& event) noexcept;
    void ConsumeState() noexcept;

    // State
    bool isUnlockerHooked;
    bool isWindowFocused;
    bool isCursorVisible;
    std::vector<HWND> targetWindows;
    Config config;
};
