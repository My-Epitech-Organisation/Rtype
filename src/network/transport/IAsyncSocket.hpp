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

using SendCallback = std::function<void(Result<std::size_t>)>;
using ReceiveCallback = std::function<void(Result<std::size_t>)>;
using ConnectCallback = std::function<void(Result<void>)>;

/**
 * @brief Abstract interface for asynchronous UDP socket operations
 *
 * Abstraction over async I/O (Asio, libuv, etc.) enabling non-blocking I/O,
 * portability, and testability with mock sockets.
 *
 * Thread-safe for async ops from any thread. Callbacks may run from I/O thread.
 * Caller must ensure buffers remain valid until callback and socket outlives
 * pending operations.
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

    /// Bind the socket to a local port (0 for ephemeral)
    virtual Result<void> bind(std::uint16_t port) = 0;

    [[nodiscard]] virtual bool isOpen() const noexcept = 0;

    [[nodiscard]] virtual std::uint16_t localPort() const noexcept = 0;

    /// Async send to endpoint. Buffer must remain valid until callback.
    virtual void asyncSendTo(const Buffer& data, const Endpoint& dest,
                             SendCallback handler) = 0;

    /// Async receive. Buffer/sender must remain valid until callback.
    virtual void asyncReceiveFrom(Buffer& buffer, Endpoint& sender,
                                  ReceiveCallback handler) = 0;

    /// Cancel pending operations (callbacks get NetworkError::Cancelled)
    virtual void cancel() = 0;

    /// Close socket and release resources
    virtual void close() = 0;

   protected:
    // Protected constructor - use factory function to create instances
    IAsyncSocket() = default;
};

/**
 * @brief Create an async UDP socket
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
