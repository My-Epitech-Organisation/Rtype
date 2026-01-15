add_library(rtype-graphic-background-spatialStation SHARED
        entrypoint.cpp
        SpatialStation.cpp
        SpatialStation.hpp
)

target_include_directories(rtype-graphic-background-spatialStation PUBLIC
        ${CMAKE_SOURCE_DIR}/src/games/
        ${CMAKE_SOURCE_DIR}/src/games/rtype/shared/
        ${CMAKE_SOURCE_DIR}
)

target_link_libraries(rtype-graphic-background-spatialStation PRIVATE ecs)

add_custom_command(TARGET rtype-graphic-background-spatialStation POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/plugins/background
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:rtype-graphic-background-spatialStation> ${CMAKE_BINARY_DIR}/plugins/background/$<TARGET_FILE_NAME:rtype-graphic-background-spatialStation>
    COMMENT "Copying rtype-graphic-background-spatialStation to ${CMAKE_BINARY_DIR}/plugins/background"
)
