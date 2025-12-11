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
)

target_include_directories(rtype-graphic-SFML PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

find_package(SFML
        COMPONENTS
        Network
        Graphics
        Window
        Audio
        System
        CONFIG
        REQUIRED
)

target_link_libraries(rtype-graphic-SFML
    PUBLIC
    SFML::Network
    SFML::Graphics
    SFML::Window
    SFML::Audio
    SFML::System
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