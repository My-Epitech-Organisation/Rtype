add_library(rtype-graphic-main-menu-scene
    STATIC
    MainMenuScene.cpp
    MainMenuScene.hpp
)

target_link_libraries(rtype-graphic-main-menu-scene
    PUBLIC
    SFML::Graphics
    SFML::Window
    SFML::Audio
    SFML::System
    rtype_engine
)

target_link_libraries(rtype-graphic-main-menu-scene
    PRIVATE
    rtype-graphic-components
    rtype-common-components
    rtype-graphic-asset-manager
    rtype-graphic-entity-factory
)
