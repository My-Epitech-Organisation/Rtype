cmake_minimum_required(VERSION 3.20)
project(RType_ECS LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Collect all source files
set(ECS_SOURCES
    Core/Registry/RegistryEntity.cpp
    Core/CommandBuffer.cpp
    Core/Relationship.cpp
    Core/Prefab.cpp
    Signal/SignalDispatcher.cpp
    System/SystemScheduler.cpp
    Serialization/Serialization.cpp
)

# Collect all header files
set(ECS_HEADERS
    ECS.hpp
    Core/Entity.hpp
    Core/Registry.hpp
    Core/Registry/Registry.hpp
    Core/Registry/RegistryComponent.inl
    Core/Registry/RegistrySingleton.inl
    Core/Registry/RegistryView.inl
    Core/CommandBuffer.hpp
    Core/Relationship.hpp
    Core/Prefab.hpp
    Storage/ISparseSet.hpp
    Storage/SparseSet.hpp
    Storage/TagSparseSet.hpp
    Traits/ComponentTraits.hpp
    Signal/SignalDispatcher.hpp
    View/View.hpp
    View/ParallelView.hpp
    View/Group.hpp
    View/ExcludeView.hpp
    System/SystemScheduler.hpp
    Utils/Benchmark.hpp
    Serialization/Serialization.hpp
)

# Create static library
add_library(rtype_ecs_static STATIC ${ECS_SOURCES} ${ECS_HEADERS})
target_include_directories(rtype_ecs_static PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

# Create shared library
add_library(rtype_ecs_shared SHARED ${ECS_SOURCES} ${ECS_HEADERS})
target_include_directories(rtype_ecs_shared PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

# Set output names
set_target_properties(rtype_ecs_static PROPERTIES OUTPUT_NAME "rtype_ecs")
set_target_properties(rtype_ecs_shared PROPERTIES OUTPUT_NAME "rtype_ecs")

# Add threading support (required for parallel processing)
find_package(Threads REQUIRED)
target_link_libraries(rtype_ecs_static PUBLIC Threads::Threads)
target_link_libraries(rtype_ecs_shared PUBLIC Threads::Threads)

# Compiler warnings
if(MSVC)
    target_compile_options(rtype_ecs_static PRIVATE /W4)
    target_compile_options(rtype_ecs_shared PRIVATE /W4)
else()
    target_compile_options(rtype_ecs_static PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(rtype_ecs_shared PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Installation rules
install(TARGETS rtype_ecs_static rtype_ecs_shared
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

# Install headers
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
    DESTINATION include/ECS
    FILES_MATCHING PATTERN "*.hpp" PATTERN "*.inl"
)

# Create alias targets for easier usage
add_library(RType::ECS::Static ALIAS rtype_ecs_static)
add_library(RType::ECS::Shared ALIAS rtype_ecs_shared)

# Export library information
export(TARGETS rtype_ecs_static rtype_ecs_shared
    FILE "${CMAKE_CURRENT_BINARY_DIR}/RTypeECSTargets.cmake"
    NAMESPACE RType::ECS::
)

# ============================================================================
# TEST EXECUTABLE
# ============================================================================

add_executable(ecs_test test_main.cpp)
target_link_libraries(ecs_test PRIVATE rtype_ecs_static Threads::Threads)
target_include_directories(ecs_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
