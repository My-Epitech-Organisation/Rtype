cmake_minimum_required(VERSION 3.20)
project(SQLiteStoragePoC LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find SQLite3
find_package(PkgConfig)
pkg_check_modules(SQLITE3 REQUIRED sqlite3)

# Add the ECS library path
set(ECS_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../src/ECS")
set(ECS_LIB_PATH "${ECS_PATH}/build")

# Create the PoC executable
add_executable(sqlite_storage_poc
    main.cpp
)

# Link against ECS library and SQLite3
target_include_directories(sqlite_storage_poc PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src
    ${SQLITE3_INCLUDE_DIRS}
)

# Find the ECS static library
find_library(ECS_LIBRARY 
    NAMES librtype_ecs.a rtype_ecs
    PATHS ${ECS_LIB_PATH}
    NO_DEFAULT_PATH
)

target_link_libraries(sqlite_storage_poc PRIVATE
    ${ECS_LIBRARY}
    ${SQLITE3_LIBRARIES}
    pthread
)

# Add compiler flags
target_compile_options(sqlite_storage_poc PRIVATE
    -Wall -Wextra -Wpedantic
)
