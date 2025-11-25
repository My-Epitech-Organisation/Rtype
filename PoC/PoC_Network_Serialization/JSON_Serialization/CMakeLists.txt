cmake_minimum_required(VERSION 3.15)
project(JSONSerializationPoC LANGUAGES CXX)

# C++20 standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Strict warning flags
add_compile_options(-Wall -Wextra -Werror -Wpedantic)

# Include CPM for package management
include(../../../cmake/CPM.cmake)

# Add nlohmann/json library
CPMAddPackage(
    NAME nlohmann_json
    VERSION 3.11.3
    GITHUB_REPOSITORY nlohmann/json
    OPTIONS
        "JSON_BuildTests OFF"
        "JSON_Install OFF"
)

# Test executables
add_executable(test_json_size test_json_size.cpp)
target_link_libraries(test_json_size PRIVATE nlohmann_json::nlohmann_json)

add_executable(benchmark_json benchmark_json.cpp)
target_link_libraries(benchmark_json PRIVATE nlohmann_json::nlohmann_json)

# Set output directory
set_target_properties(
    test_json_size
    benchmark_json
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
