add_library(rtype-graphic-background-lobby SHARED
    entrypoint.cpp
        LobbyBackground.cpp
        LobbyBackground.hpp
)

target_include_directories(rtype-graphic-background-lobby PUBLIC
        ${CMAKE_SOURCE_DIR}/src/games/
        ${CMAKE_SOURCE_DIR}/src/games/rtype/shared/
        ${CMAKE_SOURCE_DIR}
)

target_link_libraries(rtype-graphic-background-lobby PRIVATE ecs)
