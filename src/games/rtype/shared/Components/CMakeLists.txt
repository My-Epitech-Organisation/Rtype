# Components are header-only, no source files needed
# This CMakeLists.txt is kept for potential future compiled components

add_library(rtype_game_shared_components INTERFACE)

target_include_directories(rtype_game_shared_components INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)
