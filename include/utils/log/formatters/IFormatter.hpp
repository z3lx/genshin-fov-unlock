#pragma once

#include "utils/log/Common.hpp"

#include <string>

class IFormatter {
public:
    IFormatter() = default;
    virtual ~IFormatter() = default;
    virtual std::string Format(const details::Message& message) = 0;
};
