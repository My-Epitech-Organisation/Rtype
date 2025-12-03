add_library(rtype-graphic-entity-factory
    STATIC
    EntityFactory.hpp
    EntityFactory.cpp
)

target_link_libraries(rtype-graphic-entity-factory
    PUBLIC
    rtype-graphic-SFML
    rtype-graphic-components
    rtype-common-components
    rtype-graphic-asset-manager
    rtype-graphic-scene-manager
)
