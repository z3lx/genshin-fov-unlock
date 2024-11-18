#pragma once

#include <spdlog/details/file_helper.h>
#include <spdlog/details/null_mutex.h>
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
        int bufferDuration // in milliseconds
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

    spdlog::details::file_helper _file;
    std::chrono::milliseconds _bufferDuration;
    std::queue<TimedBuffer> _buffers;
};

using TimeBufferedFileSinkST = TimeBufferedFileSink<spdlog::details::null_mutex>;
using TimeBufferedFileSinkMT = TimeBufferedFileSink<std::mutex>;

template <typename Mutex>
TimeBufferedFileSink<Mutex>::TimeBufferedFileSink(
    const std::string& filename,
    const int bufferDuration)
    : _bufferDuration(std::chrono::milliseconds(bufferDuration)) {
    _file.open(filename, true);
}

template <typename Mutex>
TimeBufferedFileSink<Mutex>::~TimeBufferedFileSink() {
    _file.close();
}

template <typename Mutex>
void TimeBufferedFileSink<Mutex>::sink_it_(
    const spdlog::details::log_msg& msg) {
    RemoveOldBuffers();

    spdlog::memory_buf_t buffer;
    spdlog::sinks::base_sink<Mutex>::formatter_->format(msg, buffer);
    _buffers.emplace(msg.time, std::move(buffer));
}

template <typename Mutex>
void TimeBufferedFileSink<Mutex>::flush_() {
    RemoveOldBuffers();

    while (!_buffers.empty()) {
        const auto& buffer = _buffers.front().buffer;
        _file.write(buffer);
        _buffers.pop();
    }

    _file.flush();
}

template <typename Mutex>
void TimeBufferedFileSink<Mutex>::RemoveOldBuffers() {
    const auto now = std::chrono::system_clock::now();
    while (!_buffers.empty()) {
        auto age = std::chrono::duration_cast<std::chrono::milliseconds>(
            now - _buffers.front().time);
        if (age < _bufferDuration) {
            break;
        }
        _buffers.pop();
    }
}
