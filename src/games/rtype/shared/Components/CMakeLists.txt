# Components library with source files
add_library(rtype_game_shared_components STATIC
    BossComponent.cpp
    BossPatternComponent.cpp
    CooldownComponent.cpp
    ChargedProjectileComponent.cpp
    WeakPointComponent.cpp
)

target_include_directories(rtype_game_shared_components PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)
