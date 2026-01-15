add_library(rtype-graphic-background-meteorInside SHARED
        entrypoint.cpp
        MeteorInside.cpp
        MeteorInside.hpp
)

target_include_directories(rtype-graphic-background-meteorInside PUBLIC
        ${CMAKE_SOURCE_DIR}/src/games/
        ${CMAKE_SOURCE_DIR}/src/games/rtype/shared/
        ${CMAKE_SOURCE_DIR}
)

target_link_libraries(rtype-graphic-background-meteorInside PRIVATE ecs)

add_custom_command(TARGET rtype-graphic-background-meteorInside POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/plugins/background
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:rtype-graphic-background-meteorInside> ${CMAKE_BINARY_DIR}/plugins/background/$<TARGET_FILE_NAME:rtype-graphic-background-meteorInside>
    COMMENT "Copying rtype-graphic-background-meteorInside to ${CMAKE_BINARY_DIR}/plugins/background"
)
