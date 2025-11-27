cmake_minimum_required(VERSION 3.15)

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
set(CMAKE_CXX_EXTENSIONS OFF)

option(BUILD_TESTS "Build tests" ON)
option(BUILD_EXAMPLES "Build examples" ON)

option(USE_SFML "Build with SFML graphics backend" ON)

find_package(asio CONFIG REQUIRED)
find_package(SFML COMPONENTS Graphics Window System CONFIG REQUIRED)

add_subdirectory(src)

if(BUILD_TESTS)
    enable_testing()
    find_package(GTest CONFIG REQUIRED)
    add_subdirectory(tests)
endif()

if(BUILD_EXAMPLES)
    add_subdirectory(PoC/PoC_Network_Libs)
endif()
