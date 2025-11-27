cmake_minimum_required(VERSION 3.15)

# Prefer Clang over GCC for better diagnostics and modern C++ support
# Override with: cmake -DCMAKE_CXX_COMPILER=g++ -DCMAKE_C_COMPILER=gcc ..
if(NOT DEFINED CMAKE_CXX_COMPILER AND NOT DEFINED ENV{CXX})
    find_program(CLANG_CXX clang++)
    if(CLANG_CXX)
        set(CMAKE_CXX_COMPILER ${CLANG_CXX})
        set(CMAKE_C_COMPILER clang)
    endif()
endif()

project(r-type LANGUAGES CXX)

# Set C++20 standard globally
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Options
option(BUILD_TESTS "Build tests" ON)
option(ENABLE_COVERAGE "Enable code coverage reporting" OFF)

# Coverage configuration
if(ENABLE_COVERAGE)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        message(STATUS "Code coverage enabled")
        set(COVERAGE_COMPILE_FLAGS "--coverage -fprofile-arcs -ftest-coverage")
        set(COVERAGE_LINK_FLAGS "--coverage")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COVERAGE_COMPILE_FLAGS}")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${COVERAGE_COMPILE_FLAGS}")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${COVERAGE_LINK_FLAGS}")
        set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${COVERAGE_LINK_FLAGS}")
    else()
        message(WARNING "Code coverage is only supported with GCC or Clang")
    endif()
endif()

# Add source directory
add_subdirectory(src)

# Add tests if enabled
if(BUILD_TESTS)
    enable_testing()

    # Fetch Google Test
    include(FetchContent)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.14.0
    )
    # For Windows: Prevent overriding the parent project's compiler/linker settings
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    add_subdirectory(tests)
endif()
