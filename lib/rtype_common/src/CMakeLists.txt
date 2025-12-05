find_package(tomlplusplus CONFIG REQUIRED)

# Generic TOML parser library (game-agnostic)
add_library(rtype_toml_parser STATIC
    Config/TomlParser.cpp
)

target_include_directories(rtype_toml_parser PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(rtype_toml_parser PUBLIC
    tomlplusplus::tomlplusplus
)

add_library(rtype_common INTERFACE)

target_include_directories(rtype_common INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/Logger
    ${CMAKE_CURRENT_SOURCE_DIR}/ArgParser
)
