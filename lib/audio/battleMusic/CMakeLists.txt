add_library(rtype-graphic-music-battleMusic SHARED
    entrypoint.cpp
    BattleMusic.cpp
    BattleMusic.hpp
)

target_include_directories(rtype-graphic-music-battleMusic PUBLIC
        ${CMAKE_SOURCE_DIR}/src/games/
        ${CMAKE_SOURCE_DIR}/src/games/rtype/shared/
        ${CMAKE_SOURCE_DIR}
)

target_link_libraries(rtype-graphic-music-battleMusic PRIVATE ecs)
