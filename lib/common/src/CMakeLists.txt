add_library(common INTERFACE)
find_package(tomlplusplus CONFIG REQUIRED)

target_include_directories(common INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/Logger
    ${CMAKE_CURRENT_SOURCE_DIR}/ArgParser
    ${CMAKE_CURRENT_SOURCE_DIR}/Config
)

target_link_libraries(common INTERFACE tomlplusplus::tomlplusplus)