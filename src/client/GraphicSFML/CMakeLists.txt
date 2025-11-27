add_library(rtype_graphic_client_sfml STATIC
    Graphic.cpp
)

target_include_directories(rtype_graphic_client_sfml PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(rtype_graphic_client_sfml PUBLIC
    sfml-system
    sfml-window
    sfml-graphics
)

set_target_properties(rtype_graphic_client_sfml PROPERTIES CXX_STANDARD 20 CXX_STANDARD_REQUIRED YES)