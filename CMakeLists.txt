cmake_minimum_required(VERSION 3.27)
project(FitGalgoFrontendC++)

set(CMAKE_CXX_STANDARD 20)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wpedantic")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")

set(SOURCES
  src/main.cpp
  src/core/api.cpp
  src/core/date.cpp
  src/core/monitor.cpp
)

include_directories(include)

add_executable(fitgalgo ${SOURCES})

set(TEST_SOURCES
  test/test_core_date.cpp
  test/test_monitor.cpp

  src/core/api.cpp
  src/core/date.cpp
  src/core/monitor.cpp)

find_package(Catch2 3 REQUIRED)
# These tests can use the Catch2-provided main
add_executable(tests ${TEST_SOURCES})
target_link_libraries(tests PRIVATE Catch2::Catch2WithMain sqlite3)
# These tests need their own main
#add_executable(custom-main-tests test.cpp test-main.cpp)
#target_link_libraries(custom-main-tests PRIVATE Catch2::Catch2)

target_compile_definitions(tests PRIVATE ES_RGMF_FITGALGO_TESTING=1)

# Enable testing
include(CTest)
include(Catch)

# Add tests to CTest
catch_discover_tests(tests)
