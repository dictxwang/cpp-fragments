cmake_minimum_required(VERSION 3.10)

set(PROJECT_NAME "cpp_fragments")
set(PROCESS_MAIN "process_main")
set(PROCESS_TCP_SERVER "process_tcp_server")
set(PROCESS_WS_CLIENT "process_ws_client")
set(PROCESS_WSS_CLIENT "process_wss_client")
set(PROCESS_HTTPS_CLIENT "process_https_client")
set(PROCESS_EVENT_BASE "process_event_base")
set(PROCESS_ED25519_SODIUM "process_ed25519_sodium")
set(PROCESS_ED25519_OPENSSL "process_ed25519_openssl")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20")
# set(CMAKE_BUILD_TYPE "Release")
set(CMAKE_BUILD_TYPE "Debug")

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(CUSTOM_LIB_DIR "/usr/lib/x86_64-linux-gnu")
    set(CUSTOM_INCLUDE_DIR "/usr/lib/x86_64-linux-gnu")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(CUSTOM_LIB_DIR "/usr/local/lib")
    set(CUSTOM_INCLUDE_DIR "/usr/local/include")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(CUSTOM_LIB_DIR "/usr/local/lib")
    set(CUSTOM_INCLUDE_DIR "/usr/local/include")
else()
    message(WARNING "Unknown platform: ${CMAKE_SYSTEM_NAME}")
endif()

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    include
    src
    3rdparty/rapidjson/include
    3rdparty/spdlog/include
    ${CUSTOM_INCLUDE_DIR}
)

link_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CUSTOM_LIB_DIR}
)

# Find OpenSSL
find_package(OpenSSL REQUIRED)
# Find Curl
find_package(CURL REQUIRED)
# Find Sodium
# find_package(Sodium REQUIRED)
# Find Event
# find_package(LIBEVENT REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBEVENT REQUIRED libevent)

if("${LIBEVENT_LIBRARY}" STREQUAL "")
    # set(LIBEVENT_INCLUDE_DIR "/custom/path/to/libevent/include")
    set(LIBEVENT_LIBRARY "${CUSTOM_LIB_DIR}/libevent.a")
endif()

# For print variables
# message(STATUS ">>>>> LIBEVENT_INCLUDE_DIRS = ${LIBEVENT_INCLUDE_DIRS}")
# message(STATUS ">>>>> LIBEVENT_LIBRARY = ${LIBEVENT_LIBRARY}")

# util
FILE(GLOB_RECURSE LIB_UTIL_SRC RELATIVE ${CMAKE_SOURCE_DIR}
    "src/util/*")
add_library(util ${LIB_UTIL_SRC})

# config
FILE(GLOB_RECURSE LIB_CONFIG_SRC RELATIVE ${CMAKE_SOURCE_DIR}
    "src/config/*")
add_library(config ${LIB_CONFIG_SRC})

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
# Include CURL headers
target_include_directories(${PROCESS_HTTPS_CLIENT} PRIVATE ${CURL_INCLUDE_DIR})
target_include_directories(${PROCESS_HTTPS_CLIENT} PRIVATE ${OPENSSL_INCLUDE_DIR})
target_link_libraries(${PROCESS_HTTPS_CLIENT}
    PRIVATE OpenSSL::SSL OpenSSL::Crypto
    PRIVATE CURL::libcurl
    config
)

add_executable(${PROCESS_EVENT_BASE} "src/event/event_base.cpp")
target_include_directories(${PROCESS_EVENT_BASE} PRIVATE ${LIBEVENT_INCLUDE_DIRS})
target_link_libraries(${PROCESS_EVENT_BASE} PRIVATE ${LIBEVENT_LIBRARY})

add_executable(${PROCESS_ED25519_SODIUM} "src/encrypt/ed25519_sodium.cpp")
target_link_libraries(${PROCESS_ED25519_SODIUM}
    PRIVATE Sodium
)

add_executable(${PROCESS_ED25519_OPENSSL} "src/encrypt/ed25519_openssl.cpp")
target_link_libraries(${PROCESS_ED25519_OPENSSL}
    PRIVATE OpenSSL::SSL OpenSSL::Crypto
)