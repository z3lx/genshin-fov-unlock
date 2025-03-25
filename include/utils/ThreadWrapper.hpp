#pragma once

#include <thread>
#include <type_traits>

#define THREADWRAPPER_TEMPLATE                                                  \
    template <typename Prologue, typename Body, typename Epilogue>              \
    requires std::is_invocable_r_v<void, Prologue> &&                           \
        std::is_nothrow_invocable_r_v<void, Body> &&                            \
        (std::is_nothrow_invocable_r_v<void, Epilogue> ||                       \
        std::is_nothrow_invocable_r_v<void, Epilogue, std::thread&>)

THREADWRAPPER_TEMPLATE
class ThreadWrapper {
public:
    ThreadWrapper(Prologue prologue, Body body, Epilogue epilogue);
    ~ThreadWrapper() noexcept;

private:
    Epilogue epilogue;
    std::thread thread;
};

#include "utils/ThreadWrapperInl.hpp"
