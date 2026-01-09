add_library(rtype-graphic-main-menu-scene
    STATIC
    MainMenuScene.cpp
    MainMenuScene.hpp
)

target_link_libraries(rtype-graphic-main-menu-scene
    PUBLIC
    engine
    network
    rtype-graphic-audio
    rtype_client_network
)

target_link_libraries(rtype-graphic-main-menu-scene
    PRIVATE
    rtype-graphic-asset-manager
    rtype-graphic-entity-factory
    rtype_game_client
    rtype_game_shared
)
