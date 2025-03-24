#pragma once

#include "plugin/Events.hpp"
#include "plugin/interfaces/IComponent.hpp"
#include "plugin/interfaces/IMediator.hpp"
#include "utils/ExponentialFilter.hpp"
#include "utils/MinHook.hpp"

#include <chrono>
#include <cstdint>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <tuple>

class Unlocker final : public IComponent<Event> {
public:
    explicit Unlocker(std::weak_ptr<IMediator<Event>> mediator);
    ~Unlocker() noexcept override;

    void SetHook(bool value) const;
    void SetEnable(bool value) const noexcept;
    void SetFieldOfView(int value) noexcept;
    void SetSmoothing(float value) noexcept;

private:
    static void HkSetFieldOfView(void* instance, float value) noexcept;

    static std::mutex mutex;
    static std::unique_ptr<MinHook<void, void*, float>> hook;
    static ExponentialFilter<float> filter;

    static bool isHooked; // TODO: Refactor
    static bool isEnabled;
    static int overrideFov;

    static int setFovCount;
    static void* previousInstance;
    static float previousFov;
    static bool isPreviousFov;

    static void AddToBuffer(void* instance, float value);
    static std::string DumpBuffer();

    static std::queue<std::tuple<
        std::chrono::steady_clock::time_point, uintptr_t, float
    >> buffer;
};
