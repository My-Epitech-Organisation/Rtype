cmake_minimum_required(VERSION 3.20)

# Integration tests
add_executable(test_integration
    test_integration.cpp
    test_network_api.cpp
    test_server_integration.cpp
    ${CMAKE_SOURCE_DIR}/src/client/network/NetworkClient.cpp
    ${CMAKE_SOURCE_DIR}/src/server/network/NetworkServer.cpp
    ${CMAKE_SOURCE_DIR}/src/server/network/ServerNetworkSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/server/serverApp/ServerApp.cpp
    ${CMAKE_SOURCE_DIR}/src/server/clientManager/ClientManager.cpp
    test_network_disconnects.cpp
    test_network_packet_loss.cpp
)

target_link_libraries(test_integration
    PRIVATE
    gtest_main
    common
    rtype_toml_parser
    network
    ecs
    engine
    rtype_game_shared
    rtype_game_server
    rtype_client_network
    rtype_server_network
    rtype_server_core
)

target_include_directories(test_integration
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/src/client
        ${CMAKE_SOURCE_DIR}/src/server
        ${CMAKE_SOURCE_DIR}/src/server/serverApp
        ${CMAKE_SOURCE_DIR}/src/server/serverApp/game
        ${CMAKE_SOURCE_DIR}/src/server/serverApp/game/entitySpawnerFactory
        ${CMAKE_SOURCE_DIR}/src/server/serverApp/game/gameEvent
        ${CMAKE_SOURCE_DIR}/src/server/serverApp/game/gameSession
        ${CMAKE_SOURCE_DIR}/src/server/serverApp/packetProcessor
        ${CMAKE_SOURCE_DIR}/src/server/serverApp/player
        ${CMAKE_SOURCE_DIR}/src/server/serverApp/player/playerInputHandler
        ${CMAKE_SOURCE_DIR}/src/server/serverApp/player/playerSpawner
        ${CMAKE_SOURCE_DIR}/src/server/clientManager
        ${CMAKE_SOURCE_DIR}/src/server/shared
        ${CMAKE_SOURCE_DIR}/tests
        ${CMAKE_SOURCE_DIR}/src/network
        ${CMAKE_SOURCE_DIR}/src/common
        ${CMAKE_SOURCE_DIR}/src/games
        ${CMAKE_SOURCE_DIR}/lib/network/src
        ${CMAKE_SOURCE_DIR}/lib/ecs/src
)

if(WIN32 OR MSVC)
    add_test(NAME IntegrationTests COMMAND test_integration WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
else()
    add_test(NAME IntegrationTests COMMAND test_integration)
endif()