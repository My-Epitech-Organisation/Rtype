add_library(rtype-graphic-lobby-scene
    STATIC
    Lobby.cpp
    Lobby.hpp
)

target_link_libraries(rtype-graphic-lobby-scene
    PUBLIC
    SFML::Graphics
    SFML::Window
    SFML::Audio
    SFML::System
    engine
    network
    rtype-graphic-audio
    rtype_client_network
)

target_link_libraries(rtype-graphic-lobby-scene
    PRIVATE
    rtype-graphic-asset-manager
    rtype-graphic-entity-factory
    rtype_game_client
    rtype_game_shared
)
