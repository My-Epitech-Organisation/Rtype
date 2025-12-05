# ============================================================================
# R-Type ECS Library
# ============================================================================

# Source files (explicitly listed for proper CMake dependency tracking)
set(ECS_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/core/Registry/RegistryEntity.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/core/Relationship.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/core/CommandBuffer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/core/Prefab.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/signal/SignalDispatcher.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/system/SystemScheduler.cpp
)

# Header files (explicitly listed for IDE integration)
set(ECS_HEADERS
    # Core
    ${CMAKE_CURRENT_SOURCE_DIR}/core/Entity.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/core/Relationship.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/core/CommandBuffer.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/core/Prefab.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/core/Registry/Registry.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/core/Registry/RegistryComponent.inl
    ${CMAKE_CURRENT_SOURCE_DIR}/core/Registry/RegistrySingleton.inl
    ${CMAKE_CURRENT_SOURCE_DIR}/core/Registry/RegistryView.inl
    ${CMAKE_CURRENT_SOURCE_DIR}/signal/SignalDispatcher.hpp
)

# Header-only library (no .cpp files yet or all are commented)
if(NOT ECS_SOURCES)
    add_library(rtype_ecs INTERFACE)
    target_include_directories(rtype_ecs INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/..
    )

    # C++20 standard requirement
    target_compile_features(rtype_ecs INTERFACE cxx_std_20)

    # Threading support
    find_package(Threads REQUIRED)
    target_link_libraries(rtype_ecs INTERFACE Threads::Threads)

    # Alias
    add_library(RType::ECS ALIAS rtype_ecs)
else()
    # Static library
    add_library(rtype_ecs STATIC ${ECS_SOURCES})
    target_include_directories(rtype_ecs PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/..
    )

    # C++20 standard requirement
    target_compile_features(rtype_ecs PUBLIC cxx_std_20)

    # Threading support
    find_package(Threads REQUIRED)
    target_link_libraries(rtype_ecs PUBLIC Threads::Threads)

    # Compiler warnings
    target_compile_options(rtype_ecs PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/W4>
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic>
    )

    # Alias
    add_library(RType::ECS ALIAS rtype_ecs)
endif()