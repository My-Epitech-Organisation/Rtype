add_executable(test_spawning test_spawning.cpp)

target_link_libraries(test_spawning PRIVATE
    rtype_game_shared
    GTest::gtest_main
)

target_include_directories(test_spawning PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

include(GoogleTest)
gtest_discover_tests(test_spawning)
