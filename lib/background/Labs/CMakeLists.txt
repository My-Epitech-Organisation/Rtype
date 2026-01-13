add_library(rtype-graphic-background-labs SHARED
        entrypoint.cpp
        Labs.cpp
        Labs.hpp
)

target_include_directories(rtype-graphic-background-labs PUBLIC
        ${CMAKE_SOURCE_DIR}/src/games/
        ${CMAKE_SOURCE_DIR}/src/games/rtype/shared/
        ${CMAKE_SOURCE_DIR}
)

target_link_libraries(rtype-graphic-background-labs PRIVATE ecs)
