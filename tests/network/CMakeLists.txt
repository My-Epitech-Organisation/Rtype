# Serialization tests
add_executable(test_serialization test_serialization.cpp)

target_link_libraries(test_serialization PRIVATE
    rtype_network
    GTest::gtest_main
)

# Asio UDP Socket tests
add_executable(test_asio_socket test_asio_socket.cpp)

target_link_libraries(test_asio_socket PRIVATE
    rtype_network
    GTest::gtest_main
)

# Enable test discovery
include(GoogleTest)
gtest_discover_tests(test_serialization)
gtest_discover_tests(test_asio_socket)
