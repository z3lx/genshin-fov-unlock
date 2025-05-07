#pragma once

#include "plugin/Events.hpp"
#include "plugin/interfaces/IComponent.hpp"

#include <mutex>
#include <vector>

class KeyboardObserver final : public IComponent<Event> {
public:
    KeyboardObserver();
    ~KeyboardObserver() noexcept override;

private:
    struct Hook;

    void Update() noexcept override;

    std::mutex mutex;
    std::vector<Event> keyboardEvents;
};
