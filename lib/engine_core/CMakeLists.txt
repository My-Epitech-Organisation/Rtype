# ============================================================================
# Engine Core Library (Static)
# ============================================================================

add_library(engine_core STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Time.cpp
)

target_include_directories(engine_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/../common/src
    ${CMAKE_CURRENT_SOURCE_DIR}/../common/src/Logger
)

target_compile_features(engine_core PUBLIC cxx_std_20)

target_link_libraries(engine_core PUBLIC common)

if(MSVC)
    target_compile_options(engine_core PRIVATE /W4)
else()
    target_compile_options(engine_core PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_library(RType::EngineCore ALIAS engine_core)
