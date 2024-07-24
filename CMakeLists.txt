cmake_minimum_required(VERSION 3.28)
project(genshin-fov-unlock)

set(CMAKE_CXX_STANDARD 20)

# MinHook
set(MINHOOK_URL "https://github.com/TsudaKageyu/minhook/releases/download/v1.3.3/MinHook_133_lib.zip")
set(MINHOOK_DIR "${CMAKE_BINARY_DIR}/libs/MinHook")
set(MINHOOK_ZIP "${MINHOOK_DIR}/MinHook_133_bin.zip")
file(DOWNLOAD ${MINHOOK_URL} ${MINHOOK_ZIP})
execute_process(
    COMMAND ${CMAKE_COMMAND} -E tar xzf ${MINHOOK_ZIP}
    WORKING_DIRECTORY ${MINHOOK_DIR}
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
)

target_link_libraries(
    genshin_fov_unlock
    PRIVATE
    ${MINHOOK_DIR}/lib/libMinHook-x64-v141-mt.lib
)
