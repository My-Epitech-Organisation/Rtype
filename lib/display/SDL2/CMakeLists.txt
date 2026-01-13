add_library(rtype-display-sdl2 SHARED
    SDL2Display.hpp
    SDL2Display.cpp
    SDL2Font.hpp
    SDL2Font.cpp
    SDL2Texture.hpp
    SDL2Texture.cpp
    SDL2SoundBuffer.hpp
    SDL2SoundBuffer.cpp
    SDL2Sound.hpp
    SDL2Sound.cpp
    SDL2Music.hpp
    SDL2Music.cpp
    SDL2AudioEngine.hpp
    SDL2AudioEngine.cpp
    ../ADisplay.cpp
    ../ADisplay.hpp
    entrypoint.cpp
)

rtype_find_sdl2()

set_target_properties(rtype-display-sdl2 PROPERTIES LINK_FLAGS "-Wl,--export-dynamic")

# Find SDL2_ttf (prefer CMake config targets from vcpkg)
find_package(SDL2_ttf CONFIG QUIET)
if(TARGET SDL2::SDL2_ttf)
    target_link_libraries(rtype-display-sdl2 PUBLIC SDL2::SDL2_ttf)
else()
    # Fallback to pkg-config or find_library if CMake config is unavailable
    find_package(PkgConfig QUIET)
    if(PKG_CONFIG_FOUND)
        pkg_check_modules(SDL2_TTF QUIET SDL2_ttf)
        if(SDL2_TTF_FOUND)
            target_include_directories(rtype-display-sdl2 PUBLIC ${SDL2_TTF_INCLUDE_DIRS})
            target_link_libraries(rtype-display-sdl2 PUBLIC ${SDL2_TTF_LIBRARIES})
        else()
            find_library(SDL2_TTF_LIBRARY NAMES SDL2_ttf)
            if(SDL2_TTF_LIBRARY)
                target_link_libraries(rtype-display-sdl2 PUBLIC ${SDL2_TTF_LIBRARY})
            endif()
        endif()
    else()
        find_library(SDL2_TTF_LIBRARY NAMES SDL2_ttf)
        if(SDL2_TTF_LIBRARY)
            target_link_libraries(rtype-display-sdl2 PUBLIC ${SDL2_TTF_LIBRARY})
        endif()
    endif()
endif()

# Find SDL2_image (prefer CMake config targets from vcpkg)
find_package(SDL2_image CONFIG QUIET)
if(TARGET SDL2::SDL2_image)
    target_link_libraries(rtype-display-sdl2 PUBLIC SDL2::SDL2_image)
else()
    # Fallback to pkg-config or find_library if CMake config is unavailable
    if(PKG_CONFIG_FOUND)
        pkg_check_modules(SDL2_IMAGE QUIET SDL2_image)
        if(SDL2_IMAGE_FOUND)
            target_include_directories(rtype-display-sdl2 PUBLIC ${SDL2_IMAGE_INCLUDE_DIRS})
            target_link_libraries(rtype-display-sdl2 PUBLIC ${SDL2_IMAGE_LIBRARIES})
        else()
            find_library(SDL2_IMAGE_LIBRARY NAMES SDL2_image)
            if(SDL2_IMAGE_LIBRARY)
                target_link_libraries(rtype-display-sdl2 PUBLIC ${SDL2_IMAGE_LIBRARY})
            endif()
        endif()
    else()
        find_library(SDL2_IMAGE_LIBRARY NAMES SDL2_image)
        if(SDL2_IMAGE_LIBRARY)
            target_link_libraries(rtype-display-sdl2 PUBLIC ${SDL2_IMAGE_LIBRARY})
        endif()
    endif()
endif()

# Ensure vcpkg-installed lib directories (debug/release) are available to the linker
if(EXISTS "${CMAKE_BINARY_DIR}/vcpkg_installed/x64-linux/debug/lib")
    target_link_directories(rtype-display-sdl2 PUBLIC BEFORE "${CMAKE_BINARY_DIR}/vcpkg_installed/x64-linux/debug/lib")
endif()
if(EXISTS "${CMAKE_BINARY_DIR}/vcpkg_installed/x64-linux/lib")
    target_link_directories(rtype-display-sdl2 PUBLIC BEFORE "${CMAKE_BINARY_DIR}/vcpkg_installed/x64-linux/lib")
endif()

# Link against the CMake-provided SDL2 target (vcpkg or system)
target_link_libraries(rtype-display-sdl2
    PUBLIC
    SDL2::SDL2
)

target_include_directories(rtype-display-sdl2 PUBLIC
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/lib/display
)
