add_subdirectory(GraphicRaylib)
add_subdirectory(GraphicSFML)
add_subdirectory(GraphicSDL)

add_executable(r-type_client main.cpp ClientApp.cpp)

target_compile_features(r-type_client PRIVATE cxx_std_20)

target_link_libraries(r-type_client PRIVATE
    rtype_engine
    rtype_network
    rtype_game_shared
    rtype_game_client
    rtype_graphic_client_raylib
    rtype_graphic_client_sfml
    rtype_graphic_client_sdl
)
