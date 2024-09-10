#pragma once

#include <memory>

class ExponentialFilter {
public:
    explicit ExponentialFilter(
        float timeConstant,
        float initialValue = 0.0f
    );
    ~ExponentialFilter();
    void SetTimeConstant(float value);
    void SetInitialValue(float value);
    float Update(float value);

private:
    struct Impl;
    std::unique_ptr<Impl> impl;
};
