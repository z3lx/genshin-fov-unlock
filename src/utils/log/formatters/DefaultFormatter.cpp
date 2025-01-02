#include "utils/log/formatters/DefaultFormatter.hpp"
#include "utils/log/Common.hpp"

#include <cstddef>
#include <format>
#include <sstream>
#include <string>
#include <string_view>

DefaultFormatter::DefaultFormatter() = default;
DefaultFormatter::~DefaultFormatter() = default;

std::string DefaultFormatter::Format(const details::Message& message) {
    ss.clear();
    ss.str("");

    const std::string level = [level = message.level]() {
        switch (level) {
            case Level::Trace: return "TRACE";
            case Level::Debug: return "DEBUG";
            case Level::Info: return "INFO";
            case Level::Warn: return "WARN";
            case Level::Error: return "ERROR";
            case Level::Fatal: return "FATAL";
            default: return "UNKNOWN";
        }
    }();

    ss << message.time << " | "
        << message.thread << " | "
        << TruncateOrPad(std::format(
                "{}:{}",
                message.location.file_name(),
                message.location.line()
            ), 75) << " | "
        << TruncateOrPad(level, 5) << " | "
        << message.content << '\n';

    return ss.str();
}

std::string DefaultFormatter::TruncateOrPad(
    const std::string_view input, const size_t length) {
    std::string result;
    result.reserve(length);

    if (length < 3) {
        result.assign(length, '.');
    } else if (input.size() > length) {
        result = input.substr(0, length - 3);
        result.append("...");
    } else {
        result = input;
        result.append(length - input.size(), ' ');
    }

    return result;
}
