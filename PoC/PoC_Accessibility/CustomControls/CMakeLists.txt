cmake_minimum_required(VERSION 3.20)
project(AccessibilityPoC)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add the executable
add_executable(accessibility_poc
    main.cpp
    InputManager.cpp
    InputConfigSerializer.cpp
    AutoFireComponent.cpp
)

# Link with filesystem library if needed
target_link_libraries(accessibility_poc PRIVATE stdc++fs)
