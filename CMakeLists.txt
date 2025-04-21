cmake_minimum_required(VERSION 3.10)

set(PROJECT_NAME "cpp_fragments")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20")
set(CMAKE_BUILD_TYPE "Release")

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    include
    src
    3rdparty/rapidjson/include
    3rdparty/spdlog/include
)

add_executable(${PROJECT_NAME} "src/main.cpp")