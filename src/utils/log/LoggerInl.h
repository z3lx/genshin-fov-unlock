#include "utils/log/formatters/DefaultFormatter.h"
#include "utils/log/formatters/IFormatter.h"
#include "utils/log/sinks/ConsoleSink.h"
#include "utils/log/sinks/ISink.h"

#include <format>
#include <memory>
#include <mutex>
#include <source_location>
#include <stdexcept>
#include <string>
#include <string_view>
#include <thread>
#include <utility>
#include <vector>

inline Logger& Logger::GetInstance() {
    static Logger instance {};
    return instance;
}

inline Logger::Logger() {
#if ACTIVE_LEVEL < LEVEL_OFF
    level = Level::Info;
    sinks.push_back(std::make_unique<ConsoleSink>());
    formatter = std::make_unique<DefaultFormatter>();
#endif
}

inline Logger::~Logger() noexcept = default;

inline void Logger::SetLevel(const Level level) noexcept {
#if ACTIVE_LEVEL < LEVEL_OFF
    std::lock_guard lock { mutex };
    this->level = level;
#endif
}

template <typename... Args> void Logger::SetSinks(Args&&... sinks) {
#if ACTIVE_LEVEL < LEVEL_OFF
    std::lock_guard lock { mutex };
    this->sinks.clear();
    (this->sinks.push_back(std::forward<Args>(sinks)), ...);
#endif
}

inline void Logger::SetFormatter(std::unique_ptr<IFormatter> formatter) {
#if ACTIVE_LEVEL < LEVEL_OFF
    std::lock_guard lock { mutex };
    this->formatter = std::move(formatter);
#endif
}

template<typename... Args>
void Logger::Log(
    const std::chrono::system_clock::time_point time,
    const std::thread::id thread,
    const std::source_location location,
    const Level level,
    const std::string_view fmt, Args&&... args) noexcept try {
#if ACTIVE_LEVEL < LEVEL_OFF
    std::lock_guard lock { mutex };
    if (this->level > level) {
        return;
    }

    const std::string message = formatter->Format({
        time, thread, location, level,
        std::vformat(fmt, std::make_format_args(args...))
    });

    for (const auto& sink : sinks) {
        sink->Write(message);
        sink->Flush();
    }
#endif
} catch (const std::exception& e) {
    // Ignore exceptions
}

template<>
inline void Logger::Log(
    const std::chrono::system_clock::time_point time,
    const std::thread::id thread,
    const std::source_location location,
    const Level level,
    const std::string_view fmt) noexcept try {
#if ACTIVE_LEVEL < LEVEL_OFF
    std::lock_guard lock { mutex };
    if (this->level > level) {
        return;
    }

    const std::string message = formatter->Format({
        time, thread, location, level, fmt
    });

    for (const auto& sink : sinks) {
        sink->Write(message);
        sink->Flush();
    }
#endif
} catch (const std::exception& e) {
    // Ignore exceptions
}
