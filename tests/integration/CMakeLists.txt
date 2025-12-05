cmake_minimum_required(VERSION 3.20)

# Integration tests
add_executable(test_integration
    test_integration.cpp
    test_network_api.cpp
    ${CMAKE_SOURCE_DIR}/src/client/network/NetworkClient.cpp
    ${CMAKE_SOURCE_DIR}/src/server/network/NetworkServer.cpp
)

target_link_libraries(test_integration
    PRIVATE
        gtest_main
        rtype_common
        rtype_network
        rtype_ecs
        rtype_game_shared
)

target_include_directories(test_integration
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/src/client
        ${CMAKE_SOURCE_DIR}/src/server
        ${CMAKE_SOURCE_DIR}/tests
        ${CMAKE_SOURCE_DIR}/src/network
        ${CMAKE_SOURCE_DIR}/src/common
        ${CMAKE_SOURCE_DIR}/src/games
        ${CMAKE_SOURCE_DIR}/lib/rtype_network/src
        ${CMAKE_SOURCE_DIR}/lib/rtype_ecs/src
)

add_test(NAME IntegrationTests COMMAND test_integration)