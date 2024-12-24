// ReSharper disable CppMemberFunctionMayBeStatic
// NOLINTBEGIN(*-convert-member-functions-to-static)

#include "plugin/Unlocker.h"
#include "plugin/Events.h"
#include "plugin/IMediator.h"
#include "utils/ExponentialFilter.h"
#include "utils/Hook.h"

#include <bit>
#include <cmath>
#include <cstdint>
#include <memory>
#include <mutex>
#include <variant>

#include <Windows.h>

constexpr auto OFFSET_GL = 0x13F87C0;
constexpr auto OFFSET_CN = 0x13F38A0;

Unlocker* Unlocker::unlocker = nullptr;
std::mutex Unlocker::mutex {};
Hook<void, void*, float> Unlocker::hook {};
ExponentialFilter<float> Unlocker::filter {};

bool Unlocker::enabled = false;
int Unlocker::overrideFov = 45;

int Unlocker::setFovCount = 0;
void* Unlocker::previousInstance = nullptr;
float Unlocker::previousFov = 0.0f;
bool Unlocker::isPreviousFov = false;

Unlocker::Unlocker(const std::weak_ptr<IMediator<Event>>& mediator) try
    : IComponent(mediator) {
    std::lock_guard lock(mutex);

    // TODO: Refactor GetModuleHandle redundancy
    const auto module = reinterpret_cast<uintptr_t>(GetModuleHandle(nullptr));
    const auto global = GetModuleHandle("GenshinImpact.exe") != nullptr;
    const auto offset = global ? OFFSET_GL : OFFSET_CN;
    const auto target = reinterpret_cast<void*>(module + offset);
    const auto detour = reinterpret_cast<void*>(HkSetFieldOfView);

    hook.Initialize();
    hook.Create(target, detour);
    unlocker = this;
} catch (const std::exception& e) {
    // TODO: LOG ERROR
}

Unlocker::~Unlocker() {
    std::lock_guard lock(mutex);
    hook.Uninitialize();
    unlocker = nullptr;
}

bool Unlocker::IsCreated() const noexcept {
    return hook.IsCreated();
}

void Unlocker::Create(const bool value) const {
    std::lock_guard lock(mutex);
    if (value) {
        hook.Enable();
    } else {
        hook.Disable();
    }
}

bool Unlocker::IsEnabled() const noexcept {
    return enabled;
}

void Unlocker::Enable(const bool value) noexcept {
    enabled = value;
}

void Unlocker::SetFieldOfView(const int value) noexcept {
    overrideFov = value;
}

void Unlocker::SetSmoothing(const float value) noexcept {
    filter.SetTimeConstant(value);
}

void Unlocker::HkSetFieldOfView(void* instance, float value) noexcept {
    std::lock_guard lock(mutex);
    try {
        if (!hook.IsEnabled() || !unlocker) {
            return;
        }

        ++setFovCount;
        if (const bool isDefaultFov = value == 45.0f;
            instance == previousInstance &&
            (value == previousFov || isDefaultFov)) {
            if (isDefaultFov) {
                previousInstance = instance;
                previousFov = value;
            }

            if (setFovCount > 8) {
                filter.SetInitialValue(value);
            }
            setFovCount = 0;

            const float target = enabled ?
                static_cast<float>(overrideFov) : previousFov;
            const float filtered = filter.Update(target);

            if (enabled || !isPreviousFov) {
                isPreviousFov = std::abs(previousFov - filtered) < 0.1f;
                value = filtered;
            }
        } else {
            const auto rep = std::bit_cast<std::uint32_t>(value);
            value = std::bit_cast<float>(rep + 1); // marker value
            previousInstance = instance;
            previousFov = value;
        }

        hook.CallOriginal(instance, value);
    } catch (const std::exception& e) {
        // TODO: LOG ERROR
    }
}

void Unlocker::Handle(const Event& event) {
    std::visit(Visitor { *this }, event);
}

template <typename T>
void Unlocker::Visitor::operator()(const T& event) const { }

template <>
void Unlocker::Visitor::operator()(const OnCreateToggle& event) const {
    m.Create(event.created);
}

template <>
void Unlocker::Visitor::operator()(const OnEnableToggle& event) const {
    m.Enable(event.enabled);
}

template <>
void Unlocker::Visitor::operator()(const OnFovChange& event) const {
    m.SetFieldOfView(event.fov);
}

template <>
void Unlocker::Visitor::operator()(const OnSmoothingChange& event) const {
    m.SetSmoothing(static_cast<float>(event.smoothing));
}

// NOLINTEND(*-convert-member-functions-to-static)
