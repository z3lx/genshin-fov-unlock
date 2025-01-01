#include "utils/log/sinks/FileSink.h"

#include <filesystem>
#include <ios>
#include <ostream>
#include <stdexcept>
#include <string_view>

FileSink::FileSink(const std::filesystem::path& filePath, const bool truncate) {
    const auto mode = std::ios::out |
        (truncate ? std::ios::trunc : std::ios::app);
    file.open(filePath, mode);

    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + filePath.string());
    }
}

FileSink::~FileSink() noexcept {
    if (file.is_open()) {
        file.close();
    }
}

void FileSink::Write(const std::string_view data) {
    file << data;
    if (!file) {
        throw std::runtime_error("Failed to write to file");
    }
}

void FileSink::Flush() {
    file << std::flush;
    if (!file) {
        throw std::runtime_error("Failed to flush file");
    }
}
