cmake_minimum_required(VERSION 3.10)
project(PoC_Memory_Optimization)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../ECS)

# Object Pool PoC executable
add_executable(poc_object_pool
    ObjectPool/ObjectPool.hpp
    ObjectPool/test_object_pool.cpp
)

# Memory Pool PoC executable
add_executable(poc_memory_pool
    MemoryPool/MemoryPool.hpp
    MemoryPool/test_memory_pool.cpp
)

# Enable warnings
target_compile_options(poc_object_pool PRIVATE -Wall -Wextra -Werror)
target_compile_options(poc_memory_pool PRIVATE -Wall -Wextra -Werror)

# Link pthread for potential threading
target_link_libraries(poc_object_pool pthread)
target_link_libraries(poc_memory_pool pthread)
