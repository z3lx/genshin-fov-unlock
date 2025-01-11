#pragma once

#include <compare>
#include <optional>
#include <string>

// Specifications: https://semver.org

class SemVer {
public:
    SemVer(
        int major, int minor, int patch,
        const std::optional<std::string>& preRelease = std::nullopt,
        const std::optional<std::string>& buildMetadata = std::nullopt
    );
    explicit SemVer(const std::string& version);

    [[nodiscard]] std::string ToString() const;
    [[nodiscard]] int GetMajor() const;
    [[nodiscard]] int GetMinor() const;
    [[nodiscard]] int GetPatch() const;
    [[nodiscard]] std::optional<std::string> GetPreRelease() const;
    [[nodiscard]] std::optional<std::string> GetBuildMetadata() const;

    bool operator==(const SemVer& other) const;
    std::strong_ordering operator<=>(const SemVer& other) const;

private:
    int major;
    int minor;
    int patch;
    std::optional<std::string> preRelease;
    std::optional<std::string> buildMetadata;

    mutable std::string version;
};
