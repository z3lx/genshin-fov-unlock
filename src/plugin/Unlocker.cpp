// ReSharper disable CppMemberFunctionMayBeStatic
// NOLINTBEGIN(*-convert-member-functions-to-static)

#include "plugin/Unlocker.hpp"
#include "plugin/Events.hpp"
#include "plugin/IMediator.hpp"
#include "utils/ExponentialFilter.hpp"
#include "utils/MinHook.hpp"
#include "utils/log/Logger.hpp"

#include <nlohmann/json.hpp>

#include <bit>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <tuple>
#include <variant>

#include <Windows.h>

constexpr auto OFFSET_GL = 0xF08370;
constexpr auto OFFSET_CN = 0xF0A370;

std::mutex Unlocker::mutex {};
std::unique_ptr<MinHook<void, void*, float>> Unlocker::hook = nullptr;
ExponentialFilter<float> Unlocker::filter {};

bool Unlocker::isEnabled = false;
int Unlocker::overrideFov = 45;

int Unlocker::setFovCount = 0;
void* Unlocker::previousInstance = nullptr;
float Unlocker::previousFov = 0.0f;
bool Unlocker::isPreviousFov = false;

Unlocker::Unlocker(const std::weak_ptr<IMediator<Event>>& mediator)
    : IComponent(mediator) {
    std::lock_guard lock { mutex };

    auto module = reinterpret_cast<uintptr_t>(GetModuleHandle("GenshinImpact.exe"));
    const auto global = module ? true :
        (module = reinterpret_cast<uintptr_t>(GetModuleHandle("YuanShen.exe")), false);
    if (!module) {
        throw std::runtime_error("Failed to get module handle due to unknown game");
    }
    const auto offset = global ? OFFSET_GL : OFFSET_CN;
    const auto target = reinterpret_cast<void*>(module + offset);
    const auto detour = reinterpret_cast<void*>(HkSetFieldOfView);

    if (!hook) {
        hook = std::make_unique<MinHook<void, void*, float>>();
    }
    hook->Create(target, detour);
}

Unlocker::~Unlocker() noexcept {
    std::lock_guard lock { mutex };
    hook->Remove();
    hook = nullptr;
}

void Unlocker::SetHook(const bool value) const {
    std::lock_guard lock { mutex };
    if (value) {
        hook->Enable();
    } else {
        hook->Disable();
    }
}

void Unlocker::SetEnable(const bool value) const {
    isEnabled = value;
}

void Unlocker::SetFieldOfView(const int value) noexcept {
    overrideFov = value;
}

void Unlocker::SetSmoothing(const float value) noexcept {
    filter.SetTimeConstant(value);
}

void Unlocker::HkSetFieldOfView(void* instance, float value) noexcept {
    std::lock_guard lock { mutex };
    try {
        if (!hook->IsEnabled()) {
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

            const float target = isEnabled ?
                static_cast<float>(overrideFov) : previousFov;
            const float filtered = filter.Update(target);

            if (isEnabled || !isPreviousFov) {
                isPreviousFov = std::abs(previousFov - filtered) < 0.1f;
                value = filtered;
            }
        } else {
            const auto rep = std::bit_cast<std::uint32_t>(value);
            value = std::bit_cast<float>(rep + 1); // marker value
            previousInstance = instance;
            previousFov = value;
        }

        AddToBuffer(instance, value);
        hook->CallOriginal(instance, value);
    } catch (const std::exception& e) {
        LOG_E("Failed to set field of view: {}", e.what());
    }
}

void Unlocker::Handle(const Event& event) {
    std::visit(Visitor { *this }, event);
}

template <typename T>
void Unlocker::Visitor::operator()(const T& event) const { }

template <>
void Unlocker::Visitor::operator()(const OnHookToggle& event) const try {
    LOG_D("Handling OnHookToggle event with hooked = {}", event.hooked);
    m.SetHook(event.hooked);
    LOG_D("OnHookToggle event handled");
} catch (const std::exception& e) {
    LOG_E("Failed to handle OnHookToggle event: {}", e.what());
    throw;
}

template <>
void Unlocker::Visitor::operator()(const OnEnableToggle& event) const try {
    LOG_D("Handling OnEnableToggle event with enabled = {}", event.enabled);
    m.SetEnable(event.enabled);
    LOG_D("OnEnableToggle event handled");
} catch (const std::exception& e) {
    LOG_E("Failed to handle OnEnableToggle event: {}", e.what());
    throw;
}

template <>
void Unlocker::Visitor::operator()(const OnFovChange& event) const try {
    LOG_D("Handling OnFovChange event with fov = {}", event.fov);
    m.SetFieldOfView(event.fov);
    LOG_D("OnFovChange event handled");
} catch (const std::exception& e) {
    LOG_E("Failed to handle OnFovChange event: {}", e.what());
    throw;
}

template <>
void Unlocker::Visitor::operator()(const OnSmoothingChange& event) const try {
    LOG_D("Handling OnSmoothingChange event with smoothing = {}", event.smoothing);
    m.SetSmoothing(static_cast<float>(event.smoothing));
    LOG_D("OnSmoothingChange event handled");
} catch (const std::exception& e) {
    LOG_E("Failed to handle OnSmoothingChange event: {}", e.what());
    throw;
}

template<>
void Unlocker::Visitor::operator()(const OnDumpBuffer& event) const try {
    LOG_D("Handling OnDumpBuffer event");
    LOG_D(DumpBuffer());
    LOG_D("OnDumpBuffer event handled");
} catch (const std::exception& e) {
    LOG_E("Failed to handle OnDumpBuffer event: {}", e.what());
    throw;
}

namespace sc = std::chrono;

std::queue<std::tuple<
    sc::steady_clock::time_point, uintptr_t, float>
> Unlocker::buffer {};

void Unlocker::AddToBuffer(void* instance, const float value) {
    const auto now = sc::steady_clock::now();
    while (!buffer.empty()) {
        const auto [time, instance, value] = buffer.front();
        if (const auto elapsed = now - time;
            elapsed < sc::seconds(10)) {
            break;
        }
        buffer.pop();
    }
    buffer.emplace(now, reinterpret_cast<uintptr_t>(instance), value);
}

std::string Unlocker::DumpBuffer() {
    using namespace nlohmann;

    std::lock_guard lock { mutex };
    if (buffer.empty()) {
        return "[]";
    }

    ordered_json j = ordered_json::array();
    const auto firstTime = std::get<0>(buffer.front());
    while (!buffer.empty()) {
        const auto [time, instance, value] = buffer.front();
        const auto elapsed = sc::duration<double>(time - firstTime).count();
        j.push_back({
            { "time", elapsed },
            { "instance", instance },
            { "value", value }
        });
        buffer.pop();
    }
    return j.dump();
}

// NOLINTEND(*-convert-member-functions-to-static)
