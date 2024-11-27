#pragma once

#include <chrono>
#include <type_traits>

template <typename T>
class ExponentialFilter {
    static_assert(
        std::is_floating_point_v<T>,
        "T must be a floating-point type"
    );

public:
    explicit ExponentialFilter(
        T timeConstant = static_cast<T>(0),
        T initialValue = static_cast<T>(0)
    ) noexcept;
    ~ExponentialFilter() noexcept = default;

    void SetTimeConstant(T value) noexcept;
    void SetInitialValue(T value) noexcept;
    T Update(T value) noexcept;

private:
    T timeConstant;
    T lastFilteredValue;
    std::chrono::time_point<std::chrono::steady_clock> lastTime;
};

#include "ExponentialFilterInl.h"
