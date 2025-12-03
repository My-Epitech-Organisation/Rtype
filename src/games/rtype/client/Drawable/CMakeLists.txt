add_library(rtype_game_client_drawables STATIC
    OBJECT
    Image.hpp
)

set_target_properties(rtype_game_client_drawables PROPERTIES LINKER_LANGUAGE CXX)

target_link_libraries(rtype_game_client_drawables
    PUBLIC
    SFML::Graphics
    SFML::Window
    SFML::Audio
    SFML::System
    rtype_game_client_components
)
