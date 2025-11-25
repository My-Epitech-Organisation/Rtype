add_library(rtype_game_client_systems OBJECT
    RenderSystem.cpp
)

target_include_directories(rtype_game_client_systems PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)
