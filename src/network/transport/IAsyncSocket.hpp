/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** IAsyncSocket - Abstract interface for asynchronous UDP sockets
*/

#pragma once

#include <cstdint>
#include <functional>
#include <memory>

#include "core/Error.hpp"
#include "core/Types.hpp"

namespace rtype::network {

// ============================================================================
// Callback Types
// ============================================================================

/**
 * @brief Callback invoked when an async send operation completes
 *
 * @param result Success: number of bytes sent, Error: NetworkError code
 *
 * @note The callback may be invoked from a different thread than the caller.
 *       Ensure proper synchronization if accessing shared state.
 */
using SendCallback = std::function<void(Result<std::size_t>)>;

/**
 * @brief Callback invoked when an async receive operation completes
 *
 * @param result Success: number of bytes received, Error: NetworkError code
 *
 * @note On success, the buffer passed to asyncReceiveFrom() contains the
 *       received data, and the sender endpoint is populated.
 */
using ReceiveCallback = std::function<void(Result<std::size_t>)>;

/**
 * @brief Callback invoked when a connection/bind operation completes
 *
 * @param result Success: void, Error: NetworkError code
 */
using ConnectCallback = std::function<void(Result<void>)>;

// ============================================================================
// IAsyncSocket Interface
// ============================================================================

/**
 * @brief Abstract interface for asynchronous UDP socket operations
 *
 * This interface provides a clean abstraction over async I/O implementations
 * (e.g., Asio, libuv, raw epoll/IOCP). It enables:
 *
 * - **Non-blocking I/O**: Game loop never waits on network operations
 * - **Portability**: Swap implementations without changing game code
 * - **Testability**: Mock sockets for unit testing
 *
 * ## Thread Safety
 *
 * Implementations should be thread-safe for the following patterns:
 * - Calling async operations from any thread
 * - Callbacks may be invoked from a dedicated I/O thread
 *
 * ## Ownership
 *
 * The caller must ensure that:
 * - Buffers passed to async operations remain valid until callback
 * - The socket instance outlives all pending operations
 *
 * @example Server usage
 * ```cpp
 * auto socket = createAsyncSocket(ioContext);
 * socket->bind(4242);
 *
 * Buffer recvBuffer(1400);
 * Endpoint sender;
 *
 * socket->asyncReceiveFrom(recvBuffer, sender, [&](Result<size_t> result) {
 *     if (result.isOk()) {
 *         processPacket(recvBuffer, result.value(), sender);
 *         // Queue next receive...
 *     }
 * });
 * ```
 *
 * @example Client usage
 * ```cpp
 * auto socket = createAsyncSocket(ioContext);
 *
 * Endpoint server{"127.0.0.1", 4242};
 * Buffer packet = buildConnectPacket();
 *
 * socket->asyncSendTo(packet, server, [](Result<size_t> result) {
 *     if (result.isErr()) {
 *         handleError(result.error());
 *     }
 * });
 * ```
 */
class IAsyncSocket {
   public:
    virtual ~IAsyncSocket() = default;

    // --- Prevent copying (sockets own system resources) ---
    IAsyncSocket(const IAsyncSocket&) = delete;
    IAsyncSocket& operator=(const IAsyncSocket&) = delete;

    // --- Allow moving ---
    IAsyncSocket(IAsyncSocket&&) = default;
    IAsyncSocket& operator=(IAsyncSocket&&) = default;

    // ========================================================================
    // Socket Configuration
    // ========================================================================

    /**
     * @brief Bind the socket to a local port (server mode)
     *
     * After binding, the socket can receive datagrams on the specified port.
     * For clients, binding is optional (OS assigns ephemeral port).
     *
     * @param port Local port number to bind to (0 for ephemeral)
     * @return Success or error (e.g., AddressInUse)
     *
     * @note Must be called before asyncReceiveFrom() for servers
     */
    virtual Result<void> bind(std::uint16_t port) = 0;

    /**
     * @brief Check if the socket is open and ready for operations
     */
    [[nodiscard]] virtual bool isOpen() const noexcept = 0;

    /**
     * @brief Get the local port the socket is bound to
     *
     * @return Port number, or 0 if not bound
     */
    [[nodiscard]] virtual std::uint16_t localPort() const noexcept = 0;

    // ========================================================================
    // Asynchronous Operations
    // ========================================================================

    /**
     * @brief Asynchronously send data to a remote endpoint
     *
     * The operation completes immediately (non-blocking). The callback is
     * invoked when the data has been handed to the OS for transmission.
     *
     * @param data Buffer containing data to send (must remain valid until
     * callback)
     * @param dest Destination endpoint (IP + port)
     * @param handler Callback invoked on completion
     *
     * @note UDP is unreliable - successful send does not guarantee delivery
     * @note For reliability, use the RUDP layer built on top of this
     *
     * @pre isOpen() == true
     */
    virtual void asyncSendTo(const Buffer& data, const Endpoint& dest,
                             SendCallback handler) = 0;

    /**
     * @brief Asynchronously receive data from any remote endpoint
     *
     * The operation completes when a datagram is received. The callback is
     * invoked with the number of bytes received.
     *
     * @param buffer Buffer to store received data (must remain valid until
     * callback)
     * @param sender [out] Populated with sender's endpoint on success
     * @param handler Callback invoked on completion
     *
     * @note Only one receive operation should be pending at a time
     * @note Buffer should be at least kMaxPacketSize bytes for UDP
     *
     * @pre isOpen() == true
     * @pre bind() has been called (for servers)
     */
    virtual void asyncReceiveFrom(Buffer& buffer, Endpoint& sender,
                                  ReceiveCallback handler) = 0;

    // ========================================================================
    // Lifecycle Management
    // ========================================================================

    /**
     * @brief Cancel all pending asynchronous operations
     *
     * All pending callbacks will be invoked with NetworkError::Cancelled.
     * The socket remains open and can be used for new operations.
     *
     * @note Thread-safe: can be called from any thread
     */
    virtual void cancel() = 0;

    /**
     * @brief Close the socket and release all resources
     *
     * All pending operations are cancelled. After closing, the socket
     * cannot be used for any operations.
     *
     * @note Thread-safe: can be called from any thread
     * @note Calling close() on an already-closed socket is safe (no-op)
     */
    virtual void close() = 0;

   protected:
    // Protected constructor - use factory function to create instances
    IAsyncSocket() = default;
};

// ============================================================================
// Factory Function (Forward Declaration)
// ============================================================================

/**
 * @brief Create an async UDP socket using the default implementation (Asio)
 *
 * @param ioContext Reference to the I/O context for async operations
 * @return Unique pointer to the socket, or nullptr on failure
 *
 * @note The ioContext must outlive the socket
 *
 * @example
 * ```cpp
 * asio::io_context ioContext;
 * auto socket = createAsyncSocket(ioContext);
 * ```
 */
// Forward declared - implementation in AsioUdpSocket.cpp
// std::unique_ptr<IAsyncSocket> createAsyncSocket(asio::io_context& ioContext);

}  // namespace rtype::network
