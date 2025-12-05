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

# SaveManager functional tests
add_executable(test_rtype_savemanager
    ${CMAKE_SOURCE_DIR}/tests/savemanager_functional_test.cpp
)

target_link_libraries(test_rtype_savemanager PRIVATE
    rtype_game_shared
    rtype_engine
)

target_include_directories(test_rtype_savemanager PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
)

add_test(NAME test_rtype_savemanager COMMAND test_rtype_savemanager)

gtest_discover_tests(test_systems)

