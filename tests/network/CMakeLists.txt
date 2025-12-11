# Protocol tests (OpCode, Header, Payloads, Validator)
# Tests struct sizes, OpCode values, and validation logic as per RFC RTGP v1.0.0
add_executable(test_protocol test_protocol.cpp)

target_link_libraries(test_protocol PRIVATE
    network
    GTest::gtest_main
)

# Serialization tests (legacy - to be replaced by #161)
add_executable(test_serialization test_serialization.cpp)

target_link_libraries(test_serialization PRIVATE
    network
    GTest::gtest_main
)

# Error and Result monad tests
add_executable(test_error test_error.cpp)

target_link_libraries(test_error PRIVATE
    network
    GTest::gtest_main
)

# Asio UDP Socket tests
add_executable(test_asio_socket test_asio_socket.cpp)

target_link_libraries(test_asio_socket PRIVATE
    network
    GTest::gtest_main
)

# Reliable Channel (RUDP) tests
add_executable(test_reliable_channel test_reliable_channel.cpp)

target_link_libraries(test_reliable_channel PRIVATE
    network
    GTest::gtest_main
)

# Connection State Machine tests
add_executable(test_connection_state test_connection_state.cpp)

target_link_libraries(test_connection_state PRIVATE
    network
    GTest::gtest_main
)

# Connection class tests
add_executable(test_connection test_connection.cpp)

target_link_libraries(test_connection PRIVATE
    network
    GTest::gtest_main
)

# Packet Validation Security tests (RFC RTGP v1.1.0 Section 6)
add_executable(test_packet_validation test_packet_validation.cpp)

target_link_libraries(test_packet_validation PRIVATE
    network
    GTest::gtest_main
)

# Enable test discovery
include(GoogleTest)
gtest_discover_tests(test_protocol)
gtest_discover_tests(test_serialization)
gtest_discover_tests(test_error)
gtest_discover_tests(test_asio_socket)
gtest_discover_tests(test_reliable_channel)
gtest_discover_tests(test_connection_state)
gtest_discover_tests(test_connection)
gtest_discover_tests(test_packet_validation)
