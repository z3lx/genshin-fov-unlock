#pragma once

#include "components/ConfigManager.hpp"
#include "components/KeyboardObserver.hpp"
#include "components/MouseObserver.hpp"
#include "components/Unlocker.hpp"
#include "components/WinEventNotifier.hpp"
#include "plugin/Events.hpp"
#include "plugin/interfaces/IMediator.hpp"

#include <memory>
#include <mutex>
#include <vector>

#include <Windows.h>

class Plugin final : public IMediator<Event> {
public:
    static std::shared_ptr<Plugin> MakePlugin();
    ~Plugin() override;

    void Notify(const Event& event) noexcept override;

private:
    struct Visitor;

    Plugin();

    template <typename Event>
    void Handle(const Event& event) noexcept;
    void ConsumeState() noexcept;

    std::mutex mutex;

    // Components
    std::unique_ptr<KeyboardObserver> keyboardObserver;
    std::unique_ptr<MouseObserver> mouseObserver;
    std::unique_ptr<WinEventNotifier> winEventNotifier;
    std::unique_ptr<ConfigManager> configManager;
    std::unique_ptr<Unlocker> unlocker;

    // State
    bool isUnlockerHooked;
    bool isWindowFocused;
    bool isCursorVisible;
    std::vector<HWND> targetWindows;
    Config config;
};
