#include "utils/SemVer.hpp"

#include <algorithm>
#include <exception>
#include <filesystem>
#include <initializer_list>
#include <iostream>
#include <ranges>
#include <string>
#include <system_error>
#include <vector>

#include <Windows.h>

namespace fs = std::filesystem;

template <typename T>
void ThrowOnError(const T condition) {
    if (!condition) {
        throw std::system_error {
            static_cast<int>(GetLastError()), std::system_category()
        };
    }
}

void RequestElevation() {
    // Check if process is elevated
    const auto currentProcess = GetCurrentProcess();
    constexpr auto desiredAccess = TOKEN_QUERY;
    HANDLE tokenHandle {};
    ThrowOnError(OpenProcessToken(currentProcess, desiredAccess, &tokenHandle));

    TOKEN_ELEVATION elevation {};
    DWORD size {};
    try {
        ThrowOnError(GetTokenInformation(
            tokenHandle, TokenElevation, &elevation, sizeof(elevation), &size
        ));
    } catch (...) {
        CloseHandle(tokenHandle);
        throw;
    }
    CloseHandle(tokenHandle);

    if (elevation.TokenIsElevated) {
        return;
    }

    // Restart process with elevated privileges
    wchar_t path[MAX_PATH];
    ThrowOnError(GetModuleFileNameW(nullptr, path, MAX_PATH));
    SHELLEXECUTEINFOW info {
        .cbSize = sizeof(info),
        .hwnd = nullptr,
        .lpVerb = L"runas",
        .lpFile = path,
        .nShow = SW_NORMAL
    };
    ThrowOnError(ShellExecuteExW(&info));
    std::exit(0);
}

SemVer GetFileVersion(const fs::path& filePath) {
    const DWORD versionInfoSize = GetFileVersionInfoSizeW(
        filePath.c_str(), nullptr
    );

    std::vector<char> versionInfoBuffer(versionInfoSize, 0);
    ThrowOnError(GetFileVersionInfoW(
        filePath.c_str(), 0, versionInfoBuffer.size(), versionInfoBuffer.data()
    ));

    constexpr auto subBlock = L"\\";
    PVOID versionQueryBuffer {};
    UINT versionQueryBufferSize {};
    ThrowOnError(VerQueryValueW(
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

    const std::initializer_list<fs::path> paths = { dllPaths... };

    // Adjust privileges
    const auto currentProcess = GetCurrentProcess();
    constexpr auto desiredAccess = TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY;
    HANDLE tokenHandle {};
    ThrowOnError(OpenProcessToken(currentProcess, desiredAccess, &tokenHandle));
    try {
        constexpr auto systemName = nullptr;
        constexpr auto privilegeName = SE_DEBUG_NAME; // = 20L
        LUID luid {};
        ThrowOnError(LookupPrivilegeValue(systemName, privilegeName, &luid));

        constexpr auto disableAll = FALSE;
        TOKEN_PRIVILEGES newPrivileges {
            .PrivilegeCount = 1,
            .Privileges = {{
                .Luid = luid,
                .Attributes = SE_PRIVILEGE_ENABLED
            }}
        };
        constexpr auto newPrivilegesSize = sizeof(TOKEN_PRIVILEGES);
        TOKEN_PRIVILEGES oldPrivileges {};
        DWORD oldPrivilegesSize {};
        ThrowOnError(AdjustTokenPrivileges(
            tokenHandle, disableAll, &newPrivileges, newPrivilegesSize,
            &oldPrivileges, &oldPrivilegesSize
        ));
    } catch (...) {
        CloseHandle(tokenHandle);
        throw;
    }
    CloseHandle(tokenHandle);

    // Get LoadLibraryW
    const auto kernel32 = GetModuleHandleA("kernel32.dll");
    const auto loadLibraryW = reinterpret_cast<decltype(&LoadLibraryW)>(
        GetProcAddress(kernel32, "LoadLibraryW")
    );
    constexpr auto charSize = sizeof(std::wstring::value_type); // wchar_t

    // Allocate memory for dll path
    constexpr auto address = nullptr;
    const auto maxPathBufferSize = std::ranges::max_element(paths,
        [](const fs::path& a, const fs::path& b) {
            return a.wstring().size() < b.wstring().size();
        }
    )->wstring().size() * charSize;
    constexpr auto allocationType = MEM_COMMIT | MEM_RESERVE;
    constexpr auto protection = PAGE_READWRITE;
    LPVOID remoteWStr {};
    ThrowOnError(remoteWStr = VirtualAllocEx(
        processHandle, address, maxPathBufferSize, allocationType, protection
    ));

    for (const fs::path& path : paths) {
        // Write dll path to process
        const auto wStrPath = path.wstring();
        const auto wStrPathBuffer = wStrPath.c_str();
        const auto wStrPathBufferSize = wStrPath.size() * charSize;
        SIZE_T bytesWritten {};
        ThrowOnError(WriteProcessMemory(
            processHandle, remoteWStr, wStrPathBuffer, wStrPathBufferSize,
            &bytesWritten
        ));

        // Create thread to load dll
        constexpr auto attributes = nullptr;
        constexpr auto stackSize = 0;
        const auto remoteFunc = reinterpret_cast<LPTHREAD_START_ROUTINE>(loadLibraryW);
        constexpr auto flags = 0;
        DWORD threadId {};
        HANDLE remoteThreadHandle {};
        ThrowOnError(remoteThreadHandle = CreateRemoteThread(
            processHandle, attributes, stackSize, remoteFunc, remoteWStr, flags,
            &threadId
        ));
        WaitForSingleObject(remoteThreadHandle, INFINITE);

        // Cleanup
        CloseHandle(remoteThreadHandle);
        const std::vector<char> emptyBuffer(wStrPathBufferSize, 0);
        WriteProcessMemory(
            processHandle, remoteWStr, emptyBuffer.data(), emptyBuffer.size(),
            &bytesWritten
        );
    }
    VirtualFreeEx(processHandle, remoteWStr, 0, MEM_RELEASE);
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

    ThrowOnError(CreateProcessW(
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
