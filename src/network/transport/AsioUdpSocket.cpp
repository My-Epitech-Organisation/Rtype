/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** AsioUdpSocket - Implementation
*/

#include "AsioUdpSocket.hpp"

#include <utility>

namespace rtype::network {

// ============================================================================
// Constructor / Destructor
// ============================================================================

AsioUdpSocket::AsioUdpSocket(asio::io_context& ioContext)
    : socket_(ioContext, asio::ip::udp::v4()) {}

AsioUdpSocket::~AsioUdpSocket() { close(); }

// ============================================================================
// Socket Configuration
// ============================================================================

Result<void> AsioUdpSocket::bind(std::uint16_t port) {
    std::lock_guard<std::mutex> lock(mutex_);

    asio::error_code ec;

    // Close existing socket if open
    if (socket_.is_open()) {
        socket_.close(ec);
    }

    // Open new socket
    socket_.open(asio::ip::udp::v4(), ec);
    if (ec) {
        return Err(fromAsioError(ec));
    }

    // Set socket options
    socket_.set_option(asio::socket_base::reuse_address(true), ec);
    if (ec) {
        return Err(fromAsioError(ec));
    }

    // Bind to port
    asio::ip::udp::endpoint localEndpoint(asio::ip::udp::v4(), port);
    socket_.bind(localEndpoint, ec);
    if (ec) {
        return Err(fromAsioError(ec));
    }

    return Ok();
}

bool AsioUdpSocket::isOpen() const noexcept {
    std::lock_guard<std::mutex> lock(mutex_);
    return socket_.is_open();
}

std::uint16_t AsioUdpSocket::localPort() const noexcept {
    std::lock_guard<std::mutex> lock(mutex_);

    if (!socket_.is_open()) {
        return 0;
    }

    asio::error_code ec;
    auto endpoint = socket_.local_endpoint(ec);
    if (ec) {
        return 0;
    }

    return endpoint.port();
}

// ============================================================================
// Asynchronous Operations
// ============================================================================

void AsioUdpSocket::asyncSendTo(const Buffer& data, const Endpoint& dest,
                                SendCallback handler) {
    if (!handler) {
        return;
    }

    // Validate destination
    if (!dest.isValid()) {
        // Post callback to io_context to maintain async semantics
        asio::post(socket_.get_executor(), [handler = std::move(handler)]() {
            handler(Err<std::size_t>(NetworkError::NotConnected));
        });
        return;
    }

    // Convert endpoint
    asio::ip::udp::endpoint asioEndpoint;
    try {
        asioEndpoint = toAsioEndpoint(dest);
    } catch (const std::exception&) {
        asio::post(socket_.get_executor(), [handler = std::move(handler)]() {
            handler(Err<std::size_t>(NetworkError::HostNotFound));
        });
        return;
    }

    // Create shared buffer to ensure lifetime during async operation
    auto sharedBuffer = std::make_shared<Buffer>(data);

    socket_.async_send_to(
        asio::buffer(*sharedBuffer), asioEndpoint,
        [handler = std::move(handler), sharedBuffer](const asio::error_code& ec,
                                                     std::size_t bytesSent) {
            if (ec) {
                handler(Err<std::size_t>(fromAsioError(ec)));
            } else {
                handler(Ok(bytesSent));
            }
        });
}

void AsioUdpSocket::asyncReceiveFrom(Buffer& buffer, Endpoint& sender,
                                     ReceiveCallback handler) {
    if (!handler) {
        return;
    }

    // Ensure buffer has capacity
    if (buffer.empty()) {
        buffer.resize(kMaxPacketSize);
    }

    socket_.async_receive_from(
        asio::buffer(buffer), remoteEndpoint_,
        [this, &buffer, &sender, handler = std::move(handler)](
            const asio::error_code& ec, std::size_t bytesReceived) {
            if (ec) {
                handler(Err<std::size_t>(fromAsioError(ec)));
            } else {
                // Shrink buffer to actual received size (never grow to avoid reallocation)
                if (bytesReceived < buffer.size()) {
                    buffer.resize(bytesReceived);
                }

                // Populate sender endpoint
                sender = fromAsioEndpoint(remoteEndpoint_);

                handler(Ok(bytesReceived));
            }
        });
}

// ============================================================================
// Lifecycle Management
// ============================================================================

void AsioUdpSocket::cancel() {
    std::lock_guard<std::mutex> lock(mutex_);

    if (socket_.is_open()) {
        asio::error_code ec;
        socket_.cancel(ec);
        // Ignore errors - socket may be in invalid state
    }
}

void AsioUdpSocket::close() {
    std::lock_guard<std::mutex> lock(mutex_);

    if (socket_.is_open()) {
        asio::error_code ec;
        socket_.shutdown(asio::socket_base::shutdown_both, ec);
        socket_.close(ec);
        // Ignore errors - socket may be in invalid state
    }
}

// ============================================================================
// Helper Methods
// ============================================================================

NetworkError AsioUdpSocket::fromAsioError(const asio::error_code& ec) noexcept {
    if (!ec) {
        return NetworkError::None;
    }

    // Map common Asio errors to NetworkError
    if (ec == asio::error::operation_aborted) {
        return NetworkError::Cancelled;
    }
    if (ec == asio::error::connection_refused) {
        return NetworkError::ConnectionRefused;
    }
    if (ec == asio::error::timed_out) {
        return NetworkError::Timeout;
    }
    if (ec == asio::error::host_not_found ||
        ec == asio::error::host_not_found_try_again) {
        return NetworkError::HostNotFound;
    }
    if (ec == asio::error::network_unreachable) {
        return NetworkError::NetworkUnreachable;
    }
    if (ec == asio::error::address_in_use) {
        return NetworkError::AddressInUse;
    }
    if (ec == asio::error::would_block || ec == asio::error::try_again) {
        return NetworkError::WouldBlock;
    }
    if (ec == asio::error::not_connected) {
        return NetworkError::NotConnected;
    }
    if (ec == asio::error::message_size) {
        return NetworkError::PacketTooLarge;
    }

    // Default to internal error for unmapped errors
    return NetworkError::InternalError;
}

Endpoint AsioUdpSocket::fromAsioEndpoint(const asio::ip::udp::endpoint& ep) {
    return Endpoint{ep.address().to_string(), ep.port()};
}

asio::ip::udp::endpoint AsioUdpSocket::toAsioEndpoint(const Endpoint& ep) {
    asio::ip::address addr = asio::ip::make_address(ep.address);
    return asio::ip::udp::endpoint(addr, ep.port);
}

}  // namespace rtype::network
