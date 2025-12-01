add_subdirectory(Scenes)

add_library(rtype-graphic-scene-manager
    STATIC
    SceneManager.cpp
    SceneManager.hpp
    SceneException.hpp
)

target_link_libraries(rtype-graphic-scene-manager
    PUBLIC
    SFML::Graphics
    SFML::Window
    SFML::Audio
    SFML::System
    rtype_engine
    rtype-graphic-main-menu-scene
)