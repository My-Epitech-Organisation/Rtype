# ============================================================================
# ECS Library (Static)
# ============================================================================

set(ECS_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/Registry/RegistryEntity.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/Relationship.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/CommandBuffer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/Prefab.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/signal/SignalDispatcher.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/system/SystemScheduler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/serialization/Serialization.cpp
        ../background/ABackground.hpp
)

add_library(ecs STATIC ${ECS_SOURCES})

target_include_directories(ecs PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/../common/src
    ${CMAKE_CURRENT_SOURCE_DIR}/../common/src/Logger
    ${CMAKE_SOURCE_DIR}/include
)

target_compile_features(ecs PUBLIC cxx_std_20)

find_package(Threads REQUIRED)
target_link_libraries(ecs PUBLIC 
    Threads::Threads 
    common
)

if(MSVC)
    target_compile_options(ecs PRIVATE /W4)
else()
    target_compile_options(ecs PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_library(RType::ECS ALIAS ecs)
