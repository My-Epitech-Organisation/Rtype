# Components library with source files
add_library(rtype_game_shared_components STATIC
    CooldownComponent.cpp
    ChargedProjectileComponent.cpp
)

target_include_directories(rtype_game_shared_components PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)
