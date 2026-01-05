add_subdirectory(SceneManager)
add_subdirectory(AssetManager)
add_subdirectory(AudioLib)
add_subdirectory(EntityFactory)

add_library(rtype-graphic-SFML
    STATIC
    Graphic.cpp
    Graphic.hpp
    KeyboardActions.cpp
    KeyboardActions.hpp
    ControllerRumble.cpp
    ControllerRumble.hpp
)

target_include_directories(rtype-graphic-SFML PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

rtype_find_sfml()
rtype_find_sdl2()

target_link_libraries(rtype-graphic-SFML
    PUBLIC
    SFML::Network
    SFML::Graphics
    SFML::Window
    SFML::Audio
    SFML::System
    SDL2::SDL2
    engine
    network
    rtype-graphic-scene-manager
    rtype_game_client_systems
)

target_include_directories(rtype-graphic-SFML
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ../
)

target_include_directories(rtype-graphic-main-menu-scene
    PUBLIC
    ./
)