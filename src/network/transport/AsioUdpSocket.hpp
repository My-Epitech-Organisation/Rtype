/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** AsioUdpSocket - Asio implementation of IAsyncSocket
*/

#pragma once

#include <memory>
#include <mutex>

#include <asio.hpp>

#include "IAsyncSocket.hpp"
#include "core/Error.hpp"
#include "core/Types.hpp"
#include "protocol/Header.hpp"  // For kMaxPacketSize

namespace rtype::network {

/**
 * @brief Asio-based implementation of IAsyncSocket
 *
 * Non-blocking UDP socket using Asio standalone.
 * Thread-safe with internal mutex, cross-platform.
 */
class AsioUdpSocket : public IAsyncSocket {
   public:
    /**
     * @brief Construct a new AsioUdpSocket
     *
     * @param ioContext Reference to the Asio io_context (must outlive socket)
     */
    explicit AsioUdpSocket(asio::io_context& ioContext);

    /**
     * @brief Destructor - closes socket if open
     */
    ~AsioUdpSocket() override;

    Result<void> bind(std::uint16_t port) override;

    [[nodiscard]] bool isOpen() const noexcept override;

    [[nodiscard]] std::uint16_t localPort() const noexcept override;

    void asyncSendTo(const Buffer& data, const Endpoint& dest,
                     SendCallback handler) override;

    void asyncReceiveFrom(Buffer& buffer, Endpoint& sender,
                          ReceiveCallback handler) override;

    void cancel() override;

    void close() override;

   private:
    static NetworkError fromAsioError(const asio::error_code& ec) noexcept;
    static Endpoint fromAsioEndpoint(const asio::ip::udp::endpoint& ep);
    static asio::ip::udp::endpoint toAsioEndpoint(const Endpoint& ep);

    asio::ip::udp::socket socket_;
    mutable std::mutex mutex_;
    asio::ip::udp::endpoint remoteEndpoint_;
};

[[nodiscard]] inline std::unique_ptr<IAsyncSocket> createAsyncSocket(
    asio::io_context& ioContext) {
    return std::make_unique<AsioUdpSocket>(ioContext);
}

}  // namespace rtype::network
