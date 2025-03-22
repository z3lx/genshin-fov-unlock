#pragma once

#include "utils/ThreadWrapper.hpp"

#include <exception>
#include <future>
#include <thread>
#include <type_traits>

THREADWRAPPER_TEMPLATE
ThreadWrapper<Prologue, Body, Epilogue>::ThreadWrapper(
    Prologue prologue, Body body, Epilogue epilogue)
    : epilogue { epilogue } {
    std::promise<void> promise {};
    std::future<void> future = promise.get_future();
    const auto set = [&promise, prologue, body]() {
        try {
            prologue();
        } catch (...) {
            promise.set_exception(std::current_exception());
            return;
        }
        promise.set_value();
        body();
    };
    thread = std::thread { set };
    try {
        future.get();
    } catch (...) {
        thread.join();
        throw;
    }
}

THREADWRAPPER_TEMPLATE
ThreadWrapper<Prologue, Body, Epilogue>::~ThreadWrapper() noexcept {
    if constexpr (std::is_invocable_r_v<void, Epilogue>) {
        epilogue();
        thread.join();
    } else {
        epilogue(thread);
        if (thread.joinable()) {
            thread.join();
        }
    }
}

#undef THREADWRAPPER_TEMPLATE
