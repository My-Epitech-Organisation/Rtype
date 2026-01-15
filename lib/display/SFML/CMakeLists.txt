    add_library(rtype-display-sfml SHARED
    SFMLTexture.cpp
    SFMLTexture.hpp
    SFMLDisplay.hpp
    SFMLDisplay.cpp
    SFMLMusic.cpp
    SFMLMusic.hpp
    SFMLSound.cpp
    SFMLSound.hpp
    SFMLSoundBuffer.cpp
    SFMLSoundBuffer.hpp
    SFMLFont.cpp
    SFMLFont.hpp
    ../ADisplay.cpp
    ../ADisplay.hpp
    entrypoint.cpp
)

rtype_find_sfml()
rtype_find_sdl2()

set_target_properties(rtype-display-sfml PROPERTIES LINK_FLAGS "-Wl,--export-dynamic")

target_link_libraries(rtype-display-sfml
        PUBLIC
        SFML::Network
        SFML::Graphics
        SFML::Window
        SFML::Audio
        SFML::System
        SDL2::SDL2
)

add_custom_command(TARGET rtype-display-sfml POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/plugins
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:rtype-display-sfml> ${CMAKE_BINARY_DIR}/plugins/display.so
    COMMENT "Copying rtype-display-sfml to ${CMAKE_BINARY_DIR}/plugins/display.so"
)