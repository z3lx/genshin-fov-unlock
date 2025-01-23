#include "utils/SemVer.hpp"
#include "utils/Windows.hpp"

#include <algorithm>
#include <exception>
#include <filesystem>
#include <initializer_list>
#include <iostream>
#include <ranges>
#include <string>
#include <vector>

#include <Windows.h>

namespace fs = std::filesystem;

void RequestElevation() {
    // Check if process is elevated
    const auto currentProcess = GetCurrentProcess();
    constexpr auto desiredAccess = TOKEN_QUERY;
    HANDLE tokenHandle {};
    ThrowOnSystemError(OpenProcessToken(
        currentProcess, desiredAccess, &tokenHandle
    ));
    HandleGuard tokenHandleGuard { tokenHandle };

    TOKEN_ELEVATION elevation {};
    DWORD returnElevationSize {};
    ThrowOnSystemError(GetTokenInformation(
        tokenHandle, TokenElevation,
        &elevation, sizeof(elevation), &returnElevationSize
    ));

    if (elevation.TokenIsElevated) {
        return;
    }

    // Restart process with elevated privileges
    constexpr auto filePathSize = 1024;
    wchar_t filePath[filePathSize];
    ThrowOnSystemError(GetModuleFileNameW(
        nullptr, filePath, filePathSize
    ));
    SHELLEXECUTEINFOW info {
        .cbSize = sizeof(info),
        .hwnd = nullptr,
        .lpVerb = L"runas",
        .lpFile = filePath,
        .nShow = SW_NORMAL
    };
    ThrowOnSystemError(ShellExecuteExW(&info));
    std::exit(0);
}

SemVer GetFileVersion(const fs::path& filePath) {
    DWORD versionInfoSize {};
    ThrowOnSystemError(versionInfoSize = GetFileVersionInfoSizeW(
        filePath.c_str(), nullptr
    ));

    std::vector<char> versionInfoBuffer(versionInfoSize, 0);
    ThrowOnSystemError(GetFileVersionInfoW(
        filePath.c_str(), 0, versionInfoBuffer.size(), versionInfoBuffer.data()
    ));

    constexpr auto subBlock = L"\\";
    PVOID versionQueryBuffer {};
    UINT versionQueryBufferSize {};
    ThrowOnSystemError(VerQueryValueW(
        versionInfoBuffer.data(), subBlock,
        &versionQueryBuffer, &versionQueryBufferSize
    ));

    const auto& versionInfo = *static_cast<VS_FIXEDFILEINFO*>(versionQueryBuffer);
    return {
        static_cast<int>(HIWORD(versionInfo.dwFileVersionMS)),
        static_cast<int>(LOWORD(versionInfo.dwFileVersionMS)),
        static_cast<int>(HIWORD(versionInfo.dwFileVersionLS))
    };
}

template <typename... Args>
requires (std::convertible_to<Args, fs::path> && ...)
void InjectDlls(HANDLE processHandle, Args&&... dllPaths) {
    if constexpr (sizeof...(Args) == 0) {
        return;
    }

    const std::initializer_list<fs::path> paths = {
        std::forward<Args>(dllPaths)...
    };

    // Adjust privileges
    constexpr auto systemName = nullptr;
    constexpr auto privilegeName = SE_DEBUG_NAME; // = 20L
    LUID luid {};
    ThrowOnSystemError(LookupPrivilegeValue(
        systemName, privilegeName, &luid
    ));

    const auto currentProcess = GetCurrentProcess();
    constexpr auto desiredAccess = TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY;
    HANDLE tokenHandle {};
    ThrowOnSystemError(OpenProcessToken(
        currentProcess, desiredAccess, &tokenHandle
    ));
    HandleGuard tokenHandleGuard { tokenHandle };

    constexpr auto disableAll = FALSE;
    TOKEN_PRIVILEGES newPrivileges {
        .PrivilegeCount = 1,
        .Privileges = {{
            .Luid = luid,
            .Attributes = SE_PRIVILEGE_ENABLED
        }}
    };
    constexpr auto newPrivilegesSize = sizeof(newPrivileges);
    TOKEN_PRIVILEGES oldPrivileges {};
    DWORD oldPrivilegesSize {};
    ThrowOnSystemError(AdjustTokenPrivileges(
        tokenHandle, disableAll, &newPrivileges, newPrivilegesSize,
        &oldPrivileges, &oldPrivilegesSize
    ));

    // Get LoadLibraryW
    const auto kernel32 = GetModuleHandleA("kernel32.dll");
    const auto loadLibraryW = reinterpret_cast<decltype(&LoadLibraryW)>(
        GetProcAddress(kernel32, "LoadLibraryW")
    );

    // Allocate memory for dll path
    constexpr auto wCharSize = sizeof(std::wstring::value_type); // wchar_t

    constexpr auto address = nullptr;
    const auto maxDllPathBufferSize = std::ranges::max_element(paths,
        [](const fs::path& a, const fs::path& b) {
            return a.native().size() < b.native().size();
        }
    )->native().size() * wCharSize;
    constexpr auto allocation = MEM_COMMIT | MEM_RESERVE;
    constexpr auto protection = PAGE_READWRITE;
    LPVOID dllPathPtr {};
    ThrowOnSystemError(dllPathPtr = VirtualAllocEx(
        processHandle, address, maxDllPathBufferSize, allocation, protection
    ));
    VirtualMemoryGuard dllPathPtrGuard {
        processHandle, dllPathPtr, 0, MEM_RELEASE
    };
    const std::vector<char> emptyBuffer(maxDllPathBufferSize, 0);

    for (const fs::path& dllPath : paths) {
        // Write dll path to process
        const auto dllPathWStr = dllPath.wstring();
        const auto dllPathBuffer = dllPathWStr.c_str();
        const auto dllPathBufferSize = dllPathWStr.size() * wCharSize;
        SIZE_T bytesWritten {};
        ThrowOnSystemError(WriteProcessMemory(
            processHandle, dllPathPtr, dllPathBuffer, dllPathBufferSize,
            &bytesWritten
        ));

        // Create thread to load dll
        constexpr auto attributes = nullptr;
        constexpr auto stackSize = 0;
        const auto dllLoadPtr = reinterpret_cast<LPTHREAD_START_ROUTINE>(loadLibraryW);
        constexpr auto flags = 0;
        DWORD threadId {};
        HANDLE threadHandle {};
        ThrowOnSystemError(threadHandle = CreateRemoteThread(
            processHandle, attributes, stackSize, dllLoadPtr, dllPathPtr, flags,
            &threadId
        ));
        HandleGuard threadHandleGuard { threadHandle };
        WaitForSingleObject(threadHandle, INFINITE);

        // Cleanup
        WriteProcessMemory(
            processHandle, dllPathPtr, emptyBuffer.data(), dllPathBufferSize,
            &bytesWritten
        );
    }
}

int main() try {
    RequestElevation();

    const fs::path gamePath = R"(C:\Program Files\HoYoPlay\games\Genshin Impact game\GenshinImpact.exe)";
    std::wstring gameArgs = gamePath.wstring() + L" -popupwindow -screen-fullscreen 0";
    const fs::path gameDirectory = gamePath.parent_path();
    const fs::path pluginPath = fs::absolute("genshin_fov_unlock.dll");
    bool suspendLoad = false;

    STARTUPINFOW si { .cb = sizeof(si) };
    PROCESS_INFORMATION pi {};
    const DWORD flags = suspendLoad ? CREATE_SUSPENDED : 0;

    ThrowOnSystemError(CreateProcessW(
        gamePath.c_str(), gameArgs.data(), nullptr, nullptr,
        FALSE, flags, nullptr, gameDirectory.c_str(), &si, &pi
    ));

    InjectDlls(pi.hProcess, pluginPath);

    if (suspendLoad) {
        ResumeThread(pi.hThread);
    }
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);

    return 0;
} catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
    return 1;
}
