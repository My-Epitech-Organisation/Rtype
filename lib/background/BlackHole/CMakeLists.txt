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

add_custom_command(TARGET rtype-graphic-background-blackHole POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/plugins/background
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:rtype-graphic-background-blackHole> ${CMAKE_BINARY_DIR}/plugins/background/$<TARGET_FILE_NAME:rtype-graphic-background-blackHole>
    COMMENT "Copying rtype-graphic-background-blackHole to ${CMAKE_BINARY_DIR}/plugins/background"
)
