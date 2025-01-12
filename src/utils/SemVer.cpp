#include "utils/SemVer.hpp"

#include <cctype>
#include <charconv>
#include <compare>
#include <format>
#include <optional>
#include <ranges>
#include <regex>
#include <stdexcept>
#include <string>
#include <string_view>
#include <tuple>

constexpr auto semVerRegex {
    R"(^(0|[1-9]\d*)\.(0|[1-9]\d*)\.(0|[1-9]\d*)(?:-((?:0|[1-9]\d*|\d*[a-zA-Z-)"
    R"(][0-9a-zA-Z-]*)(?:\.(?:0|[1-9]\d*|\d*[a-zA-Z-][0-9a-zA-Z-]*))*))?(?:\+()"
    R"([0-9a-zA-Z-]+(?:\.[0-9a-zA-Z-]+)*))?$)"
};

SemVer::SemVer(const int major, const int minor, const int patch,
    const std::optional<std::string>& preRelease,
    const std::optional<std::string>& buildMetadata)
    : major { major }, minor { minor }, patch { patch }
    , preRelease { preRelease }, buildMetadata { buildMetadata } {
    if (!std::regex_match(ToString(), std::regex { semVerRegex })) {
        throw std::invalid_argument { "Invalid version string" };
    }
}

SemVer::SemVer(const std::string& version)
    : version { version } {
    std::smatch match {};
    if (!std::regex_match(version, match, std::regex { semVerRegex })) {
        throw std::invalid_argument { "Invalid version string" };
    }
    major = std::stoi(match[1].str());
    minor = std::stoi(match[2].str());
    patch = std::stoi(match[3].str());
    if (match[4].length()) {
        preRelease = match[4].str();
    }
    if (match[5].length()) {
        buildMetadata = match[5].str();
    }
}

std::string SemVer::ToString() const {
    if (!version.empty()) {
        return version;
    }
    version = std::format("{}.{}.{}{}{}", major, minor, patch,
        preRelease.has_value() ? "-" + preRelease.value() : "",
        buildMetadata.has_value() ? "+" + buildMetadata.value() : ""
    );
    return version;
}

int SemVer::GetMajor() const {
    return major;
}

int SemVer::GetMinor() const {
    return minor;
}

int SemVer::GetPatch() const {
    return patch;
}

std::optional<std::string> SemVer::GetPreRelease() const {
    return preRelease;
}

std::optional<std::string> SemVer::GetBuildMetadata() const {
    return buildMetadata;
}

bool SemVer::operator==(const SemVer& other) const {
    return std::tie(major, minor, patch, preRelease) ==
        std::tie(other.major, other.minor, other.patch, other.preRelease);
}

std::strong_ordering SemVer::operator<=>(const SemVer& other) const {
    // Specification 11.2
    if (const auto cmp = std::tie(major, minor, patch) <=>
        std::tie(other.major, other.minor, other.patch);
        cmp != std::strong_ordering::equivalent) {
        return cmp;
    }

    // Specification 11.3
    if (const auto cmp = !preRelease.has_value() <=>
        !other.preRelease.has_value();
        cmp != std::strong_ordering::equivalent || (
        cmp == std::strong_ordering::equivalent &&
        !preRelease.has_value())) {
        return cmp;
    }

    // Specifications 11.4 & 11.4.4
    auto thisIdentifiers = preRelease.value() | std::views::split('.');
    auto otherIdentifiers = other.preRelease.value() | std::views::split('.');
    auto thisIt = thisIdentifiers.begin(), otherIt = otherIdentifiers.begin();
    std::string_view thisIdentifier, otherIdentifier;
    while (true) {
        if (const auto cmp = (thisIt != thisIdentifiers.end()) <=>
            (otherIt != otherIdentifiers.end());
            cmp != std::strong_ordering::equivalent || (
            cmp == std::strong_ordering::equivalent &&
            thisIt == thisIdentifiers.end())) {
            return cmp;
        }

        thisIdentifier = std::string_view { *thisIt };
        otherIdentifier = std::string_view { *otherIt };
        if (thisIdentifier != otherIdentifier) {
            break;
        }

        ++thisIt;
        ++otherIt;
    }

    // Specification 11.4.1
    auto isDigit = [](const std::string_view& identifier) {
        return std::ranges::all_of(identifier, [](const unsigned char c) {
            return std::isdigit(c);
        });
    };
    const auto isThisDigit = isDigit(thisIdentifier);
    const auto isOtherDigit = isDigit(otherIdentifier);
    if (isThisDigit && isOtherDigit) {
        auto toDigit = [](const std::string_view& identifier) {
            int result {};
            auto [p, ec] = std::from_chars(identifier.data(),
                identifier.data() + identifier.size(), result);
            return result;
        };
        return toDigit(thisIdentifier) <=> toDigit(otherIdentifier);
    }

    // Specification 11.4.2
    auto isAlpha = [](const std::string_view& identifier) {
        return std::ranges::all_of(identifier, [](const unsigned char c) {
            return std::isalpha(c) || c == '-';
        });
    };
    if (isAlpha(thisIdentifier) && isAlpha(otherIdentifier)) {
        return thisIdentifier <=> otherIdentifier;
    }

    // Specification 11.4.3
    return !isThisDigit <=> !isOtherDigit;
}
