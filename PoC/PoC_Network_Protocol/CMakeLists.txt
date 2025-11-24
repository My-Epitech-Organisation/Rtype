cmake_minimum_required(VERSION 3.16)
project(NetworkProtocolTests)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(tcptest TcpTest.cpp)
find_package(Threads REQUIRED)
target_link_libraries(tcptest Threads::Threads)

add_executable(udptest UdpTest.cpp)
target_link_libraries(udptest Threads::Threads)

if(MSVC)
    target_compile_options(tcptest PRIVATE /W4)
    target_compile_options(udptest PRIVATE /W4)
else()
    target_compile_options(tcptest PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(udptest PRIVATE -Wall -Wextra -Wpedantic)
endif()