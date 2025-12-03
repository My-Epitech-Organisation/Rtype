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

set_target_properties(rtype_game_client_components PROPERTIES LINKER_LANGUAGE CXX)

target_link_libraries(rtype_game_client_components
    PUBLIC
    SFML::Graphics
    SFML::Window
    SFML::Audio
    SFML::System
)
