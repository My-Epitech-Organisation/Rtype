# ============================================================================
# Engine Library (Static) - Aggregates ECS and Core
# ============================================================================

add_library(engine STATIC
    src/GameEngineFactory.cpp
)

# Enable Position Independent Code (PIC) to allow linking into shared libraries
set_property(TARGET engine PROPERTY POSITION_INDEPENDENT_CODE ON)

target_sources(engine PRIVATE
    src/GameEngineFactory.hpp
    src/IGameEngine.hpp
    src/AGameEngine.hpp
    src/ISystem.hpp
    src/ASystem.hpp
)

target_link_libraries(engine PUBLIC
    ecs
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
