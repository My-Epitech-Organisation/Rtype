cmake_minimum_required(VERSION 3.20)
project(JSON_Storage_PoC LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include FetchContent for downloading nlohmann/json
include(FetchContent)

# Fetch nlohmann/json library
FetchContent_Declare(
    json
    URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
)
FetchContent_MakeAvailable(json)

# Add the ECS library path
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../src/ECS ${CMAKE_CURRENT_BINARY_DIR}/ecs)

# Create the PoC executable
add_executable(json_storage_poc
    main.cpp
    GameConfig.hpp
    JsonComponentSerializer.hpp
)

# Link libraries
target_link_libraries(json_storage_poc
    PRIVATE
        nlohmann_json::nlohmann_json
        rtype_ecs_static
)

# Include directories
target_include_directories(json_storage_poc
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# Compiler warnings
if(MSVC)
    target_compile_options(json_storage_poc PRIVATE /W4)
else()
    target_compile_options(json_storage_poc PRIVATE -Wall -Wextra -Wpedantic)
endif()
