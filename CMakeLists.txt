cmake_minimum_required(VERSION 3.28)
project(genshin-fov-unlock)

if (NOT WIN32)
    message(
        FATAL_ERROR
        "This project can only be compiled on Windows."
    )
endif()

if (NOT MSVC OR NOT ${CMAKE_VS_PLATFORM_TOOLSET} STREQUAL v141)
    message(
        WARNING
        "The precompiled MinHook library is built with MSVC v141 (VS 2017). "
        "If you encounter issues, consider recompiling MinHook from source."
    )
endif()

set(CMAKE_CXX_STANDARD 20)

function(download_and_extract url dir)
    set(ZIP_PATH ${dir}/temp.zip)
    file(DOWNLOAD ${url} ${ZIP_PATH})
    execute_process(
        COMMAND ${CMAKE_COMMAND} -E tar xzf ${ZIP_PATH}
        WORKING_DIRECTORY ${dir}
    )
    file(REMOVE ${ZIP_PATH})
endfunction()

set(DEPS_DIR ${CMAKE_BINARY_DIR}/deps)

# TsudaKageyu/minhook
set(MINHOOK_DIR ${DEPS_DIR}/minhook)
download_and_extract(
    https://github.com/TsudaKageyu/minhook/releases/download/v1.3.3/MinHook_133_lib.zip
    ${MINHOOK_DIR}
)

# nlohmann/json
set(JSON_DIR ${DEPS_DIR}/json)
download_and_extract(
    https://github.com/nlohmann/json/releases/download/v3.11.3/include.zip
    ${JSON_DIR}
)

# gabime/spdlog
include(FetchContent)
FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.12.0
)
FetchContent_GetProperties(spdlog)
if(NOT spdlog_POPULATED)
    FetchContent_Populate(spdlog)
endif()
add_subdirectory(${spdlog_SOURCE_DIR})


add_library(
    genshin_fov_unlock
    SHARED
    src/config.cpp
    src/config.h
    src/dllmain.cpp
    src/filter.h
    src/hook.cpp
    src/hook.h
    src/input.cpp
    src/input.h
    src/plugin.cpp
    src/plugin.h
    src/sink.h
    src/sink-inl.h
)

target_include_directories(
    genshin_fov_unlock
    PRIVATE
    src
    ${MINHOOK_DIR}/include
    ${JSON_DIR}/include
    ${spdlog_SOURCE_DIR}/include
)

target_link_libraries(
    genshin_fov_unlock
    PRIVATE
    ${MINHOOK_DIR}/lib/libMinHook-x64-v141-mt.lib
)

option(ENABLE_LOGGING "Enable logging" OFF)
if (ENABLE_LOGGING)
    target_compile_definitions(genshin_fov_unlock PRIVATE ENABLE_LOGGING)
endif()
