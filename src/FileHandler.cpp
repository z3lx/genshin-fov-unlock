#include "FileHandler.h"

#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>

namespace fs = std::filesystem;

FileHandler::FileHandler() noexcept
    : workingDirectory(fs::current_path()) { }

FileHandler::~FileHandler() noexcept {
    Close();
}

void FileHandler::Open(const fs::path& filePath, const bool truncate) {
    Close();

    const auto path = filePath.is_relative() ?
        workingDirectory / filePath : filePath;
    const auto mode = std::ios::in | std::ios::out |
        (truncate ? std::ios::trunc : std::ios::app);
    file.open(path, mode);

    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + path.string());
    }
}

void FileHandler::Close() noexcept {
    if (file.is_open()) {
        file.close();
    }
}

std::string FileHandler::Read() {
    if (!file.is_open()) {
        throw std::logic_error("File is not open");
    }

    file.seekg(0, std::ios::beg);
    return std::string {
        std::istreambuf_iterator(file),
        std::istreambuf_iterator<char>()
    };
}

void FileHandler::Write(const std::string& data) {
    if (!file.is_open()) {
        throw std::logic_error("File is not open");
    }

    file.seekp(0, std::ios::end);
    file << data;
    if (!file) {
        throw std::runtime_error("Failed to write to file");
    }
}

void FileHandler::Flush() {
    if (!file.is_open()) {
        throw std::logic_error("File is not open");
    }

    file.flush();
    if (!file) {
        throw std::runtime_error("Failed to flush file");
    }
}

void FileHandler::SetWorkingDirectory(const fs::path& path) {
    if (!fs::exists(path) || !fs::is_directory(path)) {
        throw std::invalid_argument(
            "Invalid working directory: " + path.string()
        );
    }
    workingDirectory = path;
}

fs::path FileHandler::GetWorkingDirectory() const noexcept {
    return workingDirectory;
}
