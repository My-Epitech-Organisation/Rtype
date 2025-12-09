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
    rtype_engine
    rtype_network
    rtype-graphic-main-menu-scene
    rtype-graphic-settings-scene
    rtype-graphic-game-scene
    rtype_game_client
)
