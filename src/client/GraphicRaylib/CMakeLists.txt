add_subdirectory(raylib)

add_library(rtype_graphic_client
    graphic.cpp
)

target_link_libraries(rtype_graphic_client raylib)