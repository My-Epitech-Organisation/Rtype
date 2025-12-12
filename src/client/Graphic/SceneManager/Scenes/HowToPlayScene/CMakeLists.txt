add_library(rtype-graphic-how-to-play-scene
    STATIC
    HowToPlayScene.cpp
    HowToPlayScene.hpp
)

target_link_libraries(rtype-graphic-how-to-play-scene
    PUBLIC
    SFML::Graphics
    SFML::Window
    SFML::Audio
    SFML::System
    engine
    network
    rtype-graphic-asset-manager
    rtype-graphic-entity-factory
    rtype-graphic-audio
)

target_link_libraries(rtype-graphic-how-to-play-scene
    PRIVATE
    rtype_game_client
    rtype_game_shared
)

target_include_directories(rtype-graphic-how-to-play-scene PUBLIC
    ${CMAKE_SOURCE_DIR}/src/client
    ${CMAKE_SOURCE_DIR}/src/client/Graphic
    ${CMAKE_SOURCE_DIR}/src/client/Graphic/SceneManager
    ${CMAKE_SOURCE_DIR}/src/games/rtype/client
)

target_compile_features(rtype-graphic-how-to-play-scene PUBLIC cxx_std_20)
