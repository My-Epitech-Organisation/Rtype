add_library(rtype-graphic-components
    INTERFACE
    ImageComponent.hpp
    RectangleComponent.hpp
    TextComponent.hpp
    ButtonComponent.hpp
    UserEventComponent.hpp
        TagComponent.hpp
)

target_link_libraries(rtype-graphic-scene-manager
    PUBLIC
    SFML::Graphics
    SFML::Window
    SFML::Audio
    SFML::System
)
