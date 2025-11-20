add_subdirectory(SFML)

add_library(graphic STATIC
    Graphic.cpp
)
target_link_libraries(graphic
    sfml-system
    sfml-window
    sfml-graphics
    sfml-audio
    sfml-network)

set_target_properties(graphic PROPERTIES CXX_STANDARD 20 CXX_STANDARD_REQUIRED YES)