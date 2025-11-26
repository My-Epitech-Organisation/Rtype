# ============================================================================
# R-Type ECS Library
# ============================================================================

# Source files (explicitly listed for proper CMake dependency tracking)
# set(ECS_SOURCES --- IGNORE For Now ---
# )

# Header files (explicitly listed for IDE integration)
set(ECS_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/core/Entity.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/storage/ISparseSet.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/storage/SparseSet.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/storage/TagSparseSet.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/traits/ComponentTraits.hpp
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
    add_library(rtype_ecs STATIC ${ECS_SOURCES})
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