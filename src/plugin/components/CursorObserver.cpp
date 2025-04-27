#include "plugin/components/CursorObserver.hpp"
#include "plugin/Events.hpp"

#include <Windows.h>

CursorObserver::CursorObserver() noexcept = default;
CursorObserver::~CursorObserver() noexcept = default;

void CursorObserver::Update() noexcept {
    CURSORINFO cursorInfo { .cbSize = sizeof(cursorInfo) };
    if (!GetCursorInfo(&cursorInfo)) {
        return;
    }

    if (const bool isCurrentCursorVisible = cursorInfo.flags & CURSOR_SHOWING;
        isCurrentCursorVisible != isPreviousCursorVisible) {
        isPreviousCursorVisible = isCurrentCursorVisible;
        Notify(OnCursorVisibilityChange { isCurrentCursorVisible });
    }
}
