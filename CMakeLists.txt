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

# Include CPM for package management
include(cmake/CPM.cmake)

# Options
option(BUILD_TESTS "Build tests" ON)
option(BUILD_EXAMPLES "Build examples" ON)

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

# Add examples if enabled
if(BUILD_EXAMPLES)
    # Add ASIO standalone (for asio_poc)
    CPMAddPackage(
        NAME asio
        GITHUB_REPOSITORY chriskohlhoff/asio
        GIT_TAG asio-1-30-2
        OPTIONS
            "ASIO_STANDALONE ON"
    )

    if(asio_ADDED)
        add_library(asio INTERFACE)
        target_include_directories(asio INTERFACE ${asio_SOURCE_DIR}/asio/include)
        target_compile_definitions(asio INTERFACE ASIO_STANDALONE ASIO_NO_DEPRECATED)

        # Platform-specific dependencies
        if(WIN32)
            target_link_libraries(asio INTERFACE ws2_32 wsock32)
        else()
            target_link_libraries(asio INTERFACE pthread)
        endif()
    endif()

    # Add Boost via CPM (for boost_asio_poc)
    CPMAddPackage(
        NAME Boost
        VERSION 1.84.0
        URL https://github.com/boostorg/boost/releases/download/boost-1.84.0/boost-1.84.0.tar.xz
        OPTIONS
            "BOOST_ENABLE_CMAKE ON"
            "BOOST_INCLUDE_LIBRARIES asio\;system\;date_time"
    )

    add_subdirectory(PoC/PoC_Network_Libs)
endif()
