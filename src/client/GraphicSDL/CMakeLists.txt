set(SDL_TEST OFF CACHE BOOL "Disable SDL tests" FORCE)
add_subdirectory(SDL)

add_library(rtype_graphic_client_sdl
    Graphic.cpp
)

target_compile_features(rtype_graphic_client_sdl PRIVATE cxx_std_20)

target_include_directories(rtype_graphic_client_sdl PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(rtype_graphic_client_sdl PUBLIC SDL3::SDL3)