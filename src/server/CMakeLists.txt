
# Find cpp-httplib for admin server
find_package(httplib CONFIG REQUIRED)

# Library for server core logic (game state, processing, spawning)
add_library(rtype_server_core STATIC
    serverApp/ServerLoop.cpp
    serverApp/ServerApp.cpp
    clientManager/ClientManager.cpp
    serverApp/game/entitySpawnerFactory/EntitySpawnerFactory.cpp
    serverApp/game/gameStateManager/GameStateManager.cpp
    serverApp/game/gameEvent/GameEventProcessor.cpp
    serverApp/game/gameSession/GameSession.cpp
    serverApp/packetProcessor/PacketProcessor.cpp
    serverApp/player/playerInputHandler/PlayerInputHandler.cpp
    serverApp/player/playerSpawner/PlayerSpawner.cpp
    shared/AdminServer.cpp
    shared/BanManager.cpp
    lobby/Lobby.cpp
    lobby/LobbyManager.cpp
    lobby/LobbyDiscoveryServer.cpp
)

target_compile_features(rtype_server_core PRIVATE cxx_std_20)
target_include_directories(rtype_server_core PUBLIC
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/serverApp
    ${CMAKE_CURRENT_SOURCE_DIR}/serverApp/game
    ${CMAKE_CURRENT_SOURCE_DIR}/serverApp/game/entitySpawnerFactory
    ${CMAKE_CURRENT_SOURCE_DIR}/serverApp/game/gameStateManager
    ${CMAKE_CURRENT_SOURCE_DIR}/serverApp/game/gameEvent
    ${CMAKE_CURRENT_SOURCE_DIR}/serverApp/game/gameSession
    ${CMAKE_CURRENT_SOURCE_DIR}/serverApp/packetProcessor
    ${CMAKE_CURRENT_SOURCE_DIR}/serverApp/player
    ${CMAKE_CURRENT_SOURCE_DIR}/serverApp/player/playerInputHandler
    ${CMAKE_CURRENT_SOURCE_DIR}/serverApp/player/playerSpawner
    ${CMAKE_CURRENT_SOURCE_DIR}/shared
)
target_link_libraries(rtype_server_core
    PUBLIC
    engine
    ecs
    common
    network
    rtype_game_server
    rtype_game_shared
)

add_library(rtype_server_network STATIC
    network/NetworkServer.cpp
    network/ServerNetworkSystem.cpp
)

target_compile_features(rtype_server_network PRIVATE cxx_std_20)
target_include_directories(rtype_server_network PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/include
)
target_link_libraries(rtype_server_network
    PRIVATE
    network
    common
    rtype_game_server
    rtype_server_core
)

add_executable(r-type_server
    main.cpp
    clientManager/ClientManager.cpp
)

target_compile_features(r-type_server PRIVATE cxx_std_20)

target_include_directories(r-type_server PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

# Server executable depends on all server-side modules
target_link_libraries(r-type_server PRIVATE
    engine
    network
    rtype_game_shared
    rtype_game_server
    rtype_server_network
    rtype_server_core
    httplib::httplib
)
