cmake_minimum_required(VERSION 3.28)
project(genshin-fov-unlock)

set(CMAKE_CXX_STANDARD 23)

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

# libcpr/cpr
FetchContent_Declare(
    cpr
    GIT_REPOSITORY https://github.com/libcpr/cpr.git
    GIT_TAG 1.11.1
)
FetchContent_MakeAvailable(cpr)

set(MAJOR_VERSION 1)
set(MINOR_VERSION 5)
set(PATCH_VERSION 0)

set(VERSION_RC ${CMAKE_CURRENT_BINARY_DIR}/version.rc)
configure_file(
    res/version.rc.in
    ${VERSION_RC}
    @ONLY
)

option(ENABLE_LOGGING "Enable logging" OFF)
if (ENABLE_LOGGING)
    add_compile_definitions(ACTIVE_LEVEL=LEVEL_TRACE)
else()
    add_compile_definitions(ACTIVE_LEVEL=LEVEL_OFF)
endif()

file(GLOB_RECURSE UTILS_SOURCES include/utils/* src/utils/*)
add_library(utils STATIC ${UTILS_SOURCES})
target_include_directories(utils PRIVATE include)
target_link_libraries(utils PRIVATE minhook)

file(GLOB_RECURSE LOADER_SOURCES include/loader/* src/loader/*)
add_executable(loader ${LOADER_SOURCES} ${VERSION_RC})
target_include_directories(loader PRIVATE include)
target_link_libraries(loader PRIVATE
    cpr::cpr
    nlohmann_json::nlohmann_json
    utils
    version.lib
)

file(GLOB_RECURSE PLUGIN_SOURCES include/plugin/* src/plugin/*)
add_library(genshin_fov_unlock SHARED ${PLUGIN_SOURCES} ${VERSION_RC})
target_include_directories(genshin_fov_unlock PRIVATE include)
target_link_libraries(genshin_fov_unlock PRIVATE
    nlohmann_json::nlohmann_json
    utils
)
