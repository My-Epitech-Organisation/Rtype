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
if(WIN32 OR MSVC)
    gtest_discover_tests(test_spawning WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
    gtest_discover_tests(test_movement_system WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
else()
    gtest_discover_tests(test_spawning)
    gtest_discover_tests(test_movement_system)
endif()

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

if(WIN32 OR MSVC)
    gtest_discover_tests(test_components WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
else()
    gtest_discover_tests(test_components)
endif()

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

if(WIN32 OR MSVC)
    gtest_discover_tests(test_behaviors WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
else()
    gtest_discover_tests(test_behaviors)
endif()

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

if(WIN32 OR MSVC)
    gtest_discover_tests(test_behavior_registry WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
else()
    gtest_discover_tests(test_behavior_registry)
endif()

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

if(WIN32 OR MSVC)
    gtest_discover_tests(test_systems WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
else()
    gtest_discover_tests(test_systems)
endif()

# Wave System tests (wave-based spawning and GameOver logic)
add_executable(test_wave_system test_wave_system.cpp)

target_link_libraries(test_wave_system PRIVATE
    rtype_game_shared
    rtype_game_server
    engine
    GTest::gtest_main
)

target_include_directories(test_wave_system PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
)

if(WIN32 OR MSVC)
    gtest_discover_tests(test_wave_system WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
else()
    gtest_discover_tests(test_wave_system)
endif()

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

if(WIN32 OR MSVC)
    gtest_discover_tests(test_collision WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
else()
    gtest_discover_tests(test_collision)
endif()

# Extra collision tests (covers pickups and obstacles)
add_executable(test_collision_extra test_collision_extra.cpp)

target_link_libraries(test_collision_extra PRIVATE
    rtype_game_shared
    rtype_game_server
    engine
    GTest::gtest_main
)

target_include_directories(test_collision_extra PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
)

if(WIN32 OR MSVC)
    gtest_discover_tests(test_collision_extra WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
else()
    gtest_discover_tests(test_collision_extra)
endif()

# Collision branches tests
add_executable(test_collision_branches test_collision_branches.cpp)

target_link_libraries(test_collision_branches PRIVATE
    rtype_game_shared
    rtype_game_server
    engine
    GTest::gtest_main
)

target_include_directories(test_collision_branches PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
)

if(WIN32 OR MSVC)
    gtest_discover_tests(test_collision_branches WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
else()
    gtest_discover_tests(test_collision_branches)
endif()

# EntityConfig branch coverage tests
add_executable(test_entityconfig_branches test_entityconfig_branches.cpp)

target_link_libraries(test_entityconfig_branches PRIVATE
    rtype_game_shared
    GTest::gtest_main
)

target_include_directories(test_entityconfig_branches PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
)

if(WIN32 OR MSVC)
    gtest_discover_tests(test_entityconfig_branches WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
else()
    gtest_discover_tests(test_entityconfig_branches)
endif()

# PrefabLoader tests
add_executable(test_prefab_loader test_prefab_loader.cpp)

target_link_libraries(test_prefab_loader PRIVATE
    rtype_game_shared
    engine
    GTest::gtest_main
)

target_include_directories(test_prefab_loader PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
)

if(WIN32 OR MSVC)
    gtest_discover_tests(test_prefab_loader WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
else()
    gtest_discover_tests(test_prefab_loader)
endif()

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

if(WIN32 OR MSVC)
    gtest_discover_tests(test_engine WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
else()
    gtest_discover_tests(test_engine)
endif()

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

if(WIN32 OR MSVC)
    gtest_discover_tests(test_projectile_system WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
else()
    gtest_discover_tests(test_projectile_system)
endif()

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

if(WIN32 OR MSVC)
    gtest_discover_tests(test_lifetime_system WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
else()
    gtest_discover_tests(test_lifetime_system)
endif()

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

if(WIN32 OR MSVC)
    gtest_discover_tests(test_quadtree WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
else()
    gtest_discover_tests(test_quadtree)
endif()

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

if(WIN32 OR MSVC)
    gtest_discover_tests(test_quadtree_integration WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
else()
    gtest_discover_tests(test_quadtree_integration)
endif()

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

if(WIN32 OR MSVC)
    gtest_discover_tests(test_projectile_spawner_system WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
else()
    gtest_discover_tests(test_projectile_spawner_system)
endif()

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

if(WIN32 OR MSVC)
    gtest_discover_tests(test_aabb WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
else()
    gtest_discover_tests(test_aabb)
endif()

# RTypeEntitySpawner tests
# These tests were previously disabled due to complex NetworkServer mocking requirements.
# The tests provide an in-test mock (MockServerNetworkSystem) so enabling the target is safe.
add_executable(test_entity_spawner test_entity_spawner.cpp)

target_link_libraries(test_entity_spawner PRIVATE
    rtype_game_shared
    rtype_game_server
    rtype_server_core
    rtype_server_network
    network
    engine
    GTest::gtest_main
)

target_include_directories(test_entity_spawner PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
)

if(WIN32 OR MSVC)
    gtest_discover_tests(test_entity_spawner WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
else()
    gtest_discover_tests(test_entity_spawner)
endif()

# PowerUpSystem tests
add_executable(test_powerup_system test_powerup_system.cpp)

target_link_libraries(test_powerup_system PRIVATE
    rtype_game_shared
    engine
    GTest::gtest_main
)

target_include_directories(test_powerup_system PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
)

if(WIN32 OR MSVC)
    gtest_discover_tests(test_powerup_system WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
else()
    gtest_discover_tests(test_powerup_system)
endif()

# EnemyHealthBarSystem tests
add_executable(test_enemy_health_bar_system client/test_enemy_health_bar_system.cpp)

target_link_libraries(test_enemy_health_bar_system PRIVATE
    rtype_game_client_systems
    rtype_game_shared
    engine
    GTest::gtest_main
)

target_include_directories(test_enemy_health_bar_system PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/games/rtype/client
    ${CMAKE_SOURCE_DIR}/src/games/rtype/shared
)

if(WIN32 OR MSVC)
    gtest_discover_tests(test_enemy_health_bar_system WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
else()
    gtest_discover_tests(test_enemy_health_bar_system)
endif()

# FileOperations tests (from shared/config/save_manager subdirectory)
add_executable(test_file_operations shared/config/save_manager/test_file_operations.cpp)

target_link_libraries(test_file_operations PRIVATE
    GTest::gtest_main
    rtype_game_config
)

target_include_directories(test_file_operations PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/games/rtype/shared
)

if(WIN32 OR MSVC)
    gtest_discover_tests(test_file_operations WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
else()
    gtest_discover_tests(test_file_operations)
endif()

# ForcePodLaunchSystem tests
add_executable(test_forcepod_launch_system test_forcepod_launch_system.cpp)

target_link_libraries(test_forcepod_launch_system PRIVATE
    rtype_game_shared
    rtype_game_server
    engine
    GTest::gtest_main
)

target_include_directories(test_forcepod_launch_system PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
)

if(WIN32 OR MSVC)
    gtest_discover_tests(test_forcepod_launch_system WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
else()
    gtest_discover_tests(test_forcepod_launch_system)
endif()

# ForcePodShootingSystem tests
add_executable(test_forcepod_shooting_system test_forcepod_shooting_system.cpp)

target_link_libraries(test_forcepod_shooting_system PRIVATE
    rtype_game_shared
    rtype_game_server
    engine
    GTest::gtest_main
)

target_include_directories(test_forcepod_shooting_system PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
)

if(WIN32 OR MSVC)
    gtest_discover_tests(test_forcepod_shooting_system WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
else()
    gtest_discover_tests(test_forcepod_shooting_system)
endif()

# PowerUpTypeComponent tests
add_executable(test_powerup_type_component test_powerup_type_component.cpp)

target_link_libraries(test_powerup_type_component PRIVATE
    rtype_game_shared
    GTest::gtest_main
)

target_include_directories(test_powerup_type_component PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
)

if(WIN32 OR MSVC)
    gtest_discover_tests(test_powerup_type_component WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
else()
    gtest_discover_tests(test_powerup_type_component)
endif()
