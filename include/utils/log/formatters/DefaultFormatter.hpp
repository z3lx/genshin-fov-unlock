#pragma once

#include "utils/log/Common.hpp"
#include "utils/log/formatters/IFormatter.hpp"

#include <cstddef>
#include <sstream>
#include <string>
#include <string_view>

class DefaultFormatter final : public IFormatter {
public:
    DefaultFormatter();
    ~DefaultFormatter() override;
    std::string Format(const details::Message& message) override;

private:
    static std::string TruncateOrPad(std::string_view input, size_t length);

    std::stringstream ss;
};
