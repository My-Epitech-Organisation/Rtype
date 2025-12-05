add_library(rtype-graphic-SFML
    STATIC
    Graphic.cpp
    Graphic.hpp
    SceneManager.cpp
    SceneManager.hpp
    KeyboardActions.cpp
    KeyboardActions.hpp
)

target_include_directories(rtype-graphic-SFML PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

find_package(SFML
        COMPONENTS
        Network
        Graphics
        Window
        Audio
        System
        CONFIG
        REQUIRED
)

target_link_libraries(rtype-graphic-SFML
        PUBLIC
        SFML::Network
        SFML::Graphics
        SFML::Window
        SFML::Audio
        SFML::System
        rtype_engine
)

target_include_directories(rtype-graphic-SFML
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ../
)