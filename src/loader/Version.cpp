#include "loader/Version.hpp"
#include "utils/SemVer.hpp"
#include "utils/Windows.hpp"

#include <cpr/cpr.h>
#include <nlohmann/json.hpp>

#include <filesystem>
#include <stdexcept>
#include <string>
#include <vector>

#include <Windows.h>

namespace fs = std::filesystem;

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
    return SemVer {
        static_cast<int>(HIWORD(versionInfo.dwFileVersionMS)),
        static_cast<int>(LOWORD(versionInfo.dwFileVersionMS)),
        static_cast<int>(HIWORD(versionInfo.dwFileVersionLS))
    };
}

SemVer GetCurrentVersion() {
    constexpr auto filePathSize = 1024;
    wchar_t filePath[filePathSize];
    ThrowOnSystemError(GetModuleFileNameW(
        nullptr, filePath, filePathSize
    ));
    return GetFileVersion(filePath);
}

SemVer GetLatestVersion() {
    const cpr::Response response = cpr::Get(
        cpr::Url { "https://api.github.com/repos/z3lx/genshin-fov-unlock/releases" },
        cpr::Header { { "X-GitHub-Api-Version", "2022-11-28" } }
    );
    if (response.status_code != 200) {
        throw std::runtime_error { "Failed to fetch releases" };
    }
    const nlohmann::json json = nlohmann::json::parse(response.text);

    auto str = json.at(0)["tag_name"].get<std::string>();
    if (!str.empty() && str[0] == 'v') {
        str.erase(0, 1);
    }
    return SemVer { str };
}
