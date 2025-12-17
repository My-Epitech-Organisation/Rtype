add_subdirectory(Scenes)

add_library(rtype-graphic-scene-manager
    STATIC
    SceneManager.cpp
    SceneManager.hpp
    SceneException.hpp
)

target_include_directories(rtype-graphic-scene-manager PRIVATE
    ${CMAKE_SOURCE_DIR}/src/games/rtype/client
)

target_link_libraries(rtype-graphic-scene-manager
    PUBLIC
    SFML::Graphics
    SFML::Window
    SFML::Audio
    SFML::System
    engine
    network
    rtype-graphic-main-menu-scene
    rtype-graphic-settings-scene
    rtype-graphic-game-scene
    rtype-graphic-how-to-play-scene
    rtype-graphic-lobby-scene
    rtype-graphic-game-over-scene
    rtype_game_client
)
