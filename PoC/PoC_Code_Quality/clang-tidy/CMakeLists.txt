cmake_minimum_required(VERSION 3.15)

project(clang-tidy-poc LANGUAGES CXX)

# Set C++20 standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Export compile commands for clang-tidy
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Enable clang-tidy with the local config
set(CMAKE_CXX_CLANG_TIDY clang-tidy;--config-file=${CMAKE_SOURCE_DIR}/.clang-tidy)

# Add the bad_code executable
add_executable(bad_code bad_code.cpp)