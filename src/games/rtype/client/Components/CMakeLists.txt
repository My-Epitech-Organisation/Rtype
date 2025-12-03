add_library(rtype_game_client_components STATIC
    OBJECT
    ImageComponent.hpp
    RectangleComponent.hpp
    TextComponent.hpp
    ButtonComponent.hpp
    UserEventComponent.hpp
    TagComponent.hpp
    ParallaxComponent.hpp
    TextureRectComponent.hpp
    BoxingComponent.hpp
)

target_link_libraries(rtype_game_client_components
    PUBLIC
    SFML::Graphics
    SFML::Window
    SFML::Audio
    SFML::System
)
