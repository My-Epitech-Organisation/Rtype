add_library(rtype-graphic-entity-factory
    STATIC
    EntityFactory.hpp
    EntityFactory.cpp
)

target_link_libraries(rtype-graphic-entity-factory
    PUBLIC
    rtype-graphic
    rtype-graphic-asset-manager
    rtype-graphic-scene-manager
    rtype_game_shared
    rtype_game_client
)

target_include_directories(rtype-graphic-entity-factory PUBLIC
        ${CMAKE_SOURCE_DIR}
)
