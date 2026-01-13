add_library(rtype-graphic-level-creator-scene
    STATIC
    LevelCreatorScene.cpp
    LevelCreatorScene.hpp
)

target_link_libraries(rtype-graphic-level-creator-scene
    PUBLIC
    engine
    network
    rtype-graphic-asset-manager
    rtype-graphic-entity-factory
    rtype-graphic-audio
)

target_link_libraries(rtype-graphic-level-creator-scene
    PRIVATE
    rtype_game_client
    rtype_game_shared
)

target_include_directories(rtype-graphic-level-creator-scene PUBLIC
    ${CMAKE_SOURCE_DIR}/src/client
    ${CMAKE_SOURCE_DIR}/src/client/Graphic
    ${CMAKE_SOURCE_DIR}/src/client/Graphic/SceneManager
    ${CMAKE_SOURCE_DIR}/src/games/rtype/client
)

target_compile_features(rtype-graphic-level-creator-scene PUBLIC cxx_std_20)
