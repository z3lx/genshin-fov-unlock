#pragma once

#include <spdlog/details/file_helper.h>
#include <spdlog/details/null_mutex.h>
#include <spdlog/details/synchronous_factory.h>
#include <spdlog/sinks/base_sink.h>

#include <chrono>
#include <mutex>
#include <queue>
#include <string>

// TODO: IMPROVE ERROR HANDLING

template<typename Mutex>
class TimeBufferedFileSink final : public spdlog::sinks::base_sink<Mutex> {
public:
    TimeBufferedFileSink(
        const std::string& filename,
        int bufferDuration, // in milliseconds
        bool truncate = false
    );
    ~TimeBufferedFileSink() override;

protected:
    void sink_it_(const spdlog::details::log_msg& msg) override;
    void flush_() override;

private:
    struct TimedBuffer {
        std::chrono::system_clock::time_point time;
        spdlog::memory_buf_t buffer;
    };

    void RemoveOldBuffers();

    spdlog::details::file_helper file;
    std::chrono::milliseconds bufferDuration;
    std::queue<TimedBuffer> buffers;
};

using TimeBufferedFileSinkST =
    TimeBufferedFileSink<spdlog::details::null_mutex>;
using TimeBufferedFileSinkMT =
    TimeBufferedFileSink<std::mutex>;

template<typename Factory = spdlog::synchronous_factory>
std::shared_ptr<spdlog::logger> TimeBufferedLoggerMt(
    const std::string& loggerName,
    const std::string& filename,
    int bufferDuration,
    bool truncate = false) {
    return Factory::template create<TimeBufferedFileSinkMT>(
        loggerName, filename, bufferDuration, truncate
    );
}

template<typename Factory = spdlog::synchronous_factory>
std::shared_ptr<spdlog::logger> TimeBufferedLoggerSt(
    const std::string& loggerName,
    const std::string& filename,
    int bufferDuration,
    bool truncate = false) {
    return Factory::template create<TimeBufferedFileSinkST>(
        loggerName, filename, bufferDuration, truncate
    );
}

#include "sink-inl.h"
