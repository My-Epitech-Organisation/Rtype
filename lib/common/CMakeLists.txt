# ============================================================================
# Common Utilities Library (Header-Only)
# ============================================================================


add_library(common INTERFACE)

target_include_directories(common INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Logger
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ArgParser
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Config
    ${CMAKE_SOURCE_DIR}/include
)

target_compile_features(common INTERFACE cxx_std_20)

add_library(RType::Common ALIAS common)

# ============================================================================
# TOML Parser Library (Static)
# ============================================================================

rtype_find_tomlplusplus()

add_library(rtype_toml_parser STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Config/TomlParser.cpp
)

target_include_directories(rtype_toml_parser PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Config
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(rtype_toml_parser PUBLIC
    tomlplusplus::tomlplusplus
)

# Ensure consumers of common also get tomlplusplus includes
target_link_libraries(common INTERFACE
    tomlplusplus::tomlplusplus
)

target_compile_features(rtype_toml_parser PUBLIC cxx_std_20)

add_library(RType::TomlParser ALIAS rtype_toml_parser)
