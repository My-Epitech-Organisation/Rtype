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

# Ensure vcpkg include directories are available for SDL2_ttf.h and SDL2_image.h
if(EXISTS "${CMAKE_BINARY_DIR}/vcpkg_installed/x64-linux/include")
    target_include_directories(rtype-display-sdl2 PUBLIC BEFORE "${CMAKE_BINARY_DIR}/vcpkg_installed/x64-linux/include")
    target_include_directories(rtype-display-sdl2 PUBLIC BEFORE "${CMAKE_BINARY_DIR}/vcpkg_installed/x64-linux/include/SDL2")
endif()

# Find SDL2_ttf (prefer shared library to match SDL2::SDL2)
find_package(SDL2_ttf CONFIG QUIET)
if(TARGET SDL2_ttf::SDL2_ttf)
    # Use the generator expression to pick shared over static, matching SDL2 linkage
    target_link_libraries(rtype-display-sdl2 PUBLIC
        $<IF:$<TARGET_EXISTS:SDL2_ttf::SDL2_ttf>,SDL2_ttf::SDL2_ttf,SDL2_ttf::SDL2_ttf-static>)
elseif(TARGET SDL2::SDL2_ttf)
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
            else()
                message(FATAL_ERROR "SDL2_ttf library not found. Please install via vcpkg or system package manager.")
            endif()
        endif()
    else()
        find_library(SDL2_TTF_LIBRARY NAMES SDL2_ttf)
        if(SDL2_TTF_LIBRARY)
            target_link_libraries(rtype-display-sdl2 PUBLIC ${SDL2_TTF_LIBRARY})
        else()
            message(FATAL_ERROR "SDL2_ttf library not found. Please install via vcpkg or system package manager.")
        endif()
    endif()
endif()

# Find SDL2_image (prefer shared library to match SDL2::SDL2)
find_package(SDL2_image CONFIG QUIET)
if(TARGET SDL2_image::SDL2_image)
    # Use the generator expression to pick shared over static, matching SDL2 linkage
    target_link_libraries(rtype-display-sdl2 PUBLIC
        $<IF:$<TARGET_EXISTS:SDL2_image::SDL2_image>,SDL2_image::SDL2_image,SDL2_image::SDL2_image-static>)
elseif(TARGET SDL2::SDL2_image)
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
            else()
                message(WARNING "SDL2_image library not found. Image loading may not work.")
            endif()
        endif()
    else()
        find_library(SDL2_IMAGE_LIBRARY NAMES SDL2_image)
        if(SDL2_IMAGE_LIBRARY)
            target_link_libraries(rtype-display-sdl2 PUBLIC ${SDL2_IMAGE_LIBRARY})
        else()
            message(WARNING "SDL2_image library not found. Image loading may not work.")
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
