#include "filter.h"

#include <chrono>
#include <cmath>

ExponentialFilter::ExponentialFilter(
    const float timeConstant,
    const float initialValue)
    : _timeConstant(0)
    , _lastFilteredValue(0) {
    SetTimeConstant(timeConstant);
    SetInitialValue(initialValue);
}

void ExponentialFilter::SetTimeConstant(const float value) {
    _timeConstant = value;
}

void ExponentialFilter::SetInitialValue(const float value) {
    _lastFilteredValue = value;
    _lastTime = std::chrono::steady_clock::now();
}

float ExponentialFilter::Update(const float value) {
    // y(k) = alpha * y(k - 1) + (1 - alpha) * x(k)
    // alpha = exp(-T / tau)
    //
    // Where:
    // x(k) is the raw input at time step k
    // y(k) is the filtered output at time step k
    // T is delta time between the current and previous time steps
    // tau is the time constant

    if (_timeConstant <= 0) {
        return value;
    }

    const auto currentTime = std::chrono::steady_clock::now();
    const auto deltaTime = std::chrono::duration_cast<std::chrono::duration<float>>(
        currentTime - _lastTime).count();
    _lastTime = currentTime;

    const auto alpha = std::expf(-deltaTime / _timeConstant);
    _lastFilteredValue = alpha * _lastFilteredValue + (1.0f - alpha) * value;
    return _lastFilteredValue;
}
