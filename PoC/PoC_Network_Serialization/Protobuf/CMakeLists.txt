cmake_minimum_required(VERSION 3.20)
project(ProtobufPoC LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
endif()

set(PROJECT_WARNINGS -Wall -Wextra)

function(enable_poc_warnings target)
    if(PROJECT_WARNINGS)
        target_compile_options(${target} PRIVATE ${PROJECT_WARNINGS})
    endif()
endfunction()

include(FetchContent)
set(FETCHCONTENT_QUIET OFF)
set(protobuf_BUILD_TESTS OFF)
set(protobuf_WITH_ZLIB OFF)
set(protobuf_BUILD_LIBPROTOC OFF)
set(protobuf_BUILD_CONFORMANCE OFF)
set(protobuf_BUILD_EXAMPLES OFF)
set(protobuf_BUILD_PROTOC_BINARIES ON)

FetchContent_Declare(
    protobuf
    GIT_REPOSITORY https://github.com/protocolbuffers/protobuf.git
    GIT_TAG v28.2
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(protobuf)

set(PROTO_DIR ${CMAKE_CURRENT_SOURCE_DIR}/proto)
file(MAKE_DIRECTORY ${PROTO_DIR})
set(PROTO_FILES ${PROTO_DIR}/game_state.proto)

set(GENERATED_PROTO_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)
file(MAKE_DIRECTORY ${GENERATED_PROTO_DIR})

set(PROTO_SRCS)
set(PROTO_HDRS)
foreach(proto ${PROTO_FILES})
    get_filename_component(proto_name ${proto} NAME_WE)
    set(cc_file ${GENERATED_PROTO_DIR}/${proto_name}.pb.cc)
    set(h_file ${GENERATED_PROTO_DIR}/${proto_name}.pb.h)
    add_custom_command(
        OUTPUT ${cc_file} ${h_file}
        COMMAND $<TARGET_FILE:protobuf::protoc>
        ARGS --cpp_out=${GENERATED_PROTO_DIR} -I ${PROTO_DIR} ${proto}
        DEPENDS ${proto} protobuf::protoc
        COMMENT "Generating ${proto_name}.pb.*"
        VERBATIM
    )
    list(APPEND PROTO_SRCS ${cc_file})
    list(APPEND PROTO_HDRS ${h_file})
endforeach()

add_library(game_proto STATIC ${PROTO_SRCS} ${PROTO_HDRS})
target_link_libraries(game_proto PUBLIC protobuf::libprotobuf)
target_include_directories(game_proto PUBLIC ${GENERATED_PROTO_DIR})

add_executable(test_protobuf_size test_protobuf_size.cpp)
target_link_libraries(test_protobuf_size PRIVATE game_proto protobuf::libprotobuf)
enable_poc_warnings(test_protobuf_size)

add_executable(benchmark_protobuf benchmark_protobuf.cpp)
target_link_libraries(benchmark_protobuf PRIVATE game_proto protobuf::libprotobuf)
enable_poc_warnings(benchmark_protobuf)

add_executable(compare_serialization compare_serialization.cpp)
target_link_libraries(compare_serialization PRIVATE game_proto protobuf::libprotobuf)
enable_poc_warnings(compare_serialization)
