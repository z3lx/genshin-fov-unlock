#include "utils/log/sinks/ConsoleSink.hpp"

#include <iostream>
#include <ostream>
#include <string_view>

ConsoleSink::ConsoleSink() = default;
ConsoleSink::~ConsoleSink() = default;

void ConsoleSink::Write(const std::string_view data) {
    std::cout << data;
}

void ConsoleSink::Flush() {
    std::cout << std::flush;
}
