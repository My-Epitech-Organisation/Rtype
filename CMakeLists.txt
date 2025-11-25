cmake_minimum_required(VERSION 3.28)
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

# Include CPM for package management
include(cmake/CPM.cmake)

# Options
option(BUILD_TESTS "Build test suite" ON)
option(BUILD_POC "Build Proof of Concepts" ON)


if(BUILD_POC)
    add_subdirectory(PoC/ECS)
endif()

# Tests (if enabled)
if(BUILD_TESTS)
    enable_testing()
    # Add test subdirectories here
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
