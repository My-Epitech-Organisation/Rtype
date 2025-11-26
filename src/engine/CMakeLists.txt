add_subdirectory(ecs)
add_subdirectory(core)

# Create unified engine library
add_library(rtype_engine INTERFACE)
target_link_libraries(rtype_engine INTERFACE
    rtype_ecs
    rtype_engine_core
)
target_include_directories(rtype_engine INTERFACE
    ${CMAKE_SOURCE_DIR}/include
)
