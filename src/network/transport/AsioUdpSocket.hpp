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

namespace rtype::network {

/**
 * @brief Asio-based implementation of IAsyncSocket
 *
 * Provides non-blocking UDP socket operations using Asio standalone.
 * Thread-safe for concurrent async operations.
 *
 * ## Features
 *
 * - Non-blocking async send/receive
 * - Cross-platform (Windows/Linux/macOS)
 * - Thread-safe with internal mutex
 * - Automatic endpoint conversion (Asio <-> rtype::network)
 *
 * ## Example
 *
 * ```cpp
 * IoContext ctx;
 * AsioUdpSocket socket(ctx);
 *
 * if (auto result = socket.bind(4242); result.isErr()) {
 *     // Handle error
 * }
 *
 * Buffer recvBuffer(kMaxPacketSize);
 * Endpoint sender;
 *
 * socket.asyncReceiveFrom(recvBuffer, sender, [](Result<size_t> result) {
 *     if (result.isOk()) {
 *         std::cout << "Received " << result.value() << " bytes\n";
 *     }
 * });
 *
 * ctx.poll();  // Process async operations
 * ```
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

    // ========================================================================
    // IAsyncSocket Implementation
    // ========================================================================

    Result<void> bind(std::uint16_t port) override;

    [[nodiscard]] bool isOpen() const noexcept override;

    [[nodiscard]] std::uint16_t localPort() const noexcept override;

    void asyncSendTo(const Buffer& data, const Endpoint& dest,
                     SendCallback handler) override;

    void asyncReceiveFrom(Buffer& buffer, Endpoint& sender,
                          ReceiveCallback handler) override;

    void cancel() override;

    void close() override;

    // ========================================================================
    // Additional Methods
    // ========================================================================

    /**
     * @brief Get the underlying Asio socket (for advanced use)
     *
     * @warning Use with caution - bypasses abstraction
     */
    [[nodiscard]] asio::ip::udp::socket& nativeSocket() noexcept {
        return socket_;
    }

   private:
    // --- Helper Methods ---

    /**
     * @brief Convert NetworkError to Asio error_code
     */
    static NetworkError fromAsioError(const asio::error_code& ec) noexcept;

    /**
     * @brief Convert Asio endpoint to our Endpoint type
     */
    static Endpoint fromAsioEndpoint(const asio::ip::udp::endpoint& ep);

    /**
     * @brief Convert our Endpoint to Asio endpoint
     */
    static asio::ip::udp::endpoint toAsioEndpoint(const Endpoint& ep);

    // --- Member Variables ---

    asio::ip::udp::socket socket_;

    /// Mutex for thread-safe operations
    mutable std::mutex mutex_;

    /// Temporary endpoint for receive operations
    asio::ip::udp::endpoint remoteEndpoint_;
};

// ============================================================================
// Factory Function
// ============================================================================

/**
 * @brief Create an async UDP socket
 *
 * @param ioContext Reference to the I/O context
 * @return Unique pointer to the socket
 */
[[nodiscard]] inline std::unique_ptr<IAsyncSocket> createAsyncSocket(
    asio::io_context& ioContext) {
    return std::make_unique<AsioUdpSocket>(ioContext);
}

}  // namespace rtype::network
