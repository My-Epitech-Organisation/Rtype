add_executable(test_spawning test_spawning.cpp)

target_link_libraries(test_spawning PRIVATE
    rtype_game_shared
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(test_spawning)

add_executable(test_movement_system test_movement_system.cpp)

target_link_libraries(test_movement_system PRIVATE
    rtype_game_shared
    GTest::gtest_main
)

gtest_discover_tests(test_movement_system)
