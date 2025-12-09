add_executable(test_spawning test_spawning.cpp)

target_link_libraries(test_spawning PRIVATE
    rtype_game_shared
    rtype_game_server
    rtype_engine
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
    rtype_engine
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
    rtype_engine
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
    rtype_engine
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
    rtype_engine
    GTest::gtest_main
)

target_include_directories(test_systems PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
)

gtest_discover_tests(test_systems)

# Engine base class tests (AGameEngine, ASystem)
add_executable(test_engine test_engine.cpp)

target_link_libraries(test_engine PRIVATE
    rtype_engine
    GTest::gtest_main
)

target_include_directories(test_engine PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/lib/rtype_engine/src
)

gtest_discover_tests(test_engine)

# ProjectileSystem tests
add_executable(test_projectile_system test_ProjectileSystem.cpp)

target_link_libraries(test_projectile_system PRIVATE
    rtype_game_shared
    rtype_engine
    GTest::gtest_main
)

target_include_directories(test_projectile_system PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
)

gtest_discover_tests(test_projectile_system)
