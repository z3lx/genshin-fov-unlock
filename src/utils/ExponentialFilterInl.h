#pragma once

#include <chrono>
#include <cmath>

template <typename T>
ExponentialFilter<T>::ExponentialFilter(
    const T timeConstant,
    const T initialValue) noexcept
    : timeConstant(static_cast<T>(0))
    , lastFilteredValue(static_cast<T>(0)) {
    SetTimeConstant(timeConstant);
    SetInitialValue(initialValue);
}

template <typename T>
void ExponentialFilter<T>::SetTimeConstant(const T value) noexcept {
    timeConstant = value;
}

template <typename T>
void ExponentialFilter<T>::SetInitialValue(const T value) noexcept {
    lastFilteredValue = value;
    lastTime = std::chrono::steady_clock::now();
}

template <typename T>
T ExponentialFilter<T>::Update(const T value) noexcept {
    // y(k) = alpha * y(k - 1) + (1 - alpha) * x(k)
    // alpha = exp(-T / tau)

    // Where:
    // x(k) is the raw input at time step k
    // y(k) is the filtered output at time step k
    // T is delta time between the current and previous time steps
    // tau is the time constant

    if (timeConstant <= static_cast<T>(0)) {
        return value;
    }

    const auto currentTime = std::chrono::steady_clock::now();
    const T deltaTime = std::chrono::duration_cast<std::chrono::duration<T>>(
        currentTime - lastTime).count();
    lastTime = currentTime;

    const T alpha = std::exp(-deltaTime / timeConstant);
    lastFilteredValue = alpha * lastFilteredValue +
        (static_cast<T>(1) - alpha) * value;
    return lastFilteredValue;
}
