/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** Client - Client data structure for server-side tracking
*/

#ifndef SRC_SERVER_SHARED_CLIENT_HPP_
#define SRC_SERVER_SHARED_CLIENT_HPP_

#include <chrono>
#include <cstdint>

#include <rtype/common.hpp>

namespace rtype::server {

using rtype::ClientId;
using rtype::ClientState;
using rtype::Endpoint;

/**
 * @brief Information about a connected client
 *
 * Tracks all necessary state for managing a client connection,
 * including timeout detection and identification.
 */
struct Client {
    ClientId id{0};     ///< Unique client identifier
    Endpoint endpoint;  ///< Network endpoint (IP:port)
    std::chrono::steady_clock::time_point
        lastActivityTime;  ///< Last time we received data from this client
    ClientState state{ClientState::Connected};  ///< Current connection state
};

}  // namespace rtype::server

#endif  // SRC_SERVER_SHARED_CLIENT_HPP_
