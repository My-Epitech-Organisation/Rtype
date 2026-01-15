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
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
)

target_compile_features(rtype_game_shared_components PUBLIC cxx_std_20)

target_link_libraries(rtype_game_shared_components PUBLIC RType::ECS)
