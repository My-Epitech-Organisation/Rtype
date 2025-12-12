#include <gtest/gtest.h>

#include <asio/error.hpp>

#include "transport/AsioUdpSocket.hpp"

using namespace rtype::network;

TEST(AsioUdpSocketErrorMapping, FromAsioErrorMappings) {
    // Covers the branch mapping for common ASIO errors
    EXPECT_EQ(AsioUdpSocket::fromAsioError(asio::error::make_error_code(asio::error::operation_aborted)), NetworkError::Cancelled);
    EXPECT_EQ(AsioUdpSocket::fromAsioError(asio::error::make_error_code(asio::error::connection_refused)), NetworkError::ConnectionRefused);
    EXPECT_EQ(AsioUdpSocket::fromAsioError(asio::error::make_error_code(asio::error::timed_out)), NetworkError::Timeout);
    EXPECT_EQ(AsioUdpSocket::fromAsioError(asio::error::make_error_code(asio::error::host_not_found)), NetworkError::HostNotFound);
    EXPECT_EQ(AsioUdpSocket::fromAsioError(asio::error::make_error_code(asio::error::network_unreachable)), NetworkError::NetworkUnreachable);
    EXPECT_EQ(AsioUdpSocket::fromAsioError(asio::error::make_error_code(asio::error::address_in_use)), NetworkError::AddressInUse);
    EXPECT_EQ(AsioUdpSocket::fromAsioError(asio::error::make_error_code(asio::error::would_block)), NetworkError::WouldBlock);
    EXPECT_EQ(AsioUdpSocket::fromAsioError(asio::error_code{}), NetworkError::None);
    // Unknown mapping falls back to InternalError
    EXPECT_EQ(AsioUdpSocket::fromAsioError(asio::error::make_error_code(asio::error::fault)), NetworkError::InternalError);
}
