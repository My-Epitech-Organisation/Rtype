add_subdirectory(Components)
add_subdirectory(Systems)

# Create shared game lib
add_library(rtype_game_shared STATIC
    $<TARGET_OBJECTS:rtype_game_shared_components>
    $<TARGET_OBJECTS:rtype_game_shared_systems>
)

target_include_directories(rtype_game_shared PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

target_compile_features(rtype_game_shared PUBLIC cxx_std_20)

# Shared depends only on engine
target_link_libraries(rtype_game_shared PUBLIC
    rtype_engine
)
