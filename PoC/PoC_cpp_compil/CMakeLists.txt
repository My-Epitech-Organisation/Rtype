cmake_minimum_required(VERSION 3.15)
project(CompilerComparisonPoC LANGUAGES CXX)

# Set C++20 standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Strict warning flags
set(STRICT_FLAGS
    -Wall
    -Wextra
    -Werror
    -Wpedantic
)

# Additional GCC-specific warnings
set(GCC_EXTRA_FLAGS
    -Wshadow
    -Wformat=2
    -Wunused
)

# Additional Clang-specific warnings
set(CLANG_EXTRA_FLAGS
    -Wshadow
    -Wformat=2
    -Wunused
)

# Apply compiler-specific flags
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    add_compile_options(${STRICT_FLAGS} ${GCC_EXTRA_FLAGS})
    message(STATUS "Using GCC with strict flags")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    add_compile_options(${STRICT_FLAGS} ${CLANG_EXTRA_FLAGS})
    message(STATUS "Using Clang with strict flags")
else()
    message(WARNING "Unknown compiler: ${CMAKE_CXX_COMPILER_ID}")
    add_compile_options(${STRICT_FLAGS})
endif()

# Test executables
add_executable(test_cpp20_features test_cpp20_features.cpp)
add_executable(test_strict_warnings test_strict_warnings.cpp)
add_executable(test_edge_cases test_edge_cases.cpp)

# Set output directory
set_target_properties(
    test_cpp20_features
    test_strict_warnings
    test_edge_cases
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
