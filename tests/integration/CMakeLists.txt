cmake_minimum_required(VERSION 3.20)

# Integration tests
add_executable(test_integration test_integration.cpp
    ${CMAKE_SOURCE_DIR}/src/server/ServerApp.cpp
    ${CMAKE_SOURCE_DIR}/src/server/ClientManager.cpp
)

target_link_libraries(test_integration
    PRIVATE
        gtest_main
        rtype_common
        rtype_network
        rtype_game_shared
        rtype_engine
)

target_include_directories(test_integration
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/tests
        ${CMAKE_SOURCE_DIR}/src/network
        ${CMAKE_SOURCE_DIR}/src/common
        ${CMAKE_SOURCE_DIR}/src/games
        ${CMAKE_SOURCE_DIR}/src/server
        ${CMAKE_SOURCE_DIR}/src/engine
)

add_test(NAME IntegrationTests COMMAND test_integration)