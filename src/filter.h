#pragma once

#include <chrono>
#include <cmath>
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
    );
    ~ExponentialFilter() = default;

    void SetTimeConstant(T value);
    void SetInitialValue(T value);
    T Update(T value);

private:
    T _timeConstant;
    T _lastFilteredValue;
    std::chrono::time_point<std::chrono::steady_clock> _lastTime;
};

template <typename T>
ExponentialFilter<T>::ExponentialFilter(
    const T timeConstant,
    const T initialValue)
    : _timeConstant(static_cast<T>(0))
    , _lastFilteredValue(static_cast<T>(0)) {
    SetTimeConstant(timeConstant);
    SetInitialValue(initialValue);
}

template <typename T>
void ExponentialFilter<T>::SetTimeConstant(const T value) {
    _timeConstant = value;
}

template <typename T>
void ExponentialFilter<T>::SetInitialValue(const T value) {
    _lastFilteredValue = value;
    _lastTime = std::chrono::steady_clock::now();
}

template <typename T>
T ExponentialFilter<T>::Update(const T value) {
    // y(k) = alpha * y(k - 1) + (1 - alpha) * x(k)
    // alpha = exp(-T / tau)

    // Where:
    // x(k) is the raw input at time step k
    // y(k) is the filtered output at time step k
    // T is delta time between the current and previous time steps
    // tau is the time constant

    if (_timeConstant <= static_cast<T>(0)) {
        return value;
    }

    const auto currentTime = std::chrono::steady_clock::now();
    const T deltaTime = std::chrono::duration_cast<std::chrono::duration<T>>(
        currentTime - _lastTime).count();
    _lastTime = currentTime;

    const T alpha = std::exp(-deltaTime / _timeConstant);
    _lastFilteredValue = alpha * _lastFilteredValue +
        (static_cast<T>(1) - alpha) * value;
    return _lastFilteredValue;
}
