# ============================================================================
# Engine Library (Static) - Aggregates ECS and Core
# ============================================================================

add_library(engine STATIC)

# Engine itself has no sources, it's purely an aggregator
# But we need to add at least one dummy source to create the library
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/engine_dummy.cpp" "// Engine aggregation library")
target_sources(engine PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/engine_dummy.cpp")

target_link_libraries(engine PUBLIC
    ecs
    engine_core
)

target_include_directories(engine PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/include
)

target_compile_features(engine PUBLIC cxx_std_20)

if(MSVC)
    target_compile_options(engine PRIVATE /W4)
else()
    target_compile_options(engine PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_library(RType::Engine ALIAS engine)
