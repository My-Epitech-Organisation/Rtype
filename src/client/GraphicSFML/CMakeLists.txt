add_subdirectory(SFML)

add_library(rtype_graphic_client_sfml STATIC
    Graphic.cpp
)
target_link_libraries(rtype_graphic_client_sfml
    sfml-system
    sfml-window
    sfml-graphics
    sfml-audio
    sfml-network)

set_target_properties(rtype_graphic_client_sfml PROPERTIES CXX_STANDARD 20 CXX_STANDARD_REQUIRED YES)