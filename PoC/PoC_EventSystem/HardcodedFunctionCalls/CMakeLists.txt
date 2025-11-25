cmake_minimum_required(VERSION 3.20)
project(PoC_HardcodedFunctionCalls)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find ECS library
set(ECS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../ECS)
add_subdirectory(${ECS_DIR} ${CMAKE_BINARY_DIR}/ECS)

# Include ECS headers
include_directories(${ECS_DIR})

add_executable(poc_hardcoded
    main.cpp
    Game.cpp
    PhysicsSystem.cpp
)

target_link_libraries(poc_hardcoded 
    rtype_ecs_static
    pthread
)
