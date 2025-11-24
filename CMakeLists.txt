cmake_minimum_required(VERSION 3.20)
project(RType VERSION 1.0.0 LANGUAGES CXX)

if(NOT DEFINED CMAKE_CXX_COMPILER AND NOT DEFINED ENV{CXX})
    find_program(CLANG_CXX clang++)
    if(CLANG_CXX)
        set(CMAKE_CXX_COMPILER ${CLANG_CXX})
        set(CMAKE_C_COMPILER clang)
    endif()
endif()
project(r-type LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Options
option(BUILD_TESTS "Build test suite" ON)
option(BUILD_POC "Build Proof of Concepts" ON)

# Add subdirectories
add_subdirectory(src/ECS)

if(BUILD_POC)
    add_subdirectory(PoC)
endif()

# Tests (if enabled)
if(BUILD_TESTS)
    enable_testing()
    # Add test subdirectories here
endif()
