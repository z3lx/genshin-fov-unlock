#pragma once

#include "utils/log/Common.h"
#include "utils/log/formatters/IFormatter.h"
#include "utils/log/sinks/ISink.h"

#include <chrono>
#include <memory>
#include <mutex>
#include <source_location>
#include <string_view>
#include <thread>
#include <vector>

#if ACTIVE_LEVEL < LEVEL_OFF
    #define LOG(level, fmt, ...)                                                \
        Logger::GetInstance().Log(                                              \
            std::chrono::system_clock::now(),                                   \
            std::this_thread::get_id(),                                         \
            std::source_location::current(),                                    \
            level, fmt __VA_OPT__(, __VA_ARGS__)                                \
        )
    #define LOG_SET_LEVEL(level) Logger::GetInstance().SetLevel(level)
    #define LOG_SET_SINKS(...) Logger::GetInstance().SetSinks(__VA_ARGS__)
    #define LOG_SET_FORMATTER(formatter) Logger::GetInstance().SetFormatter(formatter)
#else
    #define LOG_SET_LEVEL(level)
    #define LOG_SET_SINKS(...)
    #define LOG_SET_FORMATTER(formatter)
#endif

#if ACTIVE_LEVEL <= LEVEL_TRACE
    #define LOG_T(fmt, ...) LOG(Level::Trace, fmt, __VA_ARGS__)
#else
    #define LOG_T(fmt, ...)
#endif

#if ACTIVE_LEVEL <= LEVEL_DEBUG
    #define LOG_D(fmt, ...) LOG(Level::Debug, fmt, __VA_ARGS__)
#else
    #define LOG_D(fmt, ...)
#endif

#if ACTIVE_LEVEL <= LEVEL_INFO
    #define LOG_I(fmt, ...) LOG(Level::Info, fmt, __VA_ARGS__)
#else
    #define LOG_I(fmt, ...)
#endif

#if ACTIVE_LEVEL <= LEVEL_WARN
    #define LOG_W(fmt, ...) LOG(Level::Warn, fmt, __VA_ARGS__)
#else
    #define LOG_W(fmt, ...)
#endif

#if ACTIVE_LEVEL <= LEVEL_ERROR
    #define LOG_E(fmt, ...) LOG(Level::Error, fmt, __VA_ARGS__)
#else
    #define LOG_E(fmt, ...)
#endif

#if ACTIVE_LEVEL <= LEVEL_FATAL
    #define LOG_F(fmt, ...) LOG(Level::Fatal, fmt, __VA_ARGS__)
#else
    #define LOG_F(fmt, ...)
#endif

class Logger {
public:
    [[nodiscard]] static Logger& GetInstance();

    Logger();
    ~Logger() noexcept;

    void SetLevel(Level level) noexcept;
    template <typename... Args> void SetSinks(Args&&... sinks);
    void SetFormatter(std::unique_ptr<IFormatter> formatter);

    template<typename... Args>
    void Log(
        std::chrono::system_clock::time_point time,
        std::thread::id thread,
        std::source_location location,
        Level level,
        std::string_view fmt, Args&&... args
    ) noexcept;

private:
#if ACTIVE_LEVEL < LEVEL_OFF
    std::mutex mutex;
    Level level;
    std::vector<std::unique_ptr<ISink>> sinks;
    std::unique_ptr<IFormatter> formatter;
#endif
};

#include "utils/log/LoggerInl.h"
