cmake_minimum_required(VERSION 3.15)
project(r-type LANGUAGES CXX)

# Minimal top-level CMake for skeleton project
set(CMAKE_CXX_STANDARD 17)

option(BUILD_TESTS "Build tests" ON)

add_subdirectory(src)
