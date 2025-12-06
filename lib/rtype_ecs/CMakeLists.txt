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
)

add_library(rtype_ecs STATIC ${ECS_SOURCES})

target_include_directories(rtype_ecs PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/../rtype_common/src
    ${CMAKE_CURRENT_SOURCE_DIR}/../rtype_common/src/Logger
    ${CMAKE_SOURCE_DIR}/include
)

target_compile_features(rtype_ecs PUBLIC cxx_std_20)

find_package(Threads REQUIRED)
target_link_libraries(rtype_ecs PUBLIC 
    Threads::Threads 
    rtype_common
)

if(MSVC)
    target_compile_options(rtype_ecs PRIVATE /W4)
else()
    target_compile_options(rtype_ecs PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_library(RType::ECS ALIAS rtype_ecs)
