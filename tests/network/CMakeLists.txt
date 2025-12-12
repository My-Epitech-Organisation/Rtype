# Protocol tests (OpCode, Header, Payloads, Validator)
# Tests struct sizes, OpCode values, and validation logic as per RFC RTGP v1.0.0
add_executable(test_protocol test_protocol.cpp)

target_link_libraries(test_protocol PRIVATE
    rtype_network
    GTest::gtest_main
)

# Serialization tests (legacy - to be replaced by #161)
add_executable(test_serialization test_serialization.cpp)

target_link_libraries(test_serialization PRIVATE
    rtype_network
    GTest::gtest_main
)

# Error and Result monad tests
add_executable(test_error test_error.cpp)

target_link_libraries(test_error PRIVATE
    rtype_network
    GTest::gtest_main
)

# Asio UDP Socket tests
add_executable(test_asio_socket test_asio_socket.cpp)

target_link_libraries(test_asio_socket PRIVATE
    rtype_network
    GTest::gtest_main
)

# Reliable Channel (RUDP) tests
add_executable(test_reliable_channel test_reliable_channel.cpp)

target_link_libraries(test_reliable_channel PRIVATE
    rtype_network
    GTest::gtest_main
)

# Connection State Machine tests
add_executable(test_connection_state test_connection_state.cpp)

target_link_libraries(test_connection_state PRIVATE
    rtype_network
    GTest::gtest_main
)

# Connection class tests
add_executable(test_connection test_connection.cpp)

target_link_libraries(test_connection PRIVATE
    rtype_network
    GTest::gtest_main
)

# Packet Validation Security tests (RFC RTGP v1.1.0 Section 6)
add_executable(test_packet_validation test_packet_validation.cpp)

target_link_libraries(test_packet_validation PRIVATE
    rtype_network
    GTest::gtest_main
)

# OpCode tests
add_executable(test_opcode test_opcode.cpp)

target_link_libraries(test_opcode PRIVATE
    rtype_network
    GTest::gtest_main
)

# Validator tests
add_executable(test_validator test_validator.cpp)

target_link_libraries(test_validator PRIVATE
    rtype_network
    GTest::gtest_main
)

# ByteOrderSpec extra coverage tests
add_executable(test_byteorderspec_extra test_byteorderspec_extra.cpp)

target_link_libraries(test_byteorderspec_extra PRIVATE
    rtype_network
    GTest::gtest_main
)

# Exhausive coverage harness
add_executable(test_coverage_harness test_coverage_harness.cpp)

target_link_libraries(test_coverage_harness PRIVATE
    rtype_network
    GTest::gtest_main
)

# New targeted branch tests
add_executable(test_serializer_branches test_serializer_branches.cpp)
target_link_libraries(test_serializer_branches PRIVATE
    rtype_network
    GTest::gtest_main
)

add_executable(test_byteorderspec_branches test_byteorderspec_branches.cpp)
target_link_libraries(test_byteorderspec_branches PRIVATE
    rtype_network
    GTest::gtest_main
)

add_executable(test_header_branches test_header_branches.cpp)
target_link_libraries(test_header_branches PRIVATE
    rtype_network
    GTest::gtest_main
)

add_executable(test_validator_additional test_validator_additional.cpp)
target_link_libraries(test_validator_additional PRIVATE
    rtype_network
    GTest::gtest_main
)

add_executable(test_serializer_and_validator_errors test_serializer_and_validator_errors.cpp)
target_link_libraries(test_serializer_and_validator_errors PRIVATE
    rtype_network
    GTest::gtest_main
)

add_executable(test_serializer_template test_serializer_template.cpp)
target_link_libraries(test_serializer_template PRIVATE
    rtype_network
    GTest::gtest_main
)

add_executable(test_serializer_string_roundtrip test_serializer_string_roundtrip.cpp)
target_link_libraries(test_serializer_string_roundtrip PRIVATE
    rtype_network
    GTest::gtest_main
)

add_executable(test_validator_extra test_validator_extra.cpp)
target_link_libraries(test_validator_extra PRIVATE
    rtype_network
    GTest::gtest_main
)

add_executable(test_serializer_endian test_serializer_endian.cpp)
target_link_libraries(test_serializer_endian PRIVATE
    rtype_network
    GTest::gtest_main
)

add_executable(test_serializer_validate_extract_server_userid test_serializer_validate_extract_server_userid.cpp)
target_link_libraries(test_serializer_validate_extract_server_userid PRIVATE
    rtype_network
    GTest::gtest_main
)

add_executable(test_serializer_validate_extract_extra test_serializer_validate_extract_extra.cpp)
target_link_libraries(test_serializer_validate_extract_extra PRIVATE
    rtype_network
    GTest::gtest_main
)

add_executable(test_byteorderspec_various_sizes test_byteorderspec_various_sizes.cpp)
target_link_libraries(test_byteorderspec_various_sizes PRIVATE
    rtype_network
    GTest::gtest_main
)

# Additional Asio error mapping test

add_executable(test_opcode_exhaustive test_opcode_exhaustive.cpp)
target_link_libraries(test_opcode_exhaustive PRIVATE
    rtype_network
    GTest::gtest_main
)

add_executable(test_validator_success test_validator_success.cpp)
target_link_libraries(test_validator_success PRIVATE
    rtype_network
    GTest::gtest_main
)

# Extra Asio error mapping test
add_executable(test_asio_error_mapping_extra
    test_asio_error_mapping_extra.cpp
    test_asio_socket_error_cases.cpp
)
target_link_libraries(test_asio_error_mapping_extra PRIVATE
    rtype_network
    GTest::gtest_main
)

add_executable(test_asio_error_mapping_all test_asio_error_mapping_all.cpp)
target_link_libraries(test_asio_error_mapping_all PRIVATE
    rtype_network
    GTest::gtest_main
)

add_executable(test_asio_socket_misc test_asio_socket_misc.cpp)
target_link_libraries(test_asio_socket_misc PRIVATE
    rtype_network
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

gtest_discover_tests(test_opcode)
gtest_discover_tests(test_validator)
gtest_discover_tests(test_byteorderspec_extra)
gtest_discover_tests(test_coverage_harness)
gtest_discover_tests(test_serializer_branches)
gtest_discover_tests(test_byteorderspec_branches)
gtest_discover_tests(test_header_branches)
gtest_discover_tests(test_validator_additional)
gtest_discover_tests(test_serializer_template)
gtest_discover_tests(test_byteorderspec_various_sizes)
gtest_discover_tests(test_opcode_exhaustive)
gtest_discover_tests(test_validator_success)
gtest_discover_tests(test_asio_error_mapping_extra)
gtest_discover_tests(test_asio_error_mapping_all)
gtest_discover_tests(test_serializer_and_validator_errors)
gtest_discover_tests(test_serializer_string_roundtrip)
gtest_discover_tests(test_validator_extra)
gtest_discover_tests(test_serializer_endian)
gtest_discover_tests(test_asio_socket_misc)
gtest_discover_tests(test_serializer_validate_extract_server_userid)
gtest_discover_tests(test_serializer_validate_extract_extra)
gtest_discover_tests(test_asio_error_mapping_all)
## test_asio_error_mapping removed due to private API access - rely on other asio tests
