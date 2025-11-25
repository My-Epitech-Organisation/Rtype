add_library(rtype_game_shared_components OBJECT
    TransformComponent.cpp
)

target_include_directories(rtype_game_shared_components PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)
