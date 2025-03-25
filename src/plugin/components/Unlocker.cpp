#include "plugin/components/Unlocker.hpp"
#include "plugin/Events.hpp"
#include "plugin/interfaces/IMediator.hpp"
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
#include <utility>
#include <variant>

#include <Windows.h>

namespace {
constexpr auto OFFSET_GL = 0xFA2F70;
constexpr auto OFFSET_CN = 0xFA0F70;

void HkSetFieldOfView(void* instance, float value) noexcept;
void AddToBuffer(void* instance, float value);
std::string DumpBuffer();

std::mutex mutex {};
std::optional<MinHook<void, void*, float>> hook {};
ExponentialFilter<float> filter {};

bool isHooked = false;
bool isEnabled = false;
bool isEnabledOnce = false;
int overrideFov = 45;

int setFovCount = 0;
void* previousInstance = nullptr;
float previousFov = 45.0f;
bool isPreviousFov = false;
} // namespace

Unlocker::Unlocker(std::weak_ptr<IMediator<Event>> mediator) try
    : IComponent { std::move(mediator) } {
    std::lock_guard lock { mutex };

    auto module = reinterpret_cast<uintptr_t>(
        GetModuleHandle("GenshinImpact.exe"));
    const auto global = module ? true : (
        module = reinterpret_cast<uintptr_t>(
        GetModuleHandle("YuanShen.exe")), false);
    if (!module) {
        throw std::runtime_error {
            "Failed to get module handle due to unknown game"
        };
    }
    const auto offset = global ? OFFSET_GL : OFFSET_CN;
    const auto target = reinterpret_cast<void*>(module + offset);
    const auto detour = reinterpret_cast<void*>(HkSetFieldOfView);

    if (!hook) {
        hook.emplace();
    }
    hook->Create(target, detour);
} catch (const std::exception& e) {
    LOG_E("Failed to create Unlocker: {}", e.what());
    throw;
}

Unlocker::~Unlocker() noexcept {
    std::lock_guard lock { mutex };
    hook.reset();
}

void Unlocker::SetHook(const bool value) const {
    std::lock_guard lock { mutex };
    isHooked = value;
    if (value) {
        hook->Enable();
        isEnabledOnce = true;
    } else {
        // hook->Disable();
    }
}

void Unlocker::SetEnable(const bool value) const noexcept {
    isEnabled = value;
}

void Unlocker::SetFieldOfView(const int value) noexcept {
    overrideFov = value;
}

void Unlocker::SetSmoothing(const float value) noexcept {
    filter.SetTimeConstant(value);
}

namespace {
void HkSetFieldOfView(void* instance, float value) noexcept try {
    std::lock_guard lock { mutex };

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

        if (isEnabledOnce) {
            isEnabledOnce = false;
            filter.Update(value);
        }
        const float target = (isHooked && isEnabled) ?
            static_cast<float>(overrideFov) : previousFov;
        const float filtered = filter.Update(target);

        if ((isHooked && isEnabled) || !isPreviousFov) {
            isPreviousFov = std::abs(previousFov - filtered) < 0.1f;
            value = filtered;
        } else if (!isHooked) {
            isPreviousFov = false;
            hook->Disable();
        }
    } else {
        const auto rep = std::bit_cast<std::uint32_t>(value);
        value = std::bit_cast<float>(rep + 1); // marker value
        previousInstance = instance;
        previousFov = value;
    }

    // AddToBuffer(instance, value);
    hook->CallOriginal(instance, value);
} catch (const std::exception& e) {
    LOG_E("Failed to hook set field of view: {}", e.what());
}
} // namespace

#if false // TODO: Reimplement
namespace {
namespace sc = std::chrono;

std::queue<std::tuple<
    sc::steady_clock::time_point, uintptr_t, float>
> buffer {};

void AddToBuffer(void* instance, const float value) {
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

std::string DumpBuffer() {
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
} // namespace
#endif
