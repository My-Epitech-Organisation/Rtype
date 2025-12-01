cmake_minimum_required(VERSION 3.20)
project(RType_PoC)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Fetch nlohmann_json if not found
include(FetchContent)
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
)
FetchContent_MakeAvailable(nlohmann_json)

# Binary vs JSON Storage PoC
add_executable(binary_vs_json_storage
    binary_vs_json_storage.cpp
)

target_link_libraries(binary_vs_json_storage
    PRIVATE
    nlohmann_json::nlohmann_json
)

# Set output directory
set_target_properties(binary_vs_json_storage
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/PoC"
)
