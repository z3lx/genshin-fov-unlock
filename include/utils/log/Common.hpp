#pragma once

#include <chrono>
#include <source_location>
#include <string_view>
#include <thread>

#define LEVEL_TRACE 0
#define LEVEL_DEBUG 1
#define LEVEL_INFO 2
#define LEVEL_WARN 3
#define LEVEL_ERROR 4
#define LEVEL_FATAL 5
#define LEVEL_OFF 6

enum class Level : int {
    Trace = LEVEL_TRACE,
    Debug = LEVEL_DEBUG,
    Info = LEVEL_INFO,
    Warn = LEVEL_WARN,
    Error = LEVEL_ERROR,
    Fatal = LEVEL_FATAL,
    Off = LEVEL_OFF
};

namespace details {
    struct Message {
        std::chrono::system_clock::time_point time;
        std::thread::id thread;
        std::source_location location;
        Level level;
        std::string_view content;
    };
}
