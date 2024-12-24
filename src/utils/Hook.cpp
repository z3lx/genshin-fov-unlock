#include "utils/Hook.h"

#include <MinHook.h>

#include <format>
#include <stdexcept>
#include <unordered_map>

struct Status {
    bool isCreated;
    bool isEnabled;
};

bool isInitialized = false;
std::unordered_map<void*, Status> hooks;

void CheckStatus(const MH_STATUS status) {
    if (status == MH_OK) {
        return;
    }
    throw std::runtime_error(std::format(
        "MinHook failed with status {}",
        MH_StatusToString(status)
    ));
}

bool detail::HookShared::IsInitialized() noexcept {
    return isInitialized;
}

void detail::HookShared::Initialize() {
    CheckStatus(MH_Initialize());
    isInitialized = true;
}

void detail::HookShared::Uninitialize() {
    CheckStatus(MH_Uninitialize());
    isInitialized = false;
    hooks.clear();
}

bool detail::HookShared::IsCreated(void* target) noexcept {
    return hooks.contains(target) && hooks[target].isCreated;
}

void detail::HookShared::Create(void* target, void* detour, void** original) {
    CheckStatus(MH_CreateHook(target, detour, original));
    hooks[target].isCreated = true;
}

void detail::HookShared::Remove(void* target) {
    CheckStatus(MH_RemoveHook(target));
    hooks.erase(target);
}

bool detail::HookShared::IsEnabled(void* target) noexcept {
    return hooks.contains(target) && hooks[target].isEnabled;
}

void detail::HookShared::Enable(void* target) {
    CheckStatus(MH_EnableHook(target));
    hooks[target].isEnabled = true;
}

void detail::HookShared::Disable(void* target) {
    CheckStatus(MH_DisableHook(target));
    hooks[target].isEnabled = false;
}
