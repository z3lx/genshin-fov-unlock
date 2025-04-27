#include "plugin/components/MouseObserver.hpp"
#include "plugin/Events.hpp"

#include <Windows.h>

MouseObserver::MouseObserver() noexcept = default;
MouseObserver::~MouseObserver() noexcept = default;

void MouseObserver::Update() noexcept {
    CURSORINFO cursorinfo { .cbSize = sizeof(cursorinfo) };
    if (!GetCursorInfo(&cursorinfo)) {
        return;
    }

    if (const bool isCurrentCursorVisible = cursorinfo.flags & CURSOR_SHOWING;
        isCurrentCursorVisible != isPreviousCursorVisible) {
        isPreviousCursorVisible = isCurrentCursorVisible;
        Notify(OnCursorVisibilityChange { isCurrentCursorVisible });
    }
}
