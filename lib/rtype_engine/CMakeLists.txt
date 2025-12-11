# ============================================================================
# Engine Library (Static) - Aggregates ECS and Core
# ============================================================================

add_library(rtype_engine STATIC
    src/GameEngineFactory.cpp
)

target_sources(rtype_engine PRIVATE
    src/GameEngineFactory.hpp
    src/IGameEngine.hpp
    src/AGameEngine.hpp
    src/ISystem.hpp
    src/ASystem.hpp
)

target_link_libraries(rtype_engine PUBLIC
    rtype_ecs
    rtype_engine_core
)

target_include_directories(rtype_engine PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/include
)

target_compile_features(rtype_engine PUBLIC cxx_std_20)

if(MSVC)
    target_compile_options(rtype_engine PRIVATE /W4)
else()
    target_compile_options(rtype_engine PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_library(RType::Engine ALIAS rtype_engine)
