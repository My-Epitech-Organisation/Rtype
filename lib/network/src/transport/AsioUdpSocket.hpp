/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** AsioUdpSocket - Asio implementation of IAsyncSocket
*/

#pragma once

#include <functional>
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
     * @param ioContext Reference to the Asio io_context (must outlive socket).
     * @note Non-const reference required by Asio API - io_context is modified
     *       internally by Asio for event dispatching and handler management.
     *       This is a documented exception to const-correctness rules.
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

    void asyncReceiveFrom(std::shared_ptr<Buffer> buffer,
                          std::shared_ptr<Endpoint> sender,
                          ReceiveCallback handler) override;

    void cancel() override;

    void close() override;

    static NetworkError fromAsioError(const asio::error_code& ec) noexcept;

    private:
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
