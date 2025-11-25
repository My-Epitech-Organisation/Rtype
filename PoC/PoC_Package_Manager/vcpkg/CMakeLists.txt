cmake_minimum_required(VERSION 3.20)
project(PackageManagerTest)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
endif()

find_package(SFML 3.0 COMPONENTS System Window Graphics REQUIRED)
find_package(asio CONFIG REQUIRED)

add_executable(package_test ../main.cpp)

target_link_libraries(package_test
    SFML::System
    SFML::Window
    SFML::Graphics
    asio::asio
)

target_compile_features(package_test PRIVATE cxx_std_17)

if(MSVC)
    target_compile_options(package_test PRIVATE /W4)
else()
    target_compile_options(package_test PRIVATE -Wall -Wextra -Wpedantic)
endif()