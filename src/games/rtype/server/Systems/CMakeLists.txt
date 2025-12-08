add_library(rtype_game_server_systems OBJECT
    Spawner/SpawnerSystem.cpp
    Cleanup/CleanupSystem.cpp
    Destroy/DestroySystem.cpp
)

target_include_directories(rtype_game_server_systems PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(rtype_game_server_systems PUBLIC
    rtype_engine
    rtype_toml_parser
)
