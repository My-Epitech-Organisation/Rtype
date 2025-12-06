add_subdirectory(Components)
add_subdirectory(Systems)

# R-Type specific configuration library
add_library(rtype_game_config STATIC
    Config/GameConfig/RTypeGameConfig.cpp
    Config/GameState/RTypeGameState.cpp
    Config/Parser/RTypeConfigParser.cpp
    Config/SaveManager/RTypeSaveManager.cpp
    Config/SaveManager/Serialization/BinarySerializer.cpp
    Config/SaveManager/Serialization/GameStateSerializer.cpp
    Config/SaveManager/Operations/FileOperations.cpp
    Config/SaveManager/Operations/SaveInfoReader.cpp
)

target_include_directories(rtype_game_config PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(rtype_game_config PUBLIC
    rtype_toml_parser
)

# Create shared game lib
add_library(rtype_game_shared STATIC
    $<TARGET_OBJECTS:rtype_game_shared_systems>
    ${CMAKE_CURRENT_SOURCE_DIR}/Config/EntityConfig/EntityConfig.cpp
)

target_include_directories(rtype_game_shared PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
)

target_compile_features(rtype_game_shared PUBLIC cxx_std_20)

# Shared depends on engine, config, and components (interface lib)
target_link_libraries(rtype_game_shared PUBLIC
    rtype_engine
    rtype_game_shared_components
    rtype_game_config
)
