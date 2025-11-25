cmake_minimum_required(VERSION 3.20)
project(PackageManagerTest)

include(cmake/CPM.cmake)

CPMAddPackage(
    NAME SFML
    GITHUB_REPOSITORY SFML/SFML
    GIT_TAG 3.0.2
    OPTIONS
        "SFML_BUILD_AUDIO FALSE"
        "SFML_BUILD_NETWORK FALSE"
)

CPMAddPackage(
    NAME Asio
    GITHUB_REPOSITORY chriskohlhoff/asio
    GIT_TAG asio-1-32-0
    OPTIONS
        "ASIO_STANDALONE ON"
)

add_executable(package_test ../main.cpp)

target_link_libraries(package_test
    SFML::System
    SFML::Window
    SFML::Graphics
)

target_include_directories(package_test PRIVATE ${Asio_SOURCE_DIR}/asio/include)

target_compile_features(package_test PRIVATE cxx_std_17)

if(MSVC)
    target_compile_options(package_test PRIVATE /W4)
else()
    target_compile_options(package_test PRIVATE -Wall -Wextra -Wpedantic)
endif()