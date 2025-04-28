cmake_minimum_required(VERSION 3.10)

set(PROJECT_NAME "cpp_fragments")
set(PROCESS_MAIN "process_main")
set(PROCESS_TCP_SERVER "process_tcp_server")
set(PROCESS_WS_CLIENT "process_ws_client")
set(PROCESS_WSS_CLIENT "process_wss_client")
set(PROCESS_HTTPS_CLIENT "process_https_client")

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

# Find OpenSSL
find_package(OpenSSL REQUIRED)
# Find Curl
find_package(CURL REQUIRED)

# util
FILE(GLOB_RECURSE LIB_UTIL_SRC RELATIVE ${CMAKE_SOURCE_DIR}
    "src/util/*")
add_library(util ${LIB_UTIL_SRC})

# config
FILE(GLOB_RECURSE LIB_UTIL_SRC RELATIVE ${CMAKE_SOURCE_DIR}
    "src/config/*")
add_library(config ${LIB_UTIL_SRC})

add_executable(${PROCESS_MAIN} "src/main.cpp")

add_executable(${PROCESS_TCP_SERVER} "src/net/tcp_server.cpp")

add_executable(${PROCESS_WS_CLIENT} "src/net/ws_client.cpp")
# Include OpenSSL headers
target_include_directories(${PROCESS_WS_CLIENT} PRIVATE ${OPENSSL_INCLUDE_DIR})
# Link OpenSSL libraries
target_link_libraries(${PROCESS_WS_CLIENT} PRIVATE OpenSSL::SSL OpenSSL::Crypto)

add_executable(${PROCESS_WSS_CLIENT} "src/net/wss_client.cpp")
# Include OpenSSL headers
target_include_directories(${PROCESS_WSS_CLIENT} PRIVATE ${OPENSSL_INCLUDE_DIR})
# Link OpenSSL libraries
target_link_libraries(${PROCESS_WSS_CLIENT}
    PRIVATE OpenSSL::SSL OpenSSL::Crypto
    util
    config
)

add_executable(${PROCESS_HTTPS_CLIENT} "src/net/https_client.cpp")
target_link_libraries(${PROCESS_HTTPS_CLIENT}
    PRIVATE OpenSSL::SSL OpenSSL::Crypto
    PRIVATE CURL::libcurl
    config
)