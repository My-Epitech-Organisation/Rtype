# ============================================================================
# Common Utilities Library (Header-Only)
# ============================================================================

add_library(rtype_common INTERFACE)

target_include_directories(rtype_common INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Logger
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ArgParser
    ${CMAKE_SOURCE_DIR}/include
)

target_compile_features(rtype_common INTERFACE cxx_std_20)

add_library(RType::Common ALIAS rtype_common)
