add_library(rtype-graphic-stress-test-scene
    STATIC
    StressTestScene.cpp
    StressTestScene.hpp
)

target_link_libraries(rtype-graphic-stress-test-scene
    PUBLIC
    SFML::Graphics
    SFML::Window
    SFML::Audio
    SFML::System
    rtype_engine
)

target_link_libraries(rtype-graphic-stress-test-scene
    PRIVATE
    rtype-graphic-asset-manager
    rtype-graphic-entity-factory
    rtype_game_client
    rtype_game_shared
)
