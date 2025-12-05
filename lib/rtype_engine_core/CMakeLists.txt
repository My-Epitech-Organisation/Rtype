# ============================================================================
# Engine Core Library (Static)
# ============================================================================

add_library(rtype_engine_core STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/Time.cpp
)

target_include_directories(rtype_engine_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core
    ${CMAKE_CURRENT_SOURCE_DIR}/../rtype_common/src
    ${CMAKE_CURRENT_SOURCE_DIR}/../rtype_common/src/Logger
)

target_compile_features(rtype_engine_core PUBLIC cxx_std_20)

target_link_libraries(rtype_engine_core PUBLIC rtype_common)

if(MSVC)
    target_compile_options(rtype_engine_core PRIVATE /W4)
else()
    target_compile_options(rtype_engine_core PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_library(RType::EngineCore ALIAS rtype_engine_core)
