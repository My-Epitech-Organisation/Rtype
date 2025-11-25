cmake_minimum_required(VERSION 3.15)
project(MiniPoC LANGUAGES CXX)

# C++20 standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Strict warning flags
add_compile_options(
    -Wall
    -Wextra
    -Werror
    -Wpedantic
)

# Simple executable
add_executable(hello_strict hello.cpp)

# Output to bin directory
set_target_properties(hello_strict
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
