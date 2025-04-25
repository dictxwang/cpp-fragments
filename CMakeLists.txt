cmake_minimum_required(VERSION 3.10)

set(PROJECT_NAME "cpp_fragments")
set(PROCESS_MAIN "process_main")
set(PROCESS_TCP_SERVER "process_tcp_server")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20")
# set(CMAKE_BUILD_TYPE "Release")
set(CMAKE_BUILD_TYPE "Debug")

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    include
    src
    3rdparty/rapidjson/include
    3rdparty/spdlog/include
)

add_executable(${PROCESS_MAIN} "src/main.cpp")

add_executable(${PROCESS_TCP_SERVER} "src/net/tcp_server.cpp")