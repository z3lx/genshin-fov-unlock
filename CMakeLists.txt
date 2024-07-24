cmake_minimum_required(VERSION 3.28)
project(genshin-fov-unlock)

set(CMAKE_CXX_STANDARD 20)

function(download_and_extract url dir)
    set(ZIP_PATH "${dir}/temp.zip")
    file(DOWNLOAD ${url} ${ZIP_PATH})
    execute_process(
        COMMAND ${CMAKE_COMMAND} -E tar xzf ${ZIP_PATH}
        WORKING_DIRECTORY ${dir}
    )
    file(REMOVE ${ZIP_PATH})
endfunction()

set(DEPS_DIR "${CMAKE_BINARY_DIR}/deps")

# TsudaKageyu/minhook
set(MINHOOK_DIR "${DEPS_DIR}/minhook")
download_and_extract(
    "https://github.com/TsudaKageyu/minhook/releases/download/v1.3.3/MinHook_133_lib.zip"
    "${MINHOOK_DIR}"
)

# nlohmann/json
set(JSON_DIR "${DEPS_DIR}/json")
download_and_extract(
    "https://github.com/nlohmann/json/releases/download/v3.11.3/include.zip"
    "${JSON_DIR}"
)

add_library(
    genshin_fov_unlock
    SHARED
    "src/main.cpp"
)

target_include_directories(
    genshin_fov_unlock
    PRIVATE
    "src"
    ${MINHOOK_DIR}/include
    ${JSON_DIR}/include
)

target_link_libraries(
    genshin_fov_unlock
    PRIVATE
    ${MINHOOK_DIR}/lib/libMinHook-x64-v141-mt.lib
)
