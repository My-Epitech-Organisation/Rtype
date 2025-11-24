add_subdirectory(Systems)

add_library(rtype_game_client STATIC
    $<TARGET_OBJECTS:rtype_game_client_systems>
)

target_include_directories(rtype_game_client PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_features(rtype_game_client PUBLIC cxx_std_20)

# Client depends on engine, network, and shared
target_link_libraries(rtype_game_client PUBLIC
    rtype_engine
    rtype_network
    rtype_game_shared
)
