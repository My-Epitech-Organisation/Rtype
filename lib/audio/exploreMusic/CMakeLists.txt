add_library(rtype-graphic-music-exploreMusic SHARED
    entrypoint.cpp
    ExploreMusic.cpp
    ExploreMusic.hpp
    ${CMAKE_SOURCE_DIR}/src/client/Graphic/AudioLib/AudioLib.cpp
)

target_include_directories(rtype-graphic-music-exploreMusic PUBLIC
        ${CMAKE_SOURCE_DIR}/src/games/
        ${CMAKE_SOURCE_DIR}/src/games/rtype/shared/
        ${CMAKE_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/src/client/Graphic/AudioLib
)

target_link_libraries(rtype-graphic-music-exploreMusic PRIVATE ecs)

add_custom_command(TARGET rtype-graphic-music-exploreMusic POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/plugins/music
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:rtype-graphic-music-exploreMusic> ${CMAKE_BINARY_DIR}/plugins/music/$<TARGET_FILE_NAME:rtype-graphic-music-exploreMusic>
    COMMENT "Copying rtype-graphic-music-exploreMusic to ${CMAKE_BINARY_DIR}/plugins/music"
)
