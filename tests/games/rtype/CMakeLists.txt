add_executable(test_spawning test_spawning.cpp)

target_link_libraries(test_spawning PRIVATE
    rtype_game_shared
    rtype_game_server
    engine
    GTest::gtest_main
)

add_executable(test_movement_system test_movement_system.cpp)

target_link_libraries(test_movement_system PRIVATE
    rtype_game_shared
    GTest::gtest_main
)

target_include_directories(test_spawning PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
)

include(GoogleTest)
gtest_discover_tests(test_spawning)
gtest_discover_tests(test_movement_system)

# Component tests
add_executable(test_components test_components.cpp)

target_link_libraries(test_components PRIVATE
    rtype_game_shared
    engine
    GTest::gtest_main
)

target_include_directories(test_components PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
)

gtest_discover_tests(test_components)

# AI Behavior tests
add_executable(test_behaviors test_behaviors.cpp)

target_link_libraries(test_behaviors PRIVATE
    rtype_game_shared
    engine
    GTest::gtest_main
)

target_include_directories(test_behaviors PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
)

gtest_discover_tests(test_behaviors)

# Behavior Registry tests
add_executable(test_behavior_registry test_behavior_registry.cpp)

target_link_libraries(test_behavior_registry PRIVATE
    rtype_game_shared
    engine
    GTest::gtest_main
)

target_include_directories(test_behavior_registry PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
)

gtest_discover_tests(test_behavior_registry)

# Systems tests (AISystem, MovementSystem, CleanupSystem, DestroySystem, SpawnerSystem)
add_executable(test_systems test_systems.cpp)

target_link_libraries(test_systems PRIVATE
    rtype_game_shared
    rtype_game_server
    engine
    GTest::gtest_main
)

target_include_directories(test_systems PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
)

gtest_discover_tests(test_systems)

# Collision tests
add_executable(test_collision test_collision.cpp)

target_link_libraries(test_collision PRIVATE
    rtype_game_shared
    rtype_game_server
    engine
    GTest::gtest_main
)

target_include_directories(test_collision PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
)

gtest_discover_tests(test_collision)

# Engine base class tests (AGameEngine, ASystem)
add_executable(test_engine test_engine.cpp)

target_link_libraries(test_engine PRIVATE
    engine
    GTest::gtest_main
)

target_include_directories(test_engine PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/lib/engine/src
)

gtest_discover_tests(test_engine)

# ProjectileSystem tests
add_executable(test_projectile_system test_ProjectileSystem.cpp)

target_link_libraries(test_projectile_system PRIVATE
    rtype_game_shared
    engine
    GTest::gtest_main
)

target_include_directories(test_projectile_system PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
)

gtest_discover_tests(test_projectile_system)

# LifetimeSystem tests
add_executable(test_lifetime_system test_LifetimeSystem.cpp)

target_link_libraries(test_lifetime_system PRIVATE
    rtype_game_shared
    engine
    GTest::gtest_main
)

target_include_directories(test_lifetime_system PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
)

gtest_discover_tests(test_lifetime_system)

# QuadTree unit tests
add_executable(test_quadtree test_quadtree.cpp)

target_link_libraries(test_quadtree PRIVATE
    rtype_game_shared
    engine
    GTest::gtest_main
)

target_include_directories(test_quadtree PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/lib/ecs/src
)

gtest_discover_tests(test_quadtree)

# QuadTree integration tests
add_executable(test_quadtree_integration test_quadtree_integration.cpp)

target_link_libraries(test_quadtree_integration PRIVATE
    rtype_game_shared
    engine
    GTest::gtest_main
)

target_include_directories(test_quadtree_integration PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/lib/ecs/src
)

gtest_discover_tests(test_quadtree_integration)

# ProjectileSpawnerSystem tests
add_executable(test_projectile_spawner_system test_ProjectileSpawnerSystem.cpp)

target_link_libraries(test_projectile_spawner_system PRIVATE
    rtype_game_shared
    rtype_game_server
    engine
    GTest::gtest_main
)

target_include_directories(test_projectile_spawner_system PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/lib/ecs/src
)

gtest_discover_tests(test_projectile_spawner_system)

# AABB collision tests
add_executable(test_aabb test_aabb.cpp)

target_link_libraries(test_aabb PRIVATE
    rtype_game_shared
    engine
    GTest::gtest_main
)

target_include_directories(test_aabb PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
)

gtest_discover_tests(test_aabb)
