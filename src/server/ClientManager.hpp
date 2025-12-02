/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** ClientManager - Manages client connections and state
*/

#ifndef SRC_SERVER_CLIENTMANAGER_HPP_
#define SRC_SERVER_CLIENTMANAGER_HPP_

#include <cstdint>
#include <memory>
#include <optional>
#include <shared_mutex>
#include <unordered_map>
#include <vector>

#include "../common/Logger.hpp"
#include "../common/Types.hpp"
#include "Client.hpp"
#include "ServerMetrics.hpp"

namespace rtype::server {

using rtype::ClientId;
using rtype::DisconnectReason;
using rtype::Endpoint;

/**
 * @brief Manages client connections, lookups, and state
 *
 * Provides thread-safe client management with:
 * - O(1) lookup by client ID and endpoint
 * - Connection rate limiting
 * - Timeout detection support
 * - Client state tracking
 */
class ClientManager {
   public:
    /**
     * @brief Invalid client ID constant
     * @note Set to 0 because valid client IDs start at 1.
     */
    static constexpr ClientId INVALID_CLIENT_ID = 0;

    /// @brief First valid client ID (IDs are assigned starting from this
    /// value)
    static constexpr ClientId FIRST_VALID_CLIENT_ID = 1;

    /// @brief Rate limiting: max new connections per second
    static constexpr uint32_t MAX_CONNECTIONS_PER_SECOND = 10;

    /**
     * @brief Construct a new ClientManager
     * @param maxPlayers Maximum number of concurrent players
     * @param metrics Shared pointer to server metrics for updating counters
     * @param verbose Enable verbose debug output (default: false)
     */
    explicit ClientManager(size_t maxPlayers,
                           std::shared_ptr<ServerMetrics> metrics,
                           bool verbose = false);

    ~ClientManager() = default;

    ClientManager(const ClientManager&) = delete;
    ClientManager& operator=(const ClientManager&) = delete;
    ClientManager(ClientManager&&) = delete;
    ClientManager& operator=(ClientManager&&) = delete;

    /**
     * @brief Handle a new client connection
     * @param endpoint Client network endpoint
     * @return Assigned client ID, or INVALID_CLIENT_ID if rejected
     */
    [[nodiscard]] ClientId handleNewConnection(const Endpoint& endpoint);

    /**
     * @brief Handle client disconnection
     * @param clientId ID of disconnecting client
     * @param reason Reason for disconnection
     */
    void handleClientDisconnect(ClientId clientId,
                                DisconnectReason reason) noexcept;

    /**
     * @brief Update a client's last activity timestamp
     * @param clientId ID of client
     */
    void updateClientActivity(ClientId clientId) noexcept;

    /**
     * @brief Find a client ID by their network endpoint (O(1) lookup)
     * @param endpoint Network endpoint to search for
     * @return Client ID, or INVALID_CLIENT_ID if not found
     */
    [[nodiscard]] ClientId findClientByEndpoint(
        const Endpoint& endpoint) const noexcept;

    /**
     * @brief Get the number of currently connected clients
     * @return Number of connected clients
     */
    [[nodiscard]] size_t getConnectedClientCount() const noexcept;

    /**
     * @brief Get list of all connected client IDs
     * @return Vector of client IDs
     */
    [[nodiscard]] std::vector<ClientId> getConnectedClientIds() const;

    /**
     * @brief Get client information by ID (thread-safe copy)
     * @param clientId The client ID to look up
     * @return Optional containing client info if found
     */
    [[nodiscard]] std::optional<Client> getClientInfo(ClientId clientId) const;

    /**
     * @brief Check for client timeouts and disconnect them
     * @param timeoutSeconds Timeout threshold in seconds
     */
    void checkClientTimeouts(uint32_t timeoutSeconds) noexcept;

    /**
     * @brief Clear all clients (used during shutdown)
     */
    void clearAllClients() noexcept;

    /**
     * @brief Get maximum players capacity
     * @return Maximum number of concurrent players
     */
    [[nodiscard]] size_t getMaxPlayers() const noexcept { return _maxPlayers; }

   private:
    /**
     * @brief Update rate limit window if expired
     * @param nowMs Current time in milliseconds
     * @pre Caller must hold unique lock on _clientsMutex
     */
    void updateRateLimitWindow(int64_t nowMs) noexcept;

    /**
     * @brief Check if rate limit is exceeded
     * @param endpoint Client endpoint for logging
     * @return true if rate limit exceeded, false otherwise
     * @pre Caller must hold unique lock on _clientsMutex
     */
    [[nodiscard]] bool isRateLimitExceeded(const Endpoint& endpoint) noexcept;

    /**
     * @brief Check if server is at capacity
     * @return true if server is full, false otherwise
     * @pre Caller must hold unique lock on _clientsMutex
     */
    [[nodiscard]] bool isServerFull() const noexcept;

    /**
     * @brief Generate the next available client ID
     * @return New client ID, or INVALID_CLIENT_ID if overflow
     * @pre Caller must hold unique lock on _clientsMutex
     */
    [[nodiscard]] ClientId generateNextClientId() noexcept;

    /**
     * @brief Register a new client in the internal maps
     * @param clientId The assigned client ID
     * @param endpoint The client's network endpoint
     * @pre Caller must hold unique lock on _clientsMutex
     */
    void registerClient(ClientId clientId, const Endpoint& endpoint) noexcept;

    /**
     * @brief Handle client disconnection (INTERNAL - caller must hold unique
     * lock)
     * @param clientId ID of disconnecting client
     * @param reason Reason for disconnection
     * @pre Caller must hold unique lock on _clientsMutex
     */
    void handleClientDisconnectInternal(ClientId clientId,
                                        DisconnectReason reason) noexcept;

    /**
     * @brief Remove client from both maps (INTERNAL - caller must hold unique
     * lock)
     * @param clientId Client ID to remove
     * @param endpoint Client endpoint to remove
     * @pre Caller must hold unique lock on _clientsMutex
     */
    void removeClientFromMaps(ClientId clientId,
                              const Endpoint& endpoint) noexcept;

    /**
     * @brief Find a client ID by their network endpoint (INTERNAL - caller must
     * hold lock)
     * @param endpoint Network endpoint to search for
     * @return Client ID, or INVALID_CLIENT_ID if not found
     * @pre Caller must hold lock on _clientsMutex
     */
    [[nodiscard]] ClientId findClientByEndpointInternal(
        const Endpoint& endpoint) const noexcept;

    /**
     * @brief Notify all other clients that a new client connected (INTERNAL -
     * caller must hold lock)
     * @param newClientId ID of the newly connected client
     * @pre Caller must hold lock on _clientsMutex
     */
    void notifyClientConnected(ClientId newClientId) noexcept;

    /**
     * @brief Notify all remaining clients that a client disconnected (INTERNAL
     * - caller must hold lock)
     * @param clientId ID of the disconnected client
     * @param reason Reason for disconnection
     * @pre Caller must hold lock on _clientsMutex
     */
    void notifyClientDisconnected(ClientId clientId,
                                  DisconnectReason reason) noexcept;

    /**
     * @brief Print the list of connected clients to console (INTERNAL - caller
     * must hold lock)
     * @pre Caller must hold lock on _clientsMutex (debug only)
     */
    void printConnectedClients() const noexcept;

    /**
     * @brief Assert that the mutex is currently locked (debug only)
     * @note This is a best-effort check for documentation/debugging purposes
     */
    void assertLockHeld() const noexcept;

    size_t _maxPlayers;                    ///< Maximum concurrent players
    std::weak_ptr<ServerMetrics> _metrics; ///< Weak reference to server metrics
    bool _verbose;                         ///< Enable verbose debug output

    std::unordered_map<ClientId, Client> _clients;
    std::unordered_map<Endpoint, ClientId> _endpointToClient;
    mutable std::shared_mutex _clientsMutex;

    /// @brief Next client ID to assign (starts at FIRST_VALID_CLIENT_ID)
    std::atomic<ClientId> _nextClientId{FIRST_VALID_CLIENT_ID};

    /// @brief Pre-allocated buffer for timeout checking (avoids allocations per
    /// tick)
    std::vector<std::pair<ClientId, Endpoint>> _timeoutBuffer;

    /// @brief Rate limiting state
    std::atomic<uint32_t> _connectionsThisSecond{0};
    std::atomic<int64_t> _rateLimitResetTimeMs{0};
};

}  // namespace rtype::server

#endif  // SRC_SERVER_CLIENTMANAGER_HPP_
