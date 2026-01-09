add_library(rtype_game_client_components STATIC
    OBJECT
    ImageComponent.hpp
    RectangleComponent.hpp
    TextComponent.hpp
    TextInputComponent.hpp
    ButtonComponent.hpp
    UserEventComponent.hpp
    TagComponent.hpp
    ParallaxComponent.hpp
    TextureRectComponent.hpp
    BoxingComponent.hpp
    HiddenComponent.hpp
    ZIndexComponent.hpp
    SizeComponent.hpp
)

set_target_properties(rtype_game_client_components PROPERTIES LINKER_LANGUAGE CXX)

target_link_libraries(rtype_game_client_components
    PUBLIC
)
