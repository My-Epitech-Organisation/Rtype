#include <gtest/gtest.h>

#include <asio/error.hpp>

#include "transport/AsioUdpSocket.hpp"

using namespace rtype::network;

TEST(AsioUdpSocketErrorMappingAll, FullMap) {
    EXPECT_EQ(AsioUdpSocket::fromAsioError(asio::error::make_error_code(asio::error::operation_aborted)), NetworkError::Cancelled);
    EXPECT_EQ(AsioUdpSocket::fromAsioError(asio::error::make_error_code(asio::error::connection_refused)), NetworkError::ConnectionRefused);
    EXPECT_EQ(AsioUdpSocket::fromAsioError(asio::error::make_error_code(asio::error::timed_out)), NetworkError::Timeout);
    EXPECT_EQ(AsioUdpSocket::fromAsioError(asio::error::make_error_code(asio::error::host_not_found)), NetworkError::HostNotFound);
    EXPECT_EQ(AsioUdpSocket::fromAsioError(asio::error::make_error_code(asio::error::host_not_found_try_again)), NetworkError::HostNotFound);
    EXPECT_EQ(AsioUdpSocket::fromAsioError(asio::error::make_error_code(asio::error::network_unreachable)), NetworkError::NetworkUnreachable);
    EXPECT_EQ(AsioUdpSocket::fromAsioError(asio::error::make_error_code(asio::error::address_in_use)), NetworkError::AddressInUse);
    EXPECT_EQ(AsioUdpSocket::fromAsioError(asio::error::make_error_code(asio::error::would_block)), NetworkError::WouldBlock);
    EXPECT_EQ(AsioUdpSocket::fromAsioError(asio::error::make_error_code(asio::error::try_again)), NetworkError::WouldBlock);
    EXPECT_EQ(AsioUdpSocket::fromAsioError(asio::error::make_error_code(asio::error::not_connected)), NetworkError::NotConnected);
    EXPECT_EQ(AsioUdpSocket::fromAsioError(asio::error::make_error_code(asio::error::message_size)), NetworkError::PacketTooLarge);
    // Unknown code should map to InternalError
    EXPECT_EQ(AsioUdpSocket::fromAsioError(asio::error::make_error_code(static_cast<asio::error::basic_errors>(999999))), NetworkError::InternalError);
}
