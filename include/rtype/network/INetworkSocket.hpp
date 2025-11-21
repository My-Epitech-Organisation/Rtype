/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** INetworkSocket - Public interface for network communication
*/

#pragma once

#include <string>
#include <cstdint>
#include <cstddef>

namespace rtype::network {

/**
 * @brief Public interface for UDP network socket
 *
 * Provides a simple abstraction for UDP socket operations.
 * Use this for both client and server network communication.
 *
 * Example usage:
 * @code
 * // Server
 * INetworkSocket& socket = createSocket();
 * socket.bind(4242);
 *
 * // Client
 * INetworkSocket& clientSocket = createSocket();
 * clientSocket.connect("127.0.0.1", 4242);
 * @endcode
 */
class INetworkSocket {
public:
    virtual ~INetworkSocket() = default;

    /**
     * @brief Bind socket to a specific port (server mode)
     * @param port Port number to bind to
     * @return true if binding succeeded, false otherwise
     */
    virtual bool bind(uint16_t port) = 0;

    /**
     * @brief Connect socket to a remote host (client mode)
     * @param host Hostname or IP address
     * @param port Remote port number
     * @return true if connection succeeded, false otherwise
     */
    virtual bool connect(const std::string& host, uint16_t port) = 0;

    /**
     * @brief Send data through the socket
     * @param data Pointer to data buffer
     * @param size Size of data to send in bytes
     * @return Number of bytes sent, or -1 on error
     */
    virtual int send(const void* data, std::size_t size) = 0;

    /**
     * @brief Receive data from the socket
     * @param buffer Buffer to store received data
     * @param maxSize Maximum number of bytes to receive
     * @return Number of bytes received, or -1 on error
     */
    virtual int receive(void* buffer, std::size_t maxSize) = 0;

    /**
     * @brief Close the socket
     */
    virtual void close() = 0;
};

} // namespace rtype::network
