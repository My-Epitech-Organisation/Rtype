add_library(rtype-graphic-background-blackHole SHARED
        entrypoint.cpp
        BlackHole.cpp
        BlackHole.hpp
)

target_include_directories(rtype-graphic-background-blackHole PUBLIC
        ${CMAKE_SOURCE_DIR}/src/games/
        ${CMAKE_SOURCE_DIR}/src/games/rtype/shared/
        ${CMAKE_SOURCE_DIR}
)

target_link_libraries(rtype-graphic-background-blackHole PRIVATE ecs)
