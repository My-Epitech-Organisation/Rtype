add_subdirectory(Systems)

add_library(rtype_game_server STATIC
    $<TARGET_OBJECTS:rtype_game_server_systems>
)

target_include_directories(rtype_game_server PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_features(rtype_game_server PUBLIC cxx_std_20)

# Server depends on engine, network, and shared
target_link_libraries(rtype_game_server PUBLIC
    rtype_engine
    rtype_network
    rtype_game_shared
)
