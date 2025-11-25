cmake_minimum_required(VERSION 3.17)
project(AABBCollisionPoC)

# C++20 required for constexpr features and spaceship operator
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Compiler flags
if(MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Build ECS library if not already built
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../ECS ${CMAKE_CURRENT_BINARY_DIR}/ECS)

# AABB Collision PoC executable
add_executable(aabb_collision_poc
    main.cpp
)

# Include directories
target_include_directories(aabb_collision_poc PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

# Link threading library and ECS library
find_package(Threads REQUIRED)
target_link_libraries(aabb_collision_poc PRIVATE 
    Threads::Threads
    rtype_ecs_static
)

# Enable testing
enable_testing()

# Add the test
add_test(NAME AABBCollisionTest COMMAND aabb_collision_poc)

# Print build information
message(STATUS "AABB Collision PoC configured")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Build Type: ${CMAKE_BUILD_TYPE}")
