add_library(rtype-graphic-music-chillMusic SHARED
    entrypoint.cpp
    ChillMusic.cpp
    ChillMusic.hpp
    ${CMAKE_SOURCE_DIR}/src/client/Graphic/AudioLib/AudioLib.cpp
)

target_include_directories(rtype-graphic-music-chillMusic PUBLIC
        ${CMAKE_SOURCE_DIR}/src/games/
        ${CMAKE_SOURCE_DIR}/src/games/rtype/shared/
        ${CMAKE_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/src/client/Graphic/AudioLib
)

target_link_libraries(rtype-graphic-music-chillMusic PRIVATE ecs)
