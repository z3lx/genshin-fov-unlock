cmake_minimum_required(VERSION 3.28)
project(genshin-fov-unlock)

set(CMAKE_CXX_STANDARD 20)

if (MSVC)
    add_compile_options(/Zc:preprocessor)
endif()

include(FetchContent)

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

option(ENABLE_LOGGING "Enable logging" OFF)
if (ENABLE_LOGGING)
    add_compile_definitions(ACTIVE_LEVEL=LEVEL_TRACE)
else()
    add_compile_definitions(ACTIVE_LEVEL=LEVEL_OFF)
endif()

add_library(
    utils
    STATIC
    src/utils/log/formatters/DefaultFormatter.cpp
    src/utils/log/formatters/DefaultFormatter.h
    src/utils/log/formatters/IFormatter.h
    src/utils/log/sinks/ConsoleSink.cpp
    src/utils/log/sinks/ConsoleSink.h
    src/utils/log/sinks/FileSink.cpp
    src/utils/log/sinks/FileSink.h
    src/utils/log/sinks/ISink.h
    src/utils/log/Common.h
    src/utils/log/Logger.h
    src/utils/log/LoggerInl.h
    src/utils/ExponentialFilter.h
    src/utils/ExponentialFilterInl.h
    src/utils/MinHook.cpp
    src/utils/MinHook.h
    src/utils/MinHookInl.h
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
    utils
)
