cmake_minimum_required(VERSION 3.27)
project(FitGalgoFrontendC++)

set(CMAKE_CXX_STANDARD 20)

find_package(OpenSSL REQUIRED)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wpedantic")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")

set(SOURCES
    src/main.cpp
    src/core/api.cpp
    src/core/stats.cpp
    src/ui/shell.cpp
    src/ui/calendar.cpp
    src/ui/repr.cpp
    src/ui/colors.cpp
)

include_directories(
    include
    ${OPENSSL_INCLUDE_DIR}
)

add_definitions(-DCPPHTTPLIB_OPENSSL_SUPPORT)

add_executable(fitgalgo ${SOURCES})

#target_link_libraries(fitgalgo PUBLIC ${OPENSSL_LIBRARIES})
target_link_libraries(fitgalgo OpenSSL::SSL OpenSSL::Crypto)
