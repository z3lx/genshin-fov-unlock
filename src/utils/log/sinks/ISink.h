#pragma once

#include <string_view>

class ISink {
public:
    ISink() = default;
    virtual ~ISink() = default;

    virtual void Write(std::string_view data) = 0;
    virtual void Flush() = 0;
};
