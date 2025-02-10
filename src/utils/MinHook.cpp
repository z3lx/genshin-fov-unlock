#include "utils/MinHook.hpp"

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

size_t details::MinHookBackend::Count() noexcept {
    return hooks.size();
}

bool details::MinHookBackend::IsInitialized() noexcept {
    return isInitialized;
}

void details::MinHookBackend::Initialize() {
    CheckStatus(MH_Initialize());
    isInitialized = true;
}

void details::MinHookBackend::Uninitialize() {
    CheckStatus(MH_Uninitialize());
    isInitialized = false;
    hooks.clear();
}

bool details::MinHookBackend::IsCreated(void* target) noexcept {
    return hooks.contains(target) && hooks[target].isCreated;
}

void details::MinHookBackend::Create(void* target, void* detour, void** original) {
    CheckStatus(MH_CreateHook(target, detour, original));
    hooks[target].isCreated = true;
}

void details::MinHookBackend::Remove(void* target) {
    CheckStatus(MH_RemoveHook(target));
    hooks.erase(target);
}

bool details::MinHookBackend::IsEnabled(void* target) noexcept {
    return hooks.contains(target) && hooks[target].isEnabled;
}

void details::MinHookBackend::Enable(void* target) {
    CheckStatus(MH_EnableHook(target));
    hooks[target].isEnabled = true;
}

void details::MinHookBackend::Disable(void* target) {
    CheckStatus(MH_DisableHook(target));
    hooks[target].isEnabled = false;
}
