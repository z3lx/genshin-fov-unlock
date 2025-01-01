#pragma once

#include "utils/log/sinks/ISink.h"

#include <string_view>

class ConsoleSink final : public ISink {
public:
    ConsoleSink();
    ~ConsoleSink() override;

    void Write(std::string_view data) override;
    void Flush() override;
};
