#pragma once

#include "Events.h"
#include "ExponentialFilter.h"
#include "Hook.h"
#include "IComponent.h"
#include "IMediator.h"

#include <memory>
#include <mutex>

class Unlocker final : public IComponent<Event> {
public:
    explicit Unlocker(const std::weak_ptr<IMediator<Event>>& mediator);
    ~Unlocker() override;

    [[nodiscard]] bool IsCreated() const noexcept;
    void Create(bool value) const;
    [[nodiscard]] bool IsEnabled() const noexcept;
    void Enable(bool value) noexcept;

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
    static Hook<void, void*, float> hook;
    static ExponentialFilter<float> filter;

    static bool enabled;
    static int overrideFov;

    static int setFovCount;
    static void* previousInstance;
    static float previousFov;
    static bool isPreviousFov;
};
