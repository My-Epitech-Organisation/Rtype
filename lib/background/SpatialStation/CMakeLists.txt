add_library(rtype-graphic-background-spatialStation SHARED
        entrypoint.cpp
        SpatialStation.cpp
        SpatialStation.hpp
)

target_include_directories(rtype-graphic-background-spatialStation PUBLIC
        ${CMAKE_SOURCE_DIR}/src/games/
        ${CMAKE_SOURCE_DIR}/src/games/rtype/shared/
        ${CMAKE_SOURCE_DIR}
)

target_link_libraries(rtype-graphic-background-spatialStation PRIVATE ecs)
