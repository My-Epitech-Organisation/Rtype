add_library(rtype-graphic-game-over-scene
    STATIC
    GameOverScene.cpp
    GameOverScene.hpp
)

target_link_libraries(rtype-graphic-game-over-scene
    PUBLIC
    engine
    network
    rtype-graphic-audio
)

target_link_libraries(rtype-graphic-game-over-scene
    PRIVATE
    rtype-graphic-asset-manager
    rtype-graphic-entity-factory
    rtype_game_client
    rtype_game_shared
)
