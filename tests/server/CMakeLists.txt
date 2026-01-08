add_executable(test_client_manager test_ClientManager.cpp
    ${CMAKE_SOURCE_DIR}/src/server/clientManager/ClientManager.cpp
)

target_include_directories(test_client_manager PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/server
    ${CMAKE_SOURCE_DIR}/src/server/clientManager
    ${CMAKE_SOURCE_DIR}/src/server/shared
    ${CMAKE_SOURCE_DIR}/src/common
    ${CMAKE_SOURCE_DIR}/include
)

target_compile_features(test_client_manager PRIVATE cxx_std_20)

target_link_libraries(test_client_manager PRIVATE
    GTest::gtest_main
    common
    rtype_toml_parser
    rtype_server_core
)

set(SERVER_APP_TEST_SRCS test_server_app.cpp)
list(APPEND SERVER_APP_TEST_SRCS test_server_app_extra.cpp)
if(NOT WIN32)
    list(APPEND SERVER_APP_TEST_SRCS test_server_app_private.cpp)
endif()

add_executable(test_server_app ${SERVER_APP_TEST_SRCS}
    ${CMAKE_SOURCE_DIR}/src/server/serverApp/ServerApp.cpp
    ${CMAKE_SOURCE_DIR}/src/server/clientManager/ClientManager.cpp
    ${CMAKE_SOURCE_DIR}/src/server/network/NetworkServer.cpp
    ${CMAKE_SOURCE_DIR}/src/server/network/ServerNetworkSystem.cpp
)

target_include_directories(test_server_app PRIVATE
    ${CMAKE_SOURCE_DIR}/src
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
    ${CMAKE_SOURCE_DIR}/src/common
    ${CMAKE_SOURCE_DIR}/include
)

target_compile_features(test_server_app PRIVATE cxx_std_20)

target_link_libraries(test_server_app PRIVATE
    GTest::gtest_main
    common
    rtype_toml_parser
    network
    engine
    rtype_game_server
    rtype_game_shared
    rtype_server_core
)

# ServerApp GameConfig integration tests
add_executable(test_server_app_gameconfig test_server_app_gameconfig.cpp
    test_server_app_reload_configuration.cpp
    ${CMAKE_SOURCE_DIR}/src/server/serverApp/ServerApp.cpp
    ${CMAKE_SOURCE_DIR}/src/server/clientManager/ClientManager.cpp
    ${CMAKE_SOURCE_DIR}/src/server/network/NetworkServer.cpp
    ${CMAKE_SOURCE_DIR}/src/server/network/ServerNetworkSystem.cpp
)

target_include_directories(test_server_app_gameconfig PRIVATE
    ${CMAKE_SOURCE_DIR}/src
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
    ${CMAKE_SOURCE_DIR}/src/common
)

target_compile_features(test_server_app_gameconfig PRIVATE cxx_std_20)

target_link_libraries(test_server_app_gameconfig PRIVATE
    GTest::gtest_main
    common
    rtype_toml_parser
    network
    engine
    rtype_game_server
    rtype_game_shared
    rtype_server_core
)

# ServerNetworkSystem unit tests
add_executable(test_server_network_system test_ServerNetworkSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/server/network/ServerNetworkSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/server/network/NetworkServer.cpp
)

target_include_directories(test_server_network_system PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/server
    ${CMAKE_SOURCE_DIR}/src/server/shared
    ${CMAKE_SOURCE_DIR}/src/common
    ${CMAKE_SOURCE_DIR}/include
)

target_compile_features(test_server_network_system PRIVATE cxx_std_20)

target_link_libraries(test_server_network_system PRIVATE
    GTest::gtest_main
    common
    rtype_toml_parser
    network
    ecs
    rtype_game_shared
    rtype_server_core
)

# Unit tests for ServerApp
add_executable(test_server_app_unit
    ${CMAKE_CURRENT_SOURCE_DIR}/test_server_app_unit.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test_server_app_initialize.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test_server_app_initialize_additional.cpp
    ${CMAKE_SOURCE_DIR}/src/server/serverApp/ServerApp.cpp
    ${CMAKE_SOURCE_DIR}/src/server/clientManager/ClientManager.cpp
    ${CMAKE_SOURCE_DIR}/src/server/network/NetworkServer.cpp
    ${CMAKE_SOURCE_DIR}/src/server/network/ServerNetworkSystem.cpp
)

target_include_directories(test_server_app_unit PRIVATE
    ${CMAKE_SOURCE_DIR}/src
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
    ${CMAKE_SOURCE_DIR}/src/common
    ${CMAKE_SOURCE_DIR}/include
)

target_compile_features(test_server_app_unit PRIVATE cxx_std_20)

target_link_libraries(test_server_app_unit PRIVATE
    GTest::gtest_main
    common
    rtype_toml_parser
    network
    engine
    rtype_game_server
    rtype_game_shared
    rtype_server_core
)

add_executable(test_server_network_messages test_server_network_messages.cpp)

target_include_directories(test_server_network_messages PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/server
    ${CMAKE_SOURCE_DIR}/src/common
)

target_compile_features(test_server_network_messages PRIVATE cxx_std_20)

target_link_libraries(test_server_network_messages PRIVATE
    GTest::gtest_main
    common
    rtype_toml_parser
    network
    rtype_client_network
    rtype_server_network
)

# ServerLoop unit tests
add_executable(test_server_loop test_ServerLoop.cpp)

target_include_directories(test_server_loop PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/server
    ${CMAKE_SOURCE_DIR}/src/server/serverApp
    ${CMAKE_SOURCE_DIR}/include
)

target_compile_features(test_server_loop PRIVATE cxx_std_20)

target_link_libraries(test_server_loop PRIVATE
    GTest::gtest_main
    rtype_server_core
    common
)

# PlayerInputHandler unit tests
add_executable(test_player_input_handler test_player_input_handler.cpp
    ${CMAKE_SOURCE_DIR}/src/server/serverApp/player/playerInputHandler/PlayerInputHandler.cpp
    ${CMAKE_SOURCE_DIR}/src/server/serverApp/game/gameStateManager/GameStateManager.cpp
    ${CMAKE_SOURCE_DIR}/src/server/network/ServerNetworkSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/server/network/NetworkServer.cpp
)

target_include_directories(test_player_input_handler PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/server
    ${CMAKE_SOURCE_DIR}/src/server/serverApp
    ${CMAKE_SOURCE_DIR}/src/server/serverApp/game
    ${CMAKE_SOURCE_DIR}/src/server/serverApp/game/gameStateManager
    ${CMAKE_SOURCE_DIR}/src/server/serverApp/player
    ${CMAKE_SOURCE_DIR}/src/server/serverApp/player/playerInputHandler
    ${CMAKE_SOURCE_DIR}/src/server/shared
    ${CMAKE_SOURCE_DIR}/include
)

target_compile_features(test_player_input_handler PRIVATE cxx_std_20)

target_link_libraries(test_player_input_handler PRIVATE
    GTest::gtest_main
    common
    network
    ecs
    rtype_game_shared
    rtype_server_core
)

# PlayerSpawner unit tests
add_executable(test_player_spawner test_player_spawner.cpp
    ${CMAKE_SOURCE_DIR}/src/server/serverApp/player/playerSpawner/PlayerSpawner.cpp
    ${CMAKE_SOURCE_DIR}/src/server/network/ServerNetworkSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/server/network/NetworkServer.cpp
)

target_include_directories(test_player_spawner PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/server
    ${CMAKE_SOURCE_DIR}/src/server/serverApp
    ${CMAKE_SOURCE_DIR}/src/server/serverApp/player
    ${CMAKE_SOURCE_DIR}/src/server/serverApp/player/playerSpawner
    ${CMAKE_SOURCE_DIR}/src/server/shared
    ${CMAKE_SOURCE_DIR}/include
)

target_compile_features(test_player_spawner PRIVATE cxx_std_20)

target_link_libraries(test_player_spawner PRIVATE
    GTest::gtest_main
    common
    network
    ecs
    rtype_game_shared
    rtype_server_core
)

# GameEventProcessor unit tests
add_executable(test_game_event_processor test_game_event_processor.cpp
    ${CMAKE_SOURCE_DIR}/src/server/serverApp/game/gameEvent/GameEventProcessor.cpp
    ${CMAKE_SOURCE_DIR}/src/server/network/ServerNetworkSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/server/network/NetworkServer.cpp
)

# BanManager unit tests
add_executable(test_ban_manager test_ban_manager.cpp)

target_include_directories(test_ban_manager PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/server/shared
)

target_link_libraries(test_ban_manager PRIVATE
    GTest::gtest_main
    common
    rtype_server_core
)

# ServerMetrics unit tests
add_executable(test_server_metrics test_server_metrics.cpp)

target_include_directories(test_server_metrics PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/server/shared
)

target_link_libraries(test_server_metrics PRIVATE
    GTest::gtest_main
    common
)

# LobbyManager unit tests
add_executable(test_lobby_manager test_lobby_manager.cpp)

target_include_directories(test_lobby_manager PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/server
    ${CMAKE_SOURCE_DIR}/src/server/shared
)

target_link_libraries(test_lobby_manager PRIVATE
    GTest::gtest_main
    common
    rtype_server_core
    network
    rtype_server_network
    engine
    ecs
    rtype_game_shared
    rtype_game_server
)

target_include_directories(test_game_event_processor PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/server
    ${CMAKE_SOURCE_DIR}/src/server/serverApp
    ${CMAKE_SOURCE_DIR}/src/server/serverApp/game
    ${CMAKE_SOURCE_DIR}/src/server/serverApp/game/gameEvent
    ${CMAKE_SOURCE_DIR}/src/server/shared
    ${CMAKE_SOURCE_DIR}/include
)

target_compile_features(test_game_event_processor PRIVATE cxx_std_20)

target_link_libraries(test_game_event_processor PRIVATE
    GTest::gtest_main
    common
    network
    ecs
    engine
    rtype_game_shared
    rtype_server_core
)

# PacketProcessor unit tests
add_executable(test_packet_processor test_packet_processor.cpp
    ${CMAKE_SOURCE_DIR}/src/server/serverApp/packetProcessor/PacketProcessor.cpp
)

target_include_directories(test_packet_processor PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/server
    ${CMAKE_SOURCE_DIR}/src/server/serverApp
    ${CMAKE_SOURCE_DIR}/src/server/serverApp/packetProcessor
    ${CMAKE_SOURCE_DIR}/src/server/shared
    ${CMAKE_SOURCE_DIR}/include
)

target_compile_features(test_packet_processor PRIVATE cxx_std_20)

target_link_libraries(test_packet_processor PRIVATE
    GTest::gtest_main
    common
    network
)

# Lobby discovery tests
add_executable(test_lobby_discovery test_lobby_discovery.cpp)

target_include_directories(test_lobby_discovery PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/server
    ${CMAKE_SOURCE_DIR}/src/server/lobby
    ${CMAKE_SOURCE_DIR}/src/common
)

target_compile_features(test_lobby_discovery PRIVATE cxx_std_20)

target_link_libraries(test_lobby_discovery PRIVATE
    GTest::gtest_main
    common
    rtype_toml_parser
    network
    engine
    rtype_game_server
    rtype_game_shared
    rtype_server_core
    rtype_server_network
    ecs
)

# GameStateManager unit tests
add_executable(test_game_state_manager test_game_state_manager.cpp test_game_state_manager_countdown.cpp
    ${CMAKE_SOURCE_DIR}/src/server/serverApp/game/gameStateManager/GameStateManager.cpp
)

# AdminServer tests
add_executable(test_admin_server test_admin_server.cpp test_admin_server_branches.cpp test_admin_server_more_branches.cpp test_admin_server_auth_metrics.cpp test_admin_server_metrics_players.cpp test_admin_server_connected_client.cpp test_admin_server_lobby_api.cpp test_admin_server_malformed_json.cpp)


target_include_directories(test_admin_server PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/server
    ${CMAKE_SOURCE_DIR}/src/server/shared
    ${CMAKE_SOURCE_DIR}/include
)

target_compile_features(test_admin_server PRIVATE cxx_std_20)

target_link_libraries(test_admin_server PRIVATE
    GTest::gtest_main
    common
    rtype_server_core
    rtype_server_network
    network
    engine
    rtype_game_server
    rtype_game_shared
    ecs
)

target_include_directories(test_game_state_manager PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/server
    ${CMAKE_SOURCE_DIR}/src/server/serverApp
    ${CMAKE_SOURCE_DIR}/src/server/serverApp/game
    ${CMAKE_SOURCE_DIR}/src/server/serverApp/game/gameStateManager
    ${CMAKE_SOURCE_DIR}/src/server/shared
    ${CMAKE_SOURCE_DIR}/include
)

target_compile_features(test_game_state_manager PRIVATE cxx_std_20)

target_link_libraries(test_game_state_manager PRIVATE
    GTest::gtest_main
    common
    rtype_toml_parser
)

include(GoogleTest)
if(WIN32)
    gtest_discover_tests(test_server_network_messages WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
    gtest_discover_tests(test_client_manager WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
    gtest_discover_tests(test_server_app WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
    gtest_discover_tests(test_server_app_gameconfig WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
    gtest_discover_tests(test_server_network_system WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
    gtest_discover_tests(test_server_app_unit WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
    gtest_discover_tests(test_server_loop WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
    gtest_discover_tests(test_player_input_handler WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
    gtest_discover_tests(test_player_spawner WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
    gtest_discover_tests(test_game_event_processor WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
    gtest_discover_tests(test_packet_processor WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
    gtest_discover_tests(test_game_state_manager WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
    gtest_discover_tests(test_ban_manager WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
    gtest_discover_tests(test_lobby_manager WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
    gtest_discover_tests(test_lobby_discovery WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
    gtest_discover_tests(test_admin_server WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
else()
    gtest_discover_tests(test_server_network_messages)
    gtest_discover_tests(test_client_manager)
    gtest_discover_tests(test_server_app)
    gtest_discover_tests(test_server_app_gameconfig)
    gtest_discover_tests(test_server_network_system)
    gtest_discover_tests(test_server_app_unit)
    gtest_discover_tests(test_server_loop)
    gtest_discover_tests(test_player_input_handler)
    gtest_discover_tests(test_player_spawner)
    gtest_discover_tests(test_game_event_processor)
    gtest_discover_tests(test_packet_processor)
    gtest_discover_tests(test_game_state_manager)
    gtest_discover_tests(test_ban_manager)
    gtest_discover_tests(test_lobby_manager)
    gtest_discover_tests(test_lobby_discovery)
    gtest_discover_tests(test_admin_server)
endif()
