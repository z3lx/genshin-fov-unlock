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
    utils
    STATIC
    src/utils/ExponentialFilter.h
    src/utils/ExponentialFilterInl.h
    src/utils/FileHandler.cpp
    src/utils/FileHandler.h
    src/utils/Hook.cpp
    src/utils/Hook.h
    src/utils/HookInl.h
    $<$<BOOL:${ENABLE_LOGGING}>:src/utils/TimeBufferedFileSink.h>
    $<$<BOOL:${ENABLE_LOGGING}>:src/utils/TimeBufferedFileSinkInl.h>
)

target_include_directories(
    utils
    PRIVATE
    src
)

target_link_libraries(
    utils
    PRIVATE
    minhook
    $<$<BOOL:${ENABLE_LOGGING}>:spdlog::spdlog>
)

add_library(
    genshin_fov_unlock
    SHARED
    src/plugin/ConfigManager.cpp
    src/plugin/ConfigManager.h
    src/plugin/DllMain.cpp
    src/plugin/Events.h
    src/plugin/IComponent.h
    src/plugin/IMediator.h
    src/plugin/InputManager.cpp
    src/plugin/InputManager.h
    src/plugin/Plugin.cpp
    src/plugin/Plugin.h
    src/plugin/Unlocker.cpp
    src/plugin/Unlocker.h
)

target_include_directories(
    genshin_fov_unlock
    PRIVATE
    src
)

target_link_libraries(
    genshin_fov_unlock
    PRIVATE
    nlohmann_json::nlohmann_json
    $<$<BOOL:${ENABLE_LOGGING}>:spdlog::spdlog>
    utils
)

target_compile_definitions(
    genshin_fov_unlock
    PRIVATE
    $<$<BOOL:${ENABLE_LOGGING}>:ENABLE_LOGGING>
)
