cmake_minimum_required(VERSION 3.20)
project(ScriptedMovementPoC CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add the ECS library path
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../..)

# ECS source files
set(ECS_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/../../ECS/Core/Registry/RegistryEntity.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../ECS/Signal/SignalDispatcher.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../ECS/Core/CommandBuffer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../ECS/Core/Relationship.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../ECS/Core/Prefab.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../ECS/System/SystemScheduler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../ECS/Serialization/Serialization.cpp
)

add_executable(scripted_movement_poc
    main.cpp
    ${ECS_SOURCES}
)

target_link_libraries(scripted_movement_poc PRIVATE
    pthread
)

# Copy the script file to build directory
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/movement_script.txt
    ${CMAKE_CURRENT_BINARY_DIR}/movement_script.txt
    COPYONLY
)
