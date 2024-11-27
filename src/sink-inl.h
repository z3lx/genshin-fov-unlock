#pragma once

template <typename Mutex>
TimeBufferedFileSink<Mutex>::TimeBufferedFileSink(
    const std::string& filename,
    const int bufferDuration,
    const bool truncate)
    : bufferDuration(std::chrono::milliseconds(bufferDuration)) {
    file.open(filename, truncate);
}

template <typename Mutex>
TimeBufferedFileSink<Mutex>::~TimeBufferedFileSink() {
    file.close();
}

template <typename Mutex>
void TimeBufferedFileSink<Mutex>::sink_it_(
    const spdlog::details::log_msg& msg) {
    RemoveOldBuffers();

    spdlog::memory_buf_t buffer;
    spdlog::sinks::base_sink<Mutex>::formatter_->format(msg, buffer);
    buffers.emplace(msg.time, std::move(buffer));
}

template <typename Mutex>
void TimeBufferedFileSink<Mutex>::flush_() {
    RemoveOldBuffers();

    while (!buffers.empty()) {
        const auto& buffer = buffers.front().buffer;
        file.write(buffer);
        buffers.pop();
    }

    file.flush();
}

template <typename Mutex>
void TimeBufferedFileSink<Mutex>::RemoveOldBuffers() {
    const auto now = std::chrono::system_clock::now();
    while (!buffers.empty()) {
        auto age = std::chrono::duration_cast<std::chrono::milliseconds>(
            now - buffers.front().time);
        if (age < bufferDuration) {
            break;
        }
        buffers.pop();
    }
}
