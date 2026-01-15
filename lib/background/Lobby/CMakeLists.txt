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

add_custom_command(TARGET rtype-graphic-background-lobby POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/plugins/background
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:rtype-graphic-background-lobby> ${CMAKE_BINARY_DIR}/plugins/background/$<TARGET_FILE_NAME:rtype-graphic-background-lobby>
    COMMENT "Copying rtype-graphic-background-lobby to ${CMAKE_BINARY_DIR}/plugins/background"
)
