# ASIO Standalone PoC Examples

add_executable(asio_udp_server udp_server.cpp)
target_link_libraries(asio_udp_server PRIVATE asio)
target_compile_features(asio_udp_server PRIVATE cxx_std_20)

add_executable(asio_udp_client udp_client.cpp)
target_link_libraries(asio_udp_client PRIVATE asio)
target_compile_features(asio_udp_client PRIVATE cxx_std_20)

# Copy binaries to build root for easy execution
set_target_properties(asio_udp_server asio_udp_client
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
