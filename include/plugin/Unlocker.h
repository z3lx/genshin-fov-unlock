#pragma once

#include "plugin/Events.h"
#include "plugin/IComponent.h"
#include "plugin/IMediator.h"
#include "utils/ExponentialFilter.h"
#include "utils/MinHook.h"

#include <chrono>
#include <cstdint>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <tuple>

class Unlocker final : public IComponent<Event> {
public:
    explicit Unlocker(const std::weak_ptr<IMediator<Event>>& mediator) noexcept;
    ~Unlocker() noexcept override;

    [[nodiscard]] bool IsCreated() const noexcept;
    void Create(bool value);
    [[nodiscard]] bool IsHooked() const noexcept;
    void Hook(bool value) const;
    [[nodiscard]] bool IsEnabled() const noexcept;
    void Enable(bool value) const;

    void SetFieldOfView(int value) noexcept;
    void SetSmoothing(float value) noexcept;

    void Handle(const Event& event) override;

private:
    struct Visitor {
        Unlocker& m;

        template <typename T>
        void operator()(const T& event) const;
    };

    static void HkSetFieldOfView(void* instance, float value) noexcept;

    static Unlocker* unlocker;
    static std::mutex mutex;
    static std::unique_ptr<MinHook<void, void*, float>> hook;
    static ExponentialFilter<float> filter;

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
