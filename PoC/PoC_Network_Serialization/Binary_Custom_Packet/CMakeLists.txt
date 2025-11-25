cmake_minimum_required(VERSION 3.15)
project(BinaryCustomPacketPoC LANGUAGES CXX)

# C++20 standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Strict warning flags
add_compile_options(-Wall -Wextra -Werror -Wpedantic)

# Test executables
add_executable(test_binary_size test_binary_size.cpp)
add_executable(benchmark_binary benchmark_binary.cpp)
add_executable(compare_serialization compare_serialization.cpp)

# Set output directory
set_target_properties(
    test_binary_size
    benchmark_binary
    compare_serialization
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
