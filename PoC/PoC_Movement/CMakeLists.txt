cmake_minimum_required(VERSION 3.20)
project(MovementPoCs CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add subdirectories for each PoC
add_subdirectory(LinearMovement)
add_subdirectory(SineWaveMovement)
add_subdirectory(BezierMovement)
add_subdirectory(ScriptedMovement)

# Create a custom target to build all PoCs
add_custom_target(all_movement_pocs
    DEPENDS
        linear_movement_poc
        sine_wave_movement_poc
        bezier_movement_poc
        scripted_movement_poc
)
