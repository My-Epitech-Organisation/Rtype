add_library(rtype_game_shared_systems OBJECT
    MovementSystem.cpp
)

target_include_directories(rtype_game_shared_systems PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)
