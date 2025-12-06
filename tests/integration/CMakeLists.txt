cmake_minimum_required(VERSION 3.20)

# Integration tests
add_executable(test_integration test_integration.cpp)

target_link_libraries(test_integration
    PRIVATE
        gtest_main
        rtype_common
        rtype_toml_parser
        rtype_network
        rtype_game_shared
)

target_include_directories(test_integration
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/tests
        ${CMAKE_SOURCE_DIR}/src/network
        ${CMAKE_SOURCE_DIR}/src/common
        ${CMAKE_SOURCE_DIR}/src/games
)

add_test(NAME IntegrationTests COMMAND test_integration)