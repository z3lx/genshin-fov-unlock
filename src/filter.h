#pragma once

#include <chrono>

class ExponentialFilter {
public:
    explicit ExponentialFilter(
        float timeConstant = 0.0,
        float initialValue = 0.0
    );
    ~ExponentialFilter() = default;

    void SetTimeConstant(float value);
    void SetInitialValue(float value);
    float Update(float value);

private:
    float _timeConstant;
    float _lastFilteredValue;
    std::chrono::time_point<std::chrono::steady_clock> _lastTime;
};
