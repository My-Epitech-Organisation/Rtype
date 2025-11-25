cmake_minimum_required(VERSION 3.20)
project(PackageManagerTest)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "conan_toolchain.cmake" CACHE STRING "")
endif()

find_package(SFML 2.5 COMPONENTS system window graphics REQUIRED)
find_package(asio 1.24.0 REQUIRED)

add_executable(package_test ../main.cpp)

target_link_libraries(package_test
    sfml-system
    sfml-window
    sfml-graphics
    asio::asio
)

target_compile_features(package_test PRIVATE cxx_std_17)

if(MSVC)
    target_compile_options(package_test PRIVATE /W4)
else()
    target_compile_options(package_test PRIVATE -Wall -Wextra -Wpedantic)
endif()