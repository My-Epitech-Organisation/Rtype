add_subdirectory(raylib)

add_library(rtype_graphic_client_raylib
    Graphic.cpp
)

target_link_libraries(rtype_graphic_client_raylib raylib)