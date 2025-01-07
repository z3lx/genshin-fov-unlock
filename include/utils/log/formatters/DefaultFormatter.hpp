#pragma once

#include "utils/log/Common.hpp"
#include "utils/log/formatters/IFormatter.hpp"

#include <string>

class DefaultFormatter final : public IFormatter {
public:
    DefaultFormatter();
    ~DefaultFormatter() override;
    std::string Format(const details::Message& message) override;
};
