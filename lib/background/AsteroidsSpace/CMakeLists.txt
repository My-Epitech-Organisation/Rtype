add_library(rtype-graphic-background-asteroidsSpace SHARED
        entrypoint.cpp
        AsteroidsSpace.cpp
        AsteroidsSpace.hpp
)

target_include_directories(rtype-graphic-background-asteroidsSpace PUBLIC
        ${CMAKE_SOURCE_DIR}/src/games/
        ${CMAKE_SOURCE_DIR}/src/games/rtype/shared/
        ${CMAKE_SOURCE_DIR}
)

target_link_libraries(rtype-graphic-background-asteroidsSpace PRIVATE ecs)
