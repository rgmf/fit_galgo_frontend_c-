cmake_minimum_required(VERSION 3.27)
project(FitGalgoFrontendC++)

set(CMAKE_CXX_STANDARD 20)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")

set(SOURCES src/main.cpp src/config/Context.cpp src/net/HttpClient.cpp src/net/HttpClientException.cpp)

include_directories(include)

add_executable(fitgalgo ${SOURCES})
