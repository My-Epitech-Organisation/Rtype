cmake_minimum_required(VERSION 3.20)
project(SlowModePoC VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Executable for the main PoC
add_executable(slow_mode_poc
    main.cpp
    TimeSystem.cpp
    GameEntity.cpp
    DifficultyManager.cpp
)

target_include_directories(slow_mode_poc PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Enable warnings
if(MSVC)
    target_compile_options(slow_mode_poc PRIVATE /W4)
else()
    target_compile_options(slow_mode_poc PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Install target
install(TARGETS slow_mode_poc
    RUNTIME DESTINATION bin
)
