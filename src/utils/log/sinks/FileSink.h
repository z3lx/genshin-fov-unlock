#pragma once

#include "utils/log/sinks/ISink.h"

#include <filesystem>
#include <fstream>
#include <string_view>

class FileSink final : public ISink {
public:
    explicit FileSink(
        const std::filesystem::path& filePath,
        bool truncate = false
    );
    ~FileSink() noexcept override;

    void Write(std::string_view data) override;
    void Flush() override;

private:
    std::ofstream file;
};
