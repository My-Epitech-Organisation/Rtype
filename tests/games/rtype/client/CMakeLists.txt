# Client component tests

# Boss visual components tests (BossSerpentComponent, BossVisualComponent)
add_executable(test_boss_visual_components test_boss_visual_components.cpp)

target_link_libraries(test_boss_visual_components PRIVATE
    rtype_game_client_components
    rtype_game_shared_components
    GTest::gtest_main
)

target_include_directories(test_boss_visual_components PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
)

include(GoogleTest)
if(WIN32 OR MSVC)
    gtest_discover_tests(test_boss_visual_components WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
else()
    gtest_discover_tests(test_boss_visual_components)
endif()
