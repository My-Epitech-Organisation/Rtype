cmake_minimum_required(VERSION 3.20)
project(RTypeECS LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(rtype_ecs
    src/main.cpp
    src/ECS/Core/Entity.hpp
    src/ECS/Core/Registry.hpp
    src/ECS/Core/Registry.cpp
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

add_executable(rtype_ecs_test_advanced
    src/test_advanced.cpp
    src/ECS/Core/Registry.cpp
    src/ECS/Core/CommandBuffer.cpp
    src/ECS/Core/Relationship.cpp
    src/ECS/Core/Prefab.cpp
    src/ECS/Signal/SignalDispatcher.cpp
    src/ECS/System/SystemScheduler.cpp
    src/ECS/Serialization/Serialization.cpp
)

add_executable(rtype_ecs_test_new_features
    src/test_new_features.cpp
    src/ECS/Core/Registry.cpp
    src/ECS/Core/CommandBuffer.cpp
    src/ECS/Core/Relationship.cpp
    src/ECS/Core/Prefab.cpp
    src/ECS/Signal/SignalDispatcher.cpp
    src/ECS/System/SystemScheduler.cpp
    src/ECS/Serialization/Serialization.cpp
)

add_executable(rtype_ecs_test_features
    src/test_features.cpp
    src/ECS/Core/Registry.cpp
    src/ECS/Core/CommandBuffer.cpp
    src/ECS/Core/Relationship.cpp
    src/ECS/Core/Prefab.cpp
    src/ECS/Signal/SignalDispatcher.cpp
    src/ECS/System/SystemScheduler.cpp
    src/ECS/Serialization/Serialization.cpp
)

add_executable(rtype_ecs_test_stress
    src/test_stress.cpp
    src/ECS/Core/Registry.cpp
    src/ECS/Core/CommandBuffer.cpp
    src/ECS/Core/Relationship.cpp
    src/ECS/Core/Prefab.cpp
    src/ECS/Signal/SignalDispatcher.cpp
    src/ECS/System/SystemScheduler.cpp
    src/ECS/Serialization/Serialization.cpp
)

target_include_directories(rtype_ecs PRIVATE src)
target_include_directories(rtype_ecs_test_advanced PRIVATE src)
target_include_directories(rtype_ecs_test_new_features PRIVATE src)
target_include_directories(rtype_ecs_test_features PRIVATE src)
target_include_directories(rtype_ecs_test_stress PRIVATE src)
