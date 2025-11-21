cmake_minimum_required(VERSION 3.20)
project(RTypeECS LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find SFML
find_package(SFML 2 COMPONENTS graphics window system QUIET)
if(NOT SFML_FOUND)
    # Try with pkg-config
    find_package(PkgConfig)
    if(PKG_CONFIG_FOUND)
        pkg_check_modules(SFML sfml-graphics sfml-window sfml-system)
    endif()
endif()

add_executable(rtype_ecs
    tests.cpp
    src/ECS/Core/Entity.hpp
    src/ECS/Core/Registry.hpp
    src/ECS/Core/Registry/Registry.hpp
    src/ECS/Core/Registry/RegistryEntity.cpp
    src/ECS/Core/Registry/RegistryComponent.inl
    src/ECS/Core/Registry/RegistrySingleton.inl
    src/ECS/Core/Registry/RegistryView.inl
    src/ECS/Core/CommandBuffer.hpp
    src/ECS/Core/CommandBuffer.cpp
    src/ECS/Core/Relationship.hpp
    src/ECS/Core/Relationship.cpp
    src/ECS/Core/Prefab.hpp
    src/ECS/Core/Prefab.cpp
    src/ECS/Storage/ISparseSet.hpp
    src/ECS/Storage/SparseSet.hpp
    src/ECS/Storage/TagSparseSet.hpp
    src/ECS/Traits/ComponentTraits.hpp
    src/ECS/Signal/SignalDispatcher.hpp
    src/ECS/Signal/SignalDispatcher.cpp
    src/ECS/View/View.hpp
    src/ECS/View/ParallelView.hpp
    src/ECS/View/Group.hpp
    src/ECS/View/ExcludeView.hpp
    src/ECS/System/SystemScheduler.hpp
    src/ECS/System/SystemScheduler.cpp
    src/ECS/Utils/Benchmark.hpp
    src/ECS/Serialization/Serialization.hpp
    src/ECS/Serialization/Serialization.cpp
    src/ECS/ECS.hpp
)

target_include_directories(rtype_ecs PRIVATE src)
