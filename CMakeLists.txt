cmake_minimum_required(VERSION 3.28)
project(genshin-fov-unlock)

set(CMAKE_CXX_STANDARD 20)

include(FetchContent)

option(ENABLE_LOGGING "Enable logging" OFF)

# TsudaKageyu/minhook
FetchContent_Declare(
    minhook
    GIT_REPOSITORY https://github.com/TsudaKageyu/minhook.git
    GIT_COMMIT c1a7c3843bd1a5fe3eb779b64c0d823bca3dc339
)
FetchContent_MakeAvailable(minhook)

# nlohmann/json
FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
)
FetchContent_MakeAvailable(json)
set(JSON_BuildTests OFF CACHE INTERNAL "")

if (ENABLE_LOGGING)
    # gabime/spdlog
    FetchContent_Declare(
        spdlog
        GIT_REPOSITORY https://github.com/gabime/spdlog.git
        GIT_TAG v1.12.0
    )
    FetchContent_MakeAvailable(spdlog)
endif()

add_library(
    genshin_fov_unlock
    SHARED
    src/Config.cpp
    src/Config.h
    src/DllMain.cpp
    src/ExponentialFilter.h
    src/Hook.cpp
    src/Hook.h
    src/InputManager.cpp
    src/InputManager.h
    src/Plugin.cpp
    src/Plugin.h
    $<$<BOOL:${ENABLE_LOGGING}>:src/TimeBufferedFileSink.h>
    $<$<BOOL:${ENABLE_LOGGING}>:src/TimeBufferedFileSinkInl.h>
)

target_link_libraries(
    genshin_fov_unlock
    PRIVATE
    minhook
    nlohmann_json::nlohmann_json
    $<$<BOOL:${ENABLE_LOGGING}>:spdlog::spdlog>
)

target_compile_definitions(
    genshin_fov_unlock
    PRIVATE
    $<$<BOOL:${ENABLE_LOGGING}>:ENABLE_LOGGING>
)
