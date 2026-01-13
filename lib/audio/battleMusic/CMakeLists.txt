add_library(rtype-graphic-music-battleMusic SHARED
    entrypoint.cpp
    BattleMusic.cpp
    BattleMusic.hpp
    ${CMAKE_SOURCE_DIR}/src/client/Graphic/AudioLib/AudioLib.cpp
)

target_include_directories(rtype-graphic-music-battleMusic PUBLIC
        ${CMAKE_SOURCE_DIR}/src/games/
        ${CMAKE_SOURCE_DIR}/src/games/rtype/shared/
        ${CMAKE_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/src/client/Graphic/AudioLib
)

target_link_libraries(rtype-graphic-music-battleMusic PRIVATE ecs)
