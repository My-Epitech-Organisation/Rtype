# Qt Network PoC

cmake_minimum_required(VERSION 3.16)

# Find Qt6
find_package(Qt6 COMPONENTS Core Network QUIET)

if(NOT Qt6_FOUND)
    message(STATUS "Qt6 not found, skipping qt_network_poc")
    return()
endif()

message(STATUS "Qt6 found: ${Qt6_VERSION}")

# Enable Qt automoc
set(CMAKE_AUTOMOC ON)

# UDP Server example
add_executable(qt_udp_server udp_server.cpp)
target_link_libraries(qt_udp_server PRIVATE Qt6::Core Qt6::Network)
target_compile_features(qt_udp_server PRIVATE cxx_std_20)

# UDP Client example
add_executable(qt_udp_client udp_client.cpp)
target_link_libraries(qt_udp_client PRIVATE Qt6::Core Qt6::Network)
target_compile_features(qt_udp_client PRIVATE cxx_std_20)
