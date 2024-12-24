#pragma once

#include <filesystem>
#include <fstream>
#include <string>

class FileHandler {
public:
    FileHandler() noexcept;
    ~FileHandler() noexcept;

    void Open(const std::filesystem::path& filePath, bool truncate = false);
    void Close() noexcept;

    std::string Read();
    void Write(const std::string& data);
    void Flush();

    void SetWorkingDirectory(const std::filesystem::path& path);
    [[nodiscard]] std::filesystem::path GetWorkingDirectory() const noexcept;

private:
    std::fstream file;
    std::filesystem::path workingDirectory;
};
