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

add_custom_command(TARGET rtype-graphic-background-asteroidsSpace POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/plugins/background
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:rtype-graphic-background-asteroidsSpace> ${CMAKE_BINARY_DIR}/plugins/background/$<TARGET_FILE_NAME:rtype-graphic-background-asteroidsSpace>
    COMMENT "Copying rtype-graphic-background-asteroidsSpace to ${CMAKE_BINARY_DIR}/plugins/background"
)
