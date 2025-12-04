add_subdirectory(Components)
add_subdirectory(Systems)

# Create shared game lib
add_library(rtype_game_shared STATIC
    $<TARGET_OBJECTS:rtype_game_shared_systems>
)

target_include_directories(rtype_game_shared PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/include
)

target_compile_features(rtype_game_shared PUBLIC cxx_std_20)

# Shared depends on engine and components (interface lib)
target_link_libraries(rtype_game_shared PUBLIC
    rtype_engine
    rtype_game_shared_components
)
