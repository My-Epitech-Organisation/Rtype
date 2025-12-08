add_library(rtype_common INTERFACE)
find_package(tomlplusplus CONFIG REQUIRED)

target_include_directories(rtype_common INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/Logger
    ${CMAKE_CURRENT_SOURCE_DIR}/ArgParser
    ${CMAKE_CURRENT_SOURCE_DIR}/Config
)

target_link_libraries(rtype_common INTERFACE tomlplusplus::tomlplusplus)