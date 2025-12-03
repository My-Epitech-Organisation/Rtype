add_library(rtype-graphic-settings-scene
    STATIC
    SettingsScene.cpp
    SettingsScene.hpp
    SettingsSceneUtils.cpp
    SettingsSceneUtils.hpp
)

target_link_libraries(rtype-graphic-settings-scene
    PUBLIC
    SFML::Graphics
    SFML::Window
    SFML::Audio
    SFML::System
    rtype_engine
)

target_link_libraries(rtype-graphic-settings-scene
    PRIVATE
    rtype-graphic-components
    rtype-common-components
    rtype-graphic-asset-manager
    rtype-graphic-entity-factory
)