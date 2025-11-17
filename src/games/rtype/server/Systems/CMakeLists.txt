add_library(rtype_game_server_systems OBJECT
    EnemySpawnSystem.cpp
)

target_include_directories(rtype_game_server_systems PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)
