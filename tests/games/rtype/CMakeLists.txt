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
)

if(WIN32 OR MSVC)
    gtest_discover_tests(test_components WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
else()
    gtest_discover_tests(test_components)
endif()
)

if(WIN32 OR MSVC)
    gtest_discover_tests(test_behaviors WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
else()
    gtest_discover_tests(test_behaviors)
endif()
    engine
    GTest::gtest_main
)
)

if(WIN32 OR MSVC)
    gtest_discover_tests(test_behavior_registry WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
else()
    gtest_discover_tests(test_behavior_registry)
endif()
    ${CMAKE_SOURCE_DIR}/src
)
)

if(WIN32 OR MSVC)
    gtest_discover_tests(test_systems WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
else()
    gtest_discover_tests(test_systems)
endif()
else()
    gtest_discover_tests(test_components)
endif()
)

if(WIN32 OR MSVC)
    gtest_discover_tests(test_wave_system WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
else()
    gtest_discover_tests(test_wave_system)
endif()
)

if(WIN32 OR MSVC)
    gtest_discover_tests(test_collision WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
else()
    gtest_discover_tests(test_collision)
endif()
    engine
    GTest::gtest_main
)
)

if(WIN32 OR MSVC)
    gtest_discover_tests(test_engine WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
else()
    gtest_discover_tests(test_engine)
endif()
    ${CMAKE_SOURCE_DIR}/src
)
)

if(WIN32 OR MSVC)
    gtest_discover_tests(test_projectile_system WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
else()
    gtest_discover_tests(test_projectile_system)
endif()
else()
    gtest_discover_tests(test_behaviors)
endif()
)

if(WIN32 OR MSVC)
    gtest_discover_tests(test_lifetime_system WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
else()
    gtest_discover_tests(test_lifetime_system)
endif()
)

if(WIN32 OR MSVC)
    gtest_discover_tests(test_quadtree WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
else()
    gtest_discover_tests(test_quadtree)
endif()
    engine
    GTest::gtest_main
)
)

if(WIN32 OR MSVC)
    gtest_discover_tests(test_quadtree_integration WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
else()
    gtest_discover_tests(test_quadtree_integration)
endif()
    ${CMAKE_SOURCE_DIR}/src
)
)

if(WIN32 OR MSVC)
    gtest_discover_tests(test_projectile_spawner_system WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
else()
    gtest_discover_tests(test_projectile_spawner_system)
endif()
else()
    gtest_discover_tests(test_behavior_registry)
endif()
)

if(WIN32 OR MSVC)
    gtest_discover_tests(test_aabb WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
else()
    gtest_discover_tests(test_aabb)
endif()
)

if(WIN32 OR MSVC)
    gtest_discover_tests(test_powerup_system WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
else()
    gtest_discover_tests(test_powerup_system)
endif()
    rtype_game_server
    engine
    GTest::gtest_main
)

target_include_directories(test_systems PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
)

if(WIN32 OR MSVC)
    set_tests_properties(test_systems PROPERTIES SKIP_GTEST_DISCOVERY TRUE)
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
    set_tests_properties(test_wave_system PROPERTIES SKIP_GTEST_DISCOVERY TRUE)
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
    set_tests_properties(test_collision PROPERTIES SKIP_GTEST_DISCOVERY TRUE)
else()
    gtest_discover_tests(test_collision)
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
    set_tests_properties(test_engine PROPERTIES SKIP_GTEST_DISCOVERY TRUE)
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
    set_tests_properties(test_projectile_system PROPERTIES SKIP_GTEST_DISCOVERY TRUE)
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
    set_tests_properties(test_lifetime_system PROPERTIES SKIP_GTEST_DISCOVERY TRUE)
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
    set_tests_properties(test_quadtree PROPERTIES SKIP_GTEST_DISCOVERY TRUE)
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
    set_tests_properties(test_quadtree_integration PROPERTIES SKIP_GTEST_DISCOVERY TRUE)
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
    set_tests_properties(test_projectile_spawner_system PROPERTIES SKIP_GTEST_DISCOVERY TRUE)
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
    set_tests_properties(test_aabb PROPERTIES SKIP_GTEST_DISCOVERY TRUE)
else()
    gtest_discover_tests(test_aabb)
endif()

# RTypeEntitySpawner tests - DISABLED due to complex NetworkServer mocking requirements
# These tests require significant refactoring of ServerNetworkSystem to support proper mocking
# add_executable(test_entity_spawner test_entity_spawner.cpp)
# target_link_libraries(test_entity_spawner PRIVATE rtype_game_shared rtype_game_server rtype_server_core rtype_server_network network engine GTest::gtest_main)
# target_include_directories(test_entity_spawner PRIVATE ${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/src)
# gtest_discover_tests(test_entity_spawner)

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
    set_tests_properties(test_powerup_system PROPERTIES SKIP_GTEST_DISCOVERY TRUE)
else()
    gtest_discover_tests(test_powerup_system)
endif()
