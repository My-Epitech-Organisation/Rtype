add_library(rtype_game_shared_components OBJECT
    TransformComponent.cpp
)

target_include_directories(rtype_game_shared_components PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)
