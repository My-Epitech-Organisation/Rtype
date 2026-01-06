/*
** EPITECH PROJECT, 2026
** Rtype
** File description:
** LobbyDiscoveryServer - Handles lobby discovery requests on base port
*/

#pragma once

#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>
#include <vector>

#include "protocol/OpCode.hpp"
#include "protocol/Payloads.hpp"
#include "transport/IAsyncSocket.hpp"
#include "transport/IoContext.hpp"

namespace rtype::server {

class LobbyManager;

/**
 * @brief Discovery server that handles C_REQUEST_LOBBIES on the base port
 *
 * This lightweight server listens on the base port and responds to
 * lobby list requests. It does not manage full connections - clients
 * connect here only to discover available lobbies, then disconnect
 * and connect to a specific lobby port.
 */
class LobbyDiscoveryServer {
   public:
    /**
     * @brief Construct a new Lobby Discovery Server
     *
     * @param port Port to listen on (base port)
     * @param lobbyManager Reference to the lobby manager to query lobby info
     */
    LobbyDiscoveryServer(std::uint16_t port, LobbyManager& lobbyManager);

    /**
     * @brief Destroy the Lobby Discovery Server
     */
    ~LobbyDiscoveryServer();

    LobbyDiscoveryServer(const LobbyDiscoveryServer&) = delete;
    LobbyDiscoveryServer& operator=(const LobbyDiscoveryServer&) = delete;
    LobbyDiscoveryServer(LobbyDiscoveryServer&&) = delete;
    LobbyDiscoveryServer& operator=(LobbyDiscoveryServer&&) = delete;

    /**
     * @brief Start the discovery server
     *
     * @return true if started successfully, false otherwise
     */
    bool start();

    /**
     * @brief Stop the discovery server
     */
    void stop();

    /**
     * @brief Check if the server is running
     *
     * @return true if running, false otherwise
     */
    bool isRunning() const { return running_; }

    /**
     * @brief Poll for incoming requests
     *
     * Should be called periodically to process incoming packets
     */
    void poll();

   private:
    /**
     * @brief Handle an incoming packet
     *
     * @param data Packet data
     * @param sender Sender endpoint
     */
    void handlePacket(const network::Buffer& data,
                      const network::Endpoint& sender);

    /**
     * @brief Handle a C_REQUEST_LOBBIES request
     *
     * @param sender Client endpoint to send response to
     */
    void handleLobbyListRequest(const network::Endpoint& sender);

    /**
     * @brief Build a S_LOBBY_LIST response packet
     *
     * @return network::Buffer The complete packet
     */
    network::Buffer buildLobbyListPacket();

    /**
     * @brief Start receiving packets asynchronously
     */
    void startReceive();

    std::uint16_t port_;
    LobbyManager& lobbyManager_;  ///< Non-owning reference to manager (lifetime
                                  ///< guaranteed by owner)

    network::IoContext ioContext_;                    ///< ASIO I/O context
    std::unique_ptr<network::IAsyncSocket> socket_;   ///< UDP socket
    std::shared_ptr<network::Buffer> receiveBuffer_;  ///< Buffer for receiving
    std::shared_ptr<network::Endpoint> receiveSender_;  ///< Sender endpoint
    std::atomic<bool> running_{false};
};

}  // namespace rtype::server
