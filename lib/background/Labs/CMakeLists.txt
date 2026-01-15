add_library(rtype-graphic-background-labs SHARED
        entrypoint.cpp
        Labs.cpp
        Labs.hpp
)

target_include_directories(rtype-graphic-background-labs PUBLIC
        ${CMAKE_SOURCE_DIR}/src/games/
        ${CMAKE_SOURCE_DIR}/src/games/rtype/shared/
        ${CMAKE_SOURCE_DIR}
)

target_link_libraries(rtype-graphic-background-labs PRIVATE ecs)

add_custom_command(TARGET rtype-graphic-background-labs POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/plugins/background
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:rtype-graphic-background-labs> ${CMAKE_BINARY_DIR}/plugins/background/$<TARGET_FILE_NAME:rtype-graphic-background-labs>
    COMMENT "Copying rtype-graphic-background-labs to ${CMAKE_BINARY_DIR}/plugins/background"
)
