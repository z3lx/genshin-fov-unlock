#pragma once

template <typename Mutex>
TimeBufferedFileSink<Mutex>::TimeBufferedFileSink(
    const std::string& filename,
    const int bufferDuration,
    const bool truncate)
    : _bufferDuration(std::chrono::milliseconds(bufferDuration)) {
    _file.open(filename, truncate);
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
