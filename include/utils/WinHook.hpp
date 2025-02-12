#pragma once

#include <Windows.h>

enum class HookProcedure {
    WhCallWndProc = WH_CALLWNDPROC,
    WhCallWndProcRet = WH_CALLWNDPROCRET,
    WhCbt = WH_CBT,
    WhDebug = WH_DEBUG,
    WhForegroundIdle = WH_FOREGROUNDIDLE,
    WhGetMessage = WH_GETMESSAGE,
    WhJournalPlayback = WH_JOURNALPLAYBACK,
    WhJournalRecord = WH_JOURNALRECORD,
    WhKeyboard = WH_KEYBOARD,
    WhKeyboardLl = WH_KEYBOARD_LL,
    WhMouse = WH_MOUSE,
    WhMouseLl = WH_MOUSE_LL,
    WhMsgFilter = WH_MSGFILTER,
    WhShell = WH_SHELL,
    WhSysMsgFilter = WH_SYSMSGFILTER
};

class WinHook {
public:
    WinHook(HookProcedure procedure, HOOKPROC callback);
    ~WinHook() noexcept;
    [[nodiscard]] HHOOK GetHandle() const noexcept;

private:
    HHOOK hHook;
};
