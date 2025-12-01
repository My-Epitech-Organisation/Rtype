add_library(rtype_common INTERFACE)

target_include_directories(rtype_common INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/ArgParser
)
