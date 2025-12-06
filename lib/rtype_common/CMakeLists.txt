# ============================================================================
# Common Utilities Library (Header-Only)
# ============================================================================


add_library(rtype_common INTERFACE)

target_include_directories(rtype_common INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Logger
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ArgParser
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Config
    ${CMAKE_SOURCE_DIR}/include
)

target_compile_features(rtype_common INTERFACE cxx_std_20)

add_library(RType::Common ALIAS rtype_common)

# ============================================================================
# TOML Parser Library (Static)
# ============================================================================

find_package(tomlplusplus CONFIG REQUIRED)

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

target_compile_features(rtype_toml_parser PUBLIC cxx_std_20)

add_library(RType::TomlParser ALIAS rtype_toml_parser)
