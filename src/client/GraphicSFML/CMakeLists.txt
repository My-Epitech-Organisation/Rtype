add_subdirectory(SFML)

add_library(rtype_graphic_client STATIC
    Graphic.cpp
)
target_link_libraries(rtype_graphic_client
    sfml-system
    sfml-window
    sfml-graphics
    sfml-audio
    sfml-network)

set_target_properties(rtype_graphic_client PROPERTIES CXX_STANDARD 20 CXX_STANDARD_REQUIRED YES)