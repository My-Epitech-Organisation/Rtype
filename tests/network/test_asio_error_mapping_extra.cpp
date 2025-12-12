#include <gtest/gtest.h>

#include <asio/error.hpp>

#include "transport/AsioUdpSocket.hpp"

using namespace rtype::network;

TEST(AsioUdpSocketErrorMappingExtra, AdditionalMappings) {
    EXPECT_EQ(AsioUdpSocket::fromAsioError(asio::error::make_error_code(asio::error::host_not_found_try_again)), NetworkError::HostNotFound);
    EXPECT_EQ(AsioUdpSocket::fromAsioError(asio::error::make_error_code(asio::error::try_again)), NetworkError::WouldBlock);
    EXPECT_EQ(AsioUdpSocket::fromAsioError(asio::error::make_error_code(asio::error::not_connected)), NetworkError::NotConnected);
    EXPECT_EQ(AsioUdpSocket::fromAsioError(asio::error::make_error_code(asio::error::message_size)), NetworkError::PacketTooLarge);
}
