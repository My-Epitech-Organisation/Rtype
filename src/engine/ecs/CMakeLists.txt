# ============================================================================
# R-Type ECS Library
# ============================================================================

# Collect source files automatically
file(GLOB_RECURSE ECS_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

# Collect header files automatically
file(GLOB_RECURSE ECS_HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.inl"
)

# Header-only library (no .cpp files yet or all are commented)
if(NOT ECS_SOURCES)
    add_library(rtype_ecs INTERFACE)
    target_include_directories(rtype_ecs INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/..
    )

    # Threading support
    find_package(Threads REQUIRED)
    target_link_libraries(rtype_ecs INTERFACE Threads::Threads)

    # Alias
    add_library(RType::ECS ALIAS rtype_ecs)
else()
    # Static library
    add_library(rtype_ecs STATIC ${ECS_SOURCES} ${ECS_HEADERS})
    target_include_directories(rtype_ecs PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/..
    )

    # Threading support
    find_package(Threads REQUIRED)
    target_link_libraries(rtype_ecs PUBLIC Threads::Threads)

    # Compiler warnings
    target_compile_options(rtype_ecs PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/W4>
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic>
    )

    # Alias
    add_library(RType::ECS ALIAS rtype_ecs)
endif()