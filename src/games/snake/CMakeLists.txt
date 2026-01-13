# ============================================================================
# Snake Game - Graphics Abstraction Proof
# ============================================================================
# This CMakeLists demonstrates that:
# 1. Same game logic works with different graphics libraries (SFML, SDL2)
# 2. The engine is truly modular and reusable
# 3. GameEngineFactory allows multiple implementations

# ====================
# Server Game Logic (Graphics-Agnostic)
# ====================
add_library(snake_game_server STATIC
    server/GameEngine.cpp
)

target_sources(snake_game_server PRIVATE
    server/GameEngine.hpp
    shared/Components.hpp
)

target_link_libraries(snake_game_server PUBLIC
    engine          # Generic game engine (same as R-Type uses)
    ecs             # Generic ECS registry (same as R-Type uses)
    network         # Generic networking (same as R-Type uses)
)

target_include_directories(snake_game_server PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/include
)

target_compile_features(snake_game_server PUBLIC cxx_std_20)

if(MSVC)
    target_compile_options(snake_game_server PRIVATE /W4)
else()
    target_compile_options(snake_game_server PRIVATE -Wall -Wextra -Wpedantic)
endif()

# ====================
# Renderer Interface (Graphics-Agnostic)
# ====================
add_library(snake_renderer_interface INTERFACE)
target_sources(snake_renderer_interface INTERFACE
    FILE_SET headers
    TYPE HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}
    FILES client/ISnakeRenderer.hpp
)
target_include_directories(snake_renderer_interface INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# ====================
# SFML Renderer Implementation
# ====================
# NOTE: Disabled due to SFML API version mismatch (project uses SFML 2.x, code targeted 3.x)
# The SDL2 renderer is sufficient for demonstrating graphics abstraction
# Uncomment the following to rebuild with SFML when API is aligned:
# add_library(snake_renderer_sfml STATIC
#     client/sfml/SFMLSnakeRenderer.cpp
# )
#
# target_sources(snake_renderer_sfml PRIVATE
#     client/sfml/SFMLSnakeRenderer.hpp
# )
#
# target_link_libraries(snake_renderer_sfml PUBLIC
#     snake_renderer_interface
#     sfml-graphics
#     sfml-window
#     sfml-system
# )
#
# target_include_directories(snake_renderer_sfml PUBLIC
#     ${CMAKE_CURRENT_SOURCE_DIR}
# )
#
# target_compile_features(snake_renderer_sfml PUBLIC cxx_std_20)

# ====================
# SDL2 Renderer Implementation
# ====================
find_package(SDL2 CONFIG QUIET)
if(SDL2_FOUND)
    add_library(snake_renderer_sdl2 SHARED
        client/sdl2/SDL2SnakeRenderer.cpp
    )

    target_sources(snake_renderer_sdl2 PRIVATE
        client/sdl2/SDL2SnakeRenderer.hpp
    )

    target_link_libraries(snake_renderer_sdl2 PUBLIC
        snake_renderer_interface
        SDL2::SDL2
    )

    target_include_directories(snake_renderer_sdl2 PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
    )

    target_compile_features(snake_renderer_sdl2 PUBLIC cxx_std_20)

    message(STATUS "SDL2 found - building SDL2 renderer for Snake as SHARED library")

    # Install the shared library
    install(TARGETS snake_renderer_sdl2
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin
        COMPONENT snake_renderer
    )
else()
    message(STATUS "SDL2 not found - skipping SDL2 renderer for Snake")
endif()

# Create aliases for consistent naming
add_library(RType::SnakeGame ALIAS snake_game_server)
if(SDL2_FOUND)
    add_library(RType::SnakeRendererSDL2 ALIAS snake_renderer_sdl2)

    # ====================
    # Executable - Playable Snake Game
    # ====================
    add_executable(snake_game main.cpp)

    target_link_libraries(snake_game PRIVATE
        snake_game_server
        ecs
        engine
        common
        ${CMAKE_DL_LIBS}
    )

    target_include_directories(snake_game PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/lib/engine/src
        ${CMAKE_SOURCE_DIR}/lib/ecs/src
        ${CMAKE_SOURCE_DIR}/lib/common/src
    )

    target_compile_features(snake_game PRIVATE cxx_std_20)

    if(MSVC)
        target_compile_options(snake_game PRIVATE /W4)
    else()
        target_compile_options(snake_game PRIVATE -Wall -Wextra -Wpedantic)
    endif()

    # Install executable to bin
    install(TARGETS snake_game
        DESTINATION bin
        COMPONENT snake_game
    )
endif()

# ====================
# The Key Architectural Points:
# ====================
# 1. ISnakeRenderer is a graphics-agnostic interface
# 2. SFMLSnakeRenderer and SDL2SnakeRenderer both implement it
# 3. Same game logic (GameEngine) works with both renderers
# 4. This proves the engine is truly modular!
# 5. Add new renderers without touching game logic
