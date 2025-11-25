cmake_minimum_required(VERSION 3.20)
project(ColorblindAccessibilityPoC)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find SFML
find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)

# Add the executable
add_executable(colorblind_poc
    main.cpp
    ColorblindSimulator.cpp
    VisualCueSystem.cpp
    Projectile.cpp
)

# Link SFML
target_link_libraries(colorblind_poc PRIVATE
    sfml-graphics
    sfml-window
    sfml-system
)

# Set output directory
set_target_properties(colorblind_poc PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
