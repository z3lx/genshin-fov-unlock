#include "utils/log/formatters/DefaultFormatter.hpp"
#include "utils/log/Common.hpp"

#include <format>
#include <string>
#include <string_view>

DefaultFormatter::DefaultFormatter() = default;
DefaultFormatter::~DefaultFormatter() = default;

std::string DefaultFormatter::Format(const details::Message& message) {
    const std::string time = std::format("{:%Y-%m-%d %H:%M:%S}",
        floor<std::chrono::microseconds>(message.time)
    );

    const std::string location = [&location = message.location]() {
        const auto filePath = std::string_view { location.file_name() };
        std::string_view filename = filePath;
        if (const auto posSeparator = filePath.find_last_of("\\/");
            posSeparator != std::string_view::npos) {
            filename = filePath.substr(posSeparator + 1);
            }
        return std::format("{}:{}", filename, location.line());
    }();

    const std::string level = [&level = message.level]() {
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

    return std::format("{:<26} | {:<5} | {:<25} | {:<5} | {}\n",
        time, message.thread, location, level, message.content
    );
}
