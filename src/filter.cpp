#include "filter.h"
#include <chrono>

namespace chrono = std::chrono;

struct ExponentialFilter::Impl {
    Impl(float timeConstant, float initialValue);
    void SetTimeConstant(float value);
    void SetInitialValue(float value);
    float Update(float value);

    float timeConstant;
    float lastFilteredValue;
    chrono::time_point<chrono::steady_clock> lastTime;
};

ExponentialFilter::Impl::Impl(
    const float timeConstant,
    const float initialValue
) : timeConstant(0)
  , lastFilteredValue(0)
{
    SetTimeConstant(timeConstant);
    SetInitialValue(initialValue);
}

void ExponentialFilter::Impl::SetTimeConstant(const float value) {
    timeConstant = value;
}

void ExponentialFilter::Impl::SetInitialValue(const float value) {
    lastFilteredValue = value;
    lastTime = chrono::steady_clock::now();
}

float ExponentialFilter::Impl::Update(const float value) {
    // y(k) = alpha * y(k - 1) + (1 - alpha) * x(k)
    // alpha = exp(-T / tau)
    //
    // Where:
    // x(k) is the raw input at time step k
    // y(k) is the filtered output at time step k
    // T is delta time between the current and previous time steps
    // tau is the time constant

    const auto currentTime = chrono::steady_clock::now();
    const auto deltaTime = chrono::duration_cast<chrono::duration<float>>(
        currentTime - lastTime).count();
    lastTime = currentTime;

    const auto alpha = std::expf(-deltaTime / timeConstant);
    lastFilteredValue = alpha * lastFilteredValue + (1.0f - alpha) * value;
    return lastFilteredValue;
}

ExponentialFilter::ExponentialFilter(
    const float timeConstant,
    const float initialValue
) : impl(std::make_unique<Impl>(timeConstant, initialValue)) {
}

ExponentialFilter::~ExponentialFilter() = default;

void ExponentialFilter::SetInitialValue(const float value) {
    impl->SetInitialValue(value);
}

void ExponentialFilter::SetTimeConstant(const float value) {
    impl->SetTimeConstant(value);
}

float ExponentialFilter::Update(const float value) {
    return impl->Update(value);
}
