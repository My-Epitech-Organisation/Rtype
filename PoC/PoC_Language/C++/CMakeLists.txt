cmake_minimum_required(VERSION 3.10)
project(RType_CPP_PoC)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(rtype_poc main.cpp)

target_include_directories(rtype_poc PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
