set(SDL_TEST OFF CACHE BOOL "Disable SDL tests" FORCE)
add_subdirectory(SDL)

add_library(rtype_graphics_client
    Graphic.cpp
)

target_compile_features(rtype_graphics_client PRIVATE cxx_std_20)

target_include_directories(rtype_graphics_client PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(rtype_graphics_client PUBLIC SDL3::SDL3)