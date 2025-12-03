add_library(rtype_game_shared_systems OBJECT
    Movements/MovementSystem.cpp
    AISystem/AISystem.cpp
)

target_include_directories(rtype_game_shared_systems PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(rtype_game_shared_systems PUBLIC
    rtype_engine
)
