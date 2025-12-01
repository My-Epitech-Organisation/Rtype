add_subdirectory(SceneManager)
add_subdirectory(AssetManager)

add_library(rtype-graphic-SFML
    STATIC
    Graphic.cpp
    Graphic.hpp
    KeyboardActions.cpp
    KeyboardActions.hpp
)

target_link_libraries(rtype-graphic-SFML
        PUBLIC
        SFML::Network
        SFML::Graphics
        SFML::Window
        SFML::Audio
        SFML::System
        rtype_engine
        rtype-graphic-scene-manager
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