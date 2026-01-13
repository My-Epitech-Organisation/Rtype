add_subdirectory(SceneManager)
add_subdirectory(AssetManager)
add_subdirectory(AudioLib)
add_subdirectory(EntityFactory)

rtype_find_sdl2()

add_library(rtype-graphic
    STATIC
    Graphic.cpp
    Graphic.hpp
    KeyboardActions.cpp
    KeyboardActions.hpp
    ControllerRumble.cpp
    ControllerRumble.hpp
    ${CMAKE_SOURCE_DIR}/src/client/DevConsole/DevConsole.cpp
    ${CMAKE_SOURCE_DIR}/src/client/DevConsole/DevConsole.hpp
)

target_include_directories(rtype-graphic PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}
)

target_include_directories(rtype-graphic
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ../
)

target_link_libraries(rtype-graphic
    PUBLIC
    rtype-graphic-asset-manager
    PRIVATE
    SDL2::SDL2
)

target_include_directories(rtype-graphic-main-menu-scene
    PUBLIC
    ./
)