add_library(rtype-graphic-game-scene
        STATIC
        GameScene.cpp
        GameScene.hpp
        IGameScene.hpp
        AGameScene.hpp
)

target_link_libraries(rtype-graphic-game-scene
        PUBLIC
        engine
        network
        rtype_client_network
)

target_link_libraries(rtype-graphic-game-scene
        PRIVATE
        rtype-graphic-asset-manager
        rtype-graphic-entity-factory
        rtype_game_client
        rtype_game_shared
)
